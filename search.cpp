#include "search.h"
#include "engine.h"
#include "bitboard.h"
#include "eval.h"
#include "movelist.h"
#include "move.h"
#include "tt.h"
#include "gen.h"

#include <immintrin.h>
#include <intrin.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <algorithm>    // std::max

int RAZOR_DEPTH = 3;
int FUTILITY_DEPTH = 5;
int IID_DEPTH = 5;
int LMP_DEPTH = 10;
int RAZOR_MARGIN = 2974;
int FUTILITY_MARGIN = 739;
int HISTORYDIV = 480;
int reductionDiv = 197;
int selDepth = 0;

int max(int a, int b) {
	return b < a ? a : b;
}
int min(int a, int b) {
	return !(b < a) ? a : b;
}

int mateToTT(int ply, int score) {
	if (score >= mateScore - 1000) {
		return score + ply;
	}
	if (score <= -mateScore + 1000) {
		return score - ply;
	}
	return score;
}

int ttToMate(int ply, int score) {

	if (score>=mateScore-1000) {
		return score-ply;
	}
	if (score <= -mateScore + 1000) {
		return score + ply;
	}
	return score;
}

int killers[100][2];
int ht[2][64][64];
int cmh[12][64][12][64];
int lMove[200][2];
int lmrReductions[64][64];

int lmp[11] = { 0, 5, 6, 9, 14, 21, 30, 41, 55, 69, 84 };
void initReductionTable() {
	for (int depth = 1; depth < 64; depth++) {
		for (int movesSearched = 1; movesSearched < 64; movesSearched++)
			lmrReductions[depth][movesSearched] = (int)((0.5 + log(depth) * log(movesSearched))*100 / reductionDiv);
	}
}

void resetHistory() {
	for (int i = 0; i < 64; i++) {
		for (int e = 0; e < 64; e++) {
			ht[0][i][e] = 0;
			ht[1][i][e] = 0;
		}
	}
	for (int i = 0; i < 12; i++) {
		for (int e = 0; e < 64; e++) {
			for (int f = 0; f < 12; f++) {
				for (int g = 0; g < 64; g++) {
					cmh[i][e][f][g] = 0;
				}
			}
		}
	}
}



int mvVal[13] = {
	0,13,6,4,3,1,1000,1000,1,3,4,6,13
};
int inNull = 0;
void updateHistoryMinus(int ply, int depth, bool side, int f, int t, int piece, int square) {
	if (depth >= 20) {
		return;
	}
	ht[side][f][t] -= (depth * depth + 5 * depth - 2) * ht[side][f][t] / HISTORYDIV;
	cmh[lMove[ply - 1][0]][lMove[ply - 1][1]][piece][square] -= (depth * depth + 5 * depth - 2) * cmh[lMove[ply - 1][0]][lMove[ply - 1][1]][piece][square] / HISTORYDIV;
}
void updateHistoryPlus(int ply, int depth, bool side, int f, int t, struct moveList* mvl, struct scores* score, struct position* pos) {
	if (depth >= 20) {
		return;
	}
	int ctr = 0;
	for (int i = 0; i < mvl->mam; i++) {
		if (ctr == mvl->gcapt) {
			ctr = 20;
		}
		if (score->score[ctr] == -mateScore) {
			updateHistoryMinus(ply, depth, side, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t, getPiece(pos, mvl->MOVE[ctr].f) - 1, mvl->MOVE[ctr].t);
		}
		ctr++;
	}
	ht[side][f][t] -= (depth*depth+5*depth-2) * ht[side][f][t] / HISTORYDIV;
	ht[side][f][t] += (depth * depth + 5 * depth - 2);
	cmh[lMove[ply-1][0]][lMove[ply - 1][1]][lMove[ply][0]][lMove[ply][1]] -= (depth * depth + 5 * depth - 2) * cmh[lMove[ply - 1][0]][lMove[ply - 1][1]][lMove[ply][0]][lMove[ply][1]] / HISTORYDIV;
	cmh[lMove[ply - 1][0]][lMove[ply - 1][1]][lMove[ply][0]][lMove[ply][1]] += (depth * depth + 5 * depth - 2);
}

bool nullStatus(struct position* pos) {

	if (pos->side) {
		if (__popcnt64(pos->bitBoard[8]) + __popcnt64(pos->bitBoard[9]) + __popcnt64(pos->bitBoard[10]) + 2*__popcnt64(pos->bitBoard[11])>1){
		return true;
		}
	}
	else {
		if (__popcnt64(pos->bitBoard[3]) +__popcnt64(pos->bitBoard[2]) + __popcnt64(pos->bitBoard[1]) + 2 * __popcnt64(pos->bitBoard[0]) > 1) {
			return true;
		}
	}

	return false;
}

int getLeastValuableAttacker(struct position* pos,unsigned long long wOcc, unsigned long long bOcc, bool side,unsigned long long* cmask, int sq) {
	
	if (side) {
		unsigned long long attacks = bPawnAttack(sq)&~*cmask;
		if (pos->bitBoard[7] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks&pos->bitBoard[7]));
			return mvVal[8];
		}
		attacks = knightAttack(sq) & ~*cmask;
		if (pos->bitBoard[8] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[8]));
			return mvVal[9];
		}
		attacks = bishopAttack((wOcc|bOcc)&~*cmask,sq) & ~*cmask;
		if (pos->bitBoard[9] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[9]));
			return mvVal[10];
		}

		attacks = rookAttack((wOcc | bOcc) & ~*cmask, sq) & ~*cmask;
		if (pos->bitBoard[10] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[10]));
			return mvVal[11];
		}
		attacks |= bishopAttack((wOcc | bOcc) & ~*cmask, sq) & ~*cmask;
		if (pos->bitBoard[11] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[11]));
			return mvVal[12];
		}

		attacks = kingAttack(sq) & ~*cmask;
		if (pos->bitBoard[6] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[6]));
			return mvVal[7];
		}
		return 0;
	}
	else {
		unsigned long long attacks = wPawnAttack(sq) & ~*cmask;
		if (pos->bitBoard[4] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[4]));
			return mvVal[5];
		}
		attacks = knightAttack(sq) & ~*cmask;
		if (pos->bitBoard[3] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[3]));
			return mvVal[4];
		}
		attacks = bishopAttack((wOcc | bOcc) & ~*cmask, sq) & ~*cmask;
		if (pos->bitBoard[2] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[2]));
			return mvVal[3];
		}

		attacks = rookAttack((wOcc | bOcc) & ~*cmask, sq) & ~*cmask;
		if (pos->bitBoard[1] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[1]));
			return mvVal[2];
		}
		attacks |= bishopAttack((wOcc | bOcc) & ~*cmask, sq) & ~*cmask;
		if (pos->bitBoard[0] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks & pos->bitBoard[0]));
			return mvVal[1];
		}

		attacks = kingAttack(sq) & ~*cmask;
		if (pos->bitBoard[5] & attacks) {
			*cmask |= getBit(_tzcnt_u64(attacks& pos->bitBoard[5]));
			return mvVal[6];
		}
		return 0;
	}
}

int see(struct position* pos, int from, int to) {

	unsigned long long cmask = getBit(from); //this mask is used so we can remove pieces from movegen in order to see xrays
	unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
	unsigned long long bOcc = pos->bitBoard[0] | pos->bitBoard[1] | pos->bitBoard[2] | pos->bitBoard[3] | pos->bitBoard[4] | pos->bitBoard[5];
	bool side = pos->side;

	int gain[32];
	int d = 0;

	int fp = getPiece(pos, from);
	int tp = getPiece(pos, to);
	fp = mvVal[fp];
	gain[d] = mvVal[tp];


	while (true) {
		d++;
		side = !side;
		int newP = getLeastValuableAttacker(pos, wOcc, bOcc, side, &cmask, to);
		if (newP) {
			gain[d] = fp - gain[d - 1];
			fp = newP;
		}
		else {
			break;
		}
		if (max(-gain[d - 1], gain[d]) < 0) { break; }
	}
	/*for (int i = 0; i < d + 2; i++) {
		std::cout << " " << gain[i]<< " " ;
	}*/
	while (d--) {
		gain[d - 1] = -max(-gain[d - 1], gain[d]);
	}
	return gain[0];
}


void scoreMVL(struct moveList* mvl, struct scores* score, int ply, struct position* pos, int depth) {
	int interesting = 0;

	int hashmove = -1;
	struct ttEntry ttEnt = ttProbe(pos->hash);
	if (ttEnt.zHash == pos->hash) {
		hashmove = ttEnt.move;
	}

	unsigned long long bcsq = 0;
	unsigned long long pcsq = 0;
	unsigned long long ncsq = 0;
	unsigned long long rcsq = 0;

	unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
	unsigned long long bOcc = pos->bitBoard[0] | pos->bitBoard[1] | pos->bitBoard[2] | pos->bitBoard[3] | pos->bitBoard[4] | pos->bitBoard[5];


	if (pos->side) {
		bcsq = bishopAttack(wOcc | bOcc, _tzcnt_u64(pos->bitBoard[5]));
		rcsq = rookAttack(wOcc | bOcc, _tzcnt_u64(pos->bitBoard[5]));
		ncsq = knightAttack(_tzcnt_u64(pos->bitBoard[5]));
		pcsq = bPawnAttack(_tzcnt_u64(pos->bitBoard[5]));
	}
	else {
		bcsq = bishopAttack(wOcc | bOcc, _tzcnt_u64(pos->bitBoard[6]));
		rcsq = rookAttack(wOcc | bOcc, _tzcnt_u64(pos->bitBoard[6]));
		ncsq = knightAttack(_tzcnt_u64(pos->bitBoard[6]));
		pcsq = wPawnAttack(_tzcnt_u64(pos->bitBoard[6]));
	}

	int ctr = 0;
	for (int i = 0; i < mvl->mam; i++) {
		if (ctr == mvl->gcapt) {
			ctr = 20;
		}
		score->quiet[ctr] = true;
		int tp = getPiece(pos, mvl->MOVE[ctr].t);
		int ft = getPiece(pos, mvl->MOVE[ctr].f);
		if (ply > 0) {
			score->score[ctr] = ht[pos->side][mvl->MOVE[ctr].f][mvl->MOVE[ctr].t] + cmh[lMove[ply - 1][0]][lMove[ply - 1][1]][ft - 1][mvl->MOVE[ctr].t];
		}
		else {
			score->score[ctr] = ht[pos->side][mvl->MOVE[ctr].f][mvl->MOVE[ctr].t];
		}

		if (ft == 5 && mvl->MOVE[ctr].t >= 5 * 8 && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
			score->score[ctr] += 3750000;
			score->quiet[ctr] = false;
		}

		if (ft == 8 && mvl->MOVE[ctr].t < 3 * 8 && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
			score->score[ctr] += 3750000;
			score->quiet[ctr] = false;
		}

		if (pos->side) {
			switch (ft) {
			case 8:
				if (getBit(mvl->MOVE[ctr].t) & pcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			case 9:
				if (getBit(mvl->MOVE[ctr].t) & ncsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			case 10:
				if (getBit(mvl->MOVE[ctr].t) & bcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			case 11:
				if (getBit(mvl->MOVE[ctr].t) & rcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			case 12:
				if (getBit(mvl->MOVE[ctr].t) & (rcsq | bcsq) && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			}
		}
		else {
			switch (ft) {
			case 5:
				if (getBit(mvl->MOVE[ctr].t) & pcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			case 4:
				if (getBit(mvl->MOVE[ctr].t) & ncsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			case 3:
				if (getBit(mvl->MOVE[ctr].t) & bcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			case 2:
				if (getBit(mvl->MOVE[ctr].t) & rcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			case 1:
				if (getBit(mvl->MOVE[ctr].t) & (rcsq | bcsq) && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					score->score[ctr] += 2500000;
					score->quiet[ctr] = false;
				}
				break;
			}
		}

		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == killers[ply][1]) {
			score->score[ctr] = 5000001;
			score->quiet[ctr] = false;
		}

		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == killers[ply][0]) {
			score->score[ctr] = 5000002;
			score->quiet[ctr] = false;
		}
		if (tp != 0) {
			if (mvVal[ft] < mvVal[tp] || see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t)>0) {
				score->score[ctr] = 7500000 + (mvVal[tp] * 8 - mvVal[ft]) * 1000;
				score->quiet[ctr] = false;
			}
			else if (mvVal[ft] == mvVal[tp]) {
				score->score[ctr] = 7000000 + (mvVal[tp] * 8 - mvVal[ft]) * 1000;
				score->quiet[ctr] = false;
			}
		}
		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == hashmove) {
			score->score[ctr] = 100000003;
			score->quiet[ctr] = false;
			//std::cout << "h" << hashmove;
		}
		ctr++;
	}
	return;
}

bool pickMove(struct moveList* mvl, struct scores* score, struct move* MOVE) {
	int bestId = 0;
	int bestScore = -mateScore;
	int ctr = 0;
	for (int i = 0; i < mvl->mam; i++) {
		if (ctr == mvl->gcapt) {
			ctr = 20;
		}
		if (score->score[ctr] > bestScore) {
			bestId = ctr;
			bestScore = score->score[ctr];
		}
		ctr++;
	}
	score->score[bestId] = -mateScore;
	*MOVE = mvl->MOVE[bestId];
	return score->quiet[bestId];
}

int orderMvl(struct moveList* mvl, int ply, struct position* pos, int depth) {
	int interesting = 0;

	int scores[100];
	int hashmove = -1;
	struct ttEntry ttEnt = ttProbe(pos->hash);
	if (ttEnt.zHash == pos->hash) {
		hashmove = ttEnt.move;
	}

	unsigned long long bcsq = 0;
	unsigned long long pcsq = 0;
	unsigned long long ncsq = 0;
	unsigned long long rcsq = 0;

	unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
	unsigned long long bOcc = pos->bitBoard[0] | pos->bitBoard[1] | pos->bitBoard[2] | pos->bitBoard[3] | pos->bitBoard[4] | pos->bitBoard[5];


	if (pos->side) {
		bcsq = bishopAttack(wOcc | bOcc, _tzcnt_u64(pos->bitBoard[5]));
		rcsq = rookAttack(wOcc | bOcc, _tzcnt_u64(pos->bitBoard[5]));
		ncsq = knightAttack(_tzcnt_u64(pos->bitBoard[5]));
		pcsq = bPawnAttack(_tzcnt_u64(pos->bitBoard[5]));
	}
	else {
		bcsq = bishopAttack(wOcc | bOcc, _tzcnt_u64(pos->bitBoard[6]));
		rcsq = rookAttack(wOcc | bOcc, _tzcnt_u64(pos->bitBoard[6]));
		ncsq = knightAttack(_tzcnt_u64(pos->bitBoard[6]));
		pcsq = wPawnAttack(_tzcnt_u64(pos->bitBoard[6]));
	}

	int ctr = 0;
	for (int i = 0; i < mvl->mam; i++) {
		bool interest = false;
		if (ctr == mvl->gcapt) {
			ctr = 20;
		}
		int tp = getPiece(pos, mvl->MOVE[ctr].t);
		int ft = getPiece(pos, mvl->MOVE[ctr].f);
		if (ply > 0) {
			scores[ctr] = ht[pos->side][mvl->MOVE[ctr].f][mvl->MOVE[ctr].t]+cmh[lMove[ply - 1][0]][lMove[ply - 1][1]][ft - 1][mvl->MOVE[ctr].t];
		}
		else {
			scores[ctr] = ht[pos->side][mvl->MOVE[ctr].f][mvl->MOVE[ctr].t];
		}

		if (ft == 5 && mvl->MOVE[ctr].t >= 5 * 8&&see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t)>=0) {
			scores[ctr] += 3750000;
			interest = true;
		}

		if (ft == 8 && mvl->MOVE[ctr].t < 3 * 8 && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
			scores[ctr] += 3750000;
			interest = true;
		}

		if (pos->side) {
			switch (ft) {
			case 8:
				if (getBit(mvl->MOVE[ctr].t) & pcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			case 9:
				if (getBit(mvl->MOVE[ctr].t) & ncsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			case 10:
				if (getBit(mvl->MOVE[ctr].t) & bcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			case 11:
				if (getBit(mvl->MOVE[ctr].t) & rcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			case 12:
				if (getBit(mvl->MOVE[ctr].t) & (rcsq | bcsq) && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			}
		}
		else {
			switch (ft) {
			case 5:
				if (getBit(mvl->MOVE[ctr].t) & pcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			case 4:
				if (getBit(mvl->MOVE[ctr].t) & ncsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			case 3:
				if (getBit(mvl->MOVE[ctr].t) & bcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			case 2:
				if (getBit(mvl->MOVE[ctr].t) & rcsq && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			case 1:
				if (getBit(mvl->MOVE[ctr].t) & (rcsq | bcsq) && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
					scores[ctr] += 2500000;
					interest = true;
				}
				break;
			}
		}

		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == killers[ply][1]) {
			scores[ctr] = 5000001;
			interest = true;
		}

		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == killers[ply][0]) {
			scores[ctr] = 5000002;
			interest = true;
		}
		if (tp != 0) {
			if (mvVal[ft] < mvVal[tp]|| see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t)>0) {
				scores[ctr] = 7500000 + (mvVal[tp] * 8 - mvVal[ft]) * 1000;

				interest = true;
			}
			else if(mvVal[ft] == mvVal[tp]) {
				scores[ctr] = 7000000 + (mvVal[tp] * 8 - mvVal[ft]) * 1000;
				interest = true;
			}
		}
		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == hashmove) {
			scores[ctr] = 100000003;
			interest = true;
			//std::cout << "h" << hashmove;
		}
		if (interest) { interesting++; }
		ctr++;
	}
	/*int maxo = min(interesting+2, mvl->mam);
	if (depth >= 3) {
		maxo = mvl->mam;
	}*/
	int maxo = mvl->mam;
	/*if (mvl->mam <= maxo + 1) {
		maxo = mvl->mam - 1;
	}
	if (maxo <= 3) {
		return mvl->mam;
	}*/
	ctr = 0;
	for (int i = 0; i < maxo; i++) {
		if (ctr == mvl->gcapt) {
			ctr = 20;
		}
		int bestid = 0;
		int bests = 0;
		int ctr2 = ctr;
		for (int e = 0; e < mvl->mam - i; e++) {
			if (ctr2 == mvl->gcapt) {
				ctr2 = 20;
			}

			if (bests <= scores[ctr2]) {
				bestid = ctr2;
				bests = scores[ctr2];
			}
			ctr2++;
		}

		int bf = mvl->MOVE[bestid].f;
		int bt = mvl->MOVE[bestid].t;
		int type = mvl->MOVE[bestid].type;


		mvl->MOVE[bestid] = mvl->MOVE[ctr];
		mvl->MOVE[ctr].f = bf;
		mvl->MOVE[ctr].t = bt;
		mvl->MOVE[ctr].type = type;
		scores[bestid] = scores[ctr];
		scores[ctr] = bests;

		ctr++;
	}
	return interesting;
}

void printCord(int cord) {
	int y = cord / 8;
	int x = cord - y * 8;
	switch (x) {
	case 0:
		std::cout << "a";
		break;
	case 1:
		std::cout << "b";
		break;
	case 2:
		std::cout << "c";
		break;
	case 3:
		std::cout << "d";
		break;
	case 4:
		std::cout << "e";
		break;
	case 5:
		std::cout << "f";
		break;
	case 6:
		std::cout << "g";
		break;
	case 7:
		std::cout << "h";
		break;
	}
	switch (y) {
	case 0:
		std::cout << "8";
		break;
	case 1:
		std::cout << "7";
		break;
	case 2:
		std::cout << "6";
		break;
	case 3:
		std::cout << "5";
		break;
	case 4:
		std::cout << "4";
		break;
	case 5:
		std::cout << "3";
		break;
	case 6:
		std::cout << "2";
		break;
	case 7:
		std::cout << "1";
		break;
	}
}

void printMove(struct move MOVE, struct position pos) {
	int f = MOVE.f;
	int t = MOVE.t;
	if (MOVE.type != 0) {
		switch (MOVE.type) {
		case 1:
			f = 60;
			t = 62;
			break;
		case 2:
			f = 60;
			t = 58;
			break;
		case 3:
			f = 4;
			t = 6;
			break;
		case 4:
			f = 4;
			t = 2;
			break;
		}
	}

	printCord(f);
	printCord(t);

	if ((getPiece(&pos, f) == 5 && t > 55)) {
		std::cout << "q";
	}
	if ((getPiece(&pos, f) == 8 && t < 8)) {
		std::cout << "q";
	}
	std::cout << " ";
}

void printpv(struct position pos, int ply) {
	int hashmove = 0;
	struct ttEntry ttEnt = ttProbe(pos.hash);
	if (ttEnt.zHash == pos.hash) {
		hashmove = ttEnt.move;
	}
	
	if (hashmove) {
		struct move MOVE = ttMoveToMove(hashmove);
		struct position pos2 = makeMove(MOVE, pos);
		if (isLegal(pos.side, &pos2)) {
			printMove(MOVE, pos);
			ply--;
			if (ply < 0) {
				return;
			}
			printpv(pos2, ply);
		}
	}
}

void infoString(struct move MOVE, int depth, int score, int nodes, struct position* pos, struct search* s, int scoretype) {

	unsigned long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	unsigned long long sTime = s->sTime;
	int tTime = currentTime - sTime + 1;

	if (abs(score) > mateScore - 1000) {
		int side = score;
		score =( mateScore - abs(score)+1 )/ 2;
		if (side < 0) {
			score = -1 * score;
		}
		std::cout << "info depth " << depth << " seldepth " << selDepth << " score mate " << score << " nodes " << nodes << " nps " << nodes / tTime *1000 << " time " << tTime << " pv ";
	}
	else {
		if (scoretype == 0) {
			std::cout << "info depth " << depth <<" seldepth " << selDepth <<  " score cp " << score / 10 << " nodes " << nodes << " nps " << nodes / tTime * 1000 << " time " << tTime << " pv ";
		}
		else {
			if (scoretype == 1) {
				std::cout << "info depth " << depth << " seldepth " << selDepth << " score cp " << score / 10 << " lowerbound nodes " << nodes << " nps " << nodes / tTime *1000 << " time " << tTime << " pv ";
			}
			if (scoretype == 2) {
				std::cout << "info depth " << depth << " seldepth " << selDepth << " score cp " << score / 10 << " upperbound nodes " << nodes << " nps " << nodes / tTime *1000 << " time " << tTime << " pv ";
			}
		}
	}
	
	
	printpv(*pos, depth);


	std::cout << std::endl;

	fflush(stdout);

	return;
}

int Quis(struct position pos, int alpha, int beta, int ply, struct QTable* ct) {
	if (ply >= 10) {
		return eval(&pos);
	}

	int staticEval = eval(&pos);
	if (staticEval >= alpha) {
		alpha = staticEval;
	}
	if (alpha >= beta) {
		return alpha;
	}
	//delta pruning
	/*if (staticEval < alpha - queenMiddleGame-300) {
		return alpha;
	}*/
	genAllCaptures(&ct->QL[ply], pos.side, &pos);
	for (int i = 0; i < ct->QL[ply].mam; i++) {
			int score = -Quis(makeMove(ct->QL[ply].MOVE[i], pos), -beta, -alpha, ply + 1, ct);
			if (score > staticEval) {
				staticEval = score;
				if (staticEval >= alpha) {
					alpha = staticEval;
				}
				if (alpha >= beta) {
					return alpha;
				}
		}
	}
	return staticEval;
}
int pvs(struct search* s, struct position pos, bool pvnode, int alpha, int beta, int depth, int ply, struct moveTable* mt, struct QTable* ct, struct historyhash* hh, int skipMove) {
	selDepth = max(selDepth, ply);
	//if (!isLegal(pos.side, &pos)) { depth += 1; }


	if (!pos.bitBoard[5]) {
		if (pos.side) {
			return wMateScore - ply;
		}
		else {
			return bMateScore + ply;
		}
	}
	if (!pos.bitBoard[6]) {
		if (pos.side) {
			return bMateScore + ply;
		}
		else {
			return wMateScore - ply;
		}
	}
	if (ply>=90 || depth <= 0) {
		if (!isLegal(pos.side, &pos)) { depth = 1; }
		else {
			s->nodeCount++;
			return Quis(pos, alpha, beta, 0, ct);
		}
	}

	//mate score pruning
	if (beta > mateScore - ply) {
		beta = mateScore - ply;
		if (alpha >= mateScore - ply) {
			return mateScore - ply;
		}
	}
	if (alpha < -mateScore + ply) {
		alpha = -mateScore + ply;
		if (beta <= -mateScore + ply) {
			return -mateScore + ply;
		}
	}


	if (!s->searching) {
		return 0;
	}

	for (int i = 0; i < pos.mov50; i++) {
		//std::cout << "yesss";
		if (pos.hash == hh->hh[hh->index + ply - 2 - i]) {
			return 0;
		}
	}

	hh->hh[hh->index + ply-1] = pos.hash;
	if (!isLegal(!pos.side, &pos)) {
		return mateScore - ply;
	}


	struct ttEntry ttEnt = ttProbe(pos.hash);
	if (!skipMove) {
		if (pos.hash == ttEnt.zHash) {
			if (ttEnt.depth >= depth) {
				int ttev = ttToMate(ply, tt[pos.hash % ttSize].eval);
				if (ttEnt.type == 0) {
					return  ttev;
				}
				if (ttEnt.type == 2) {
					if (ttev <= alpha) {
						return ttev;
					}
				}
				if (ttEnt.type == 1) {
					if (ttev >= beta) {
						return ttev;
					}
				}
			}
		}
	}

	bool isdraw = isLegal(pos.side, &pos);
	bool incheck = !isdraw;

	int staticEval = eval(&pos);

	//razoring
	if (!pvnode && depth <= RAZOR_DEPTH && staticEval + RAZOR_MARGIN < beta && !incheck)
	{
		int score = Quis(pos, alpha, beta, 0, ct);
		if (score < beta) return score;
	}
	//futility pruning
	if (!pvnode && !incheck && depth <= FUTILITY_DEPTH && staticEval >= beta + FUTILITY_MARGIN * depth) {
		return staticEval;
	}
	//NULL MOVE STUFF
	if (!skipMove && depth >= 2 &&  !pvnode && !incheck && !inNull && nullStatus(&pos) && staticEval > beta) {
		makeNull(&pos);
		inNull++;

		int score = alpha;
		score = -pvs(s, pos, false, -beta, -alpha, depth - 4, ply+1, mt, ct, hh, 0);
		

		makeNull(&pos);
		inNull--;

		if (score >= beta) {
			return score;
		}

	}
	
	//IID
	if (!skipMove && depth >= IID_DEPTH && pvnode &&pos.hash != tt[pos.hash % ttSize].zHash) {
		pvs(s, pos, pvnode, alpha, beta, depth-2, ply, mt, ct, hh, 0);
		ttEnt = ttProbe(pos.hash);
	}


	int bs = -mateScore;
	int bm = 0;
	int type = 2;

	genAllMoves(&mt->mvl[ply], pos.side, &pos);
	scoreMVL(&mt->mvl[ply], &mt->score[ply], ply, &pos, depth);
	int ctr = 0;
	struct move nextMove;
	for (int i = 0; i < mt->mvl[ply].mam; i++) {
		if (ctr == mt->mvl[ply].gcapt) {
			ctr = 20;
		}
		bool quiet = pickMove(&mt->mvl[ply],&mt->score[ply], &nextMove);
		int extension = 0;
		if (i == 0 && extension == 0 && ttEnt.move == nextMove.f + nextMove.t * 100&& depth >= 6 && !skipMove && ttEnt.zHash == pos.hash && (ttEnt.type == 1||ttEnt.type==0) && ttEnt.depth >= depth - 3) {
			int betaCut = ttEnt.eval - depth * 20;
			int score = pvs(s, pos, false, betaCut - 1, betaCut, depth-4, ply, mt, ct, hh, nextMove.f + nextMove.t * 100);
			if (score < betaCut) {
				extension = 1;
			}
			scoreMVL(&mt->mvl[ply], &mt->score[ply], ply, &pos, depth);
			quiet = pickMove(&mt->mvl[ply], &mt->score[ply], &nextMove);

			//implement multi-cut?

		}

		if (skipMove == nextMove.f + nextMove.t * 100) { ctr++; continue; }


		if (!pvnode && quiet&& depth <= LMP_DEPTH && i >= lmp[depth] && !incheck && bs>-mateScore+100) {
			continue;
		}
		struct position pos2 = makeMove(nextMove, pos);
		lMove[ply][0] = getPiece(&pos, nextMove.f)-1;
		lMove[ply][1] = nextMove.t;
		int score = -mateScore;
		if (isLegal(pos.side, &pos2)) {
			if (!quiet && !isLegal(pos2.side, &pos2)) {
				extension = 1;
			}

			int lmr = 0;
			if (depth >= 3 && !extension && isLegal(pos2.side, &pos2)&& i > 0 && quiet && !incheck) {
				lmr =  lmrReductions[depth][i];
				if (getPiece(&pos, nextMove.t)) {
					lmr = min(lmr, 2);
				}
				//lmr -= ht[pos.side][nextMove.f][nextMove.t] / 400;
				if (ttEnt.type == 1)lmr++;
				lmr = max(min(lmr, depth - 2), 0); // do not drop into qsearch & do not extend
			}
			if (pvnode) {
				if (i == 0 || incheck) {
					score = -pvs(s, pos2, true, -beta, -alpha, depth - 1 + extension, ply + 1, mt, ct, hh,0);
				}
				else {
					score = -pvs(s, pos2, false, -alpha - 1, -alpha, depth - 1-lmr + extension, ply + 1, mt, ct, hh,0);
					if (score > alpha) {
						score = -pvs(s, pos2, true, -beta, -alpha, depth - 1 + extension, ply + 1, mt, ct, hh,0);
					}
				}
			}
			else {
				score = -pvs(s, pos2, false, -beta, -alpha, depth - 1-lmr + extension, ply + 1, mt, ct, hh,0);
				if (lmr&&score>alpha){
					score = -pvs(s, pos2, false, -beta, -alpha, depth - 1 + extension, ply + 1, mt, ct, hh,0);
				}
			}
			isdraw = false;
			incheck = false;
		}
		if (!s->searching) {
			return 0;
		}
		if (score > bs) {
			bs = score;
			bm = nextMove.f + 100 * nextMove.t;
		}

		if (bs > alpha) {
			alpha = bs;
			type = 0;
			if (alpha >= beta) {
				if (!skipMove) {
					updateHistoryPlus(ply, depth, pos.side, nextMove.f, nextMove.t, &mt->mvl[ply], &mt->score[ply], &pos);
					killers[ply][1] = killers[ply][0];
					killers[ply][0] = bm;
					ttSave(depth, pos.hash, mateToTT(ply, bs), 1, bm, pvnode);
				}
				return alpha;
			}
		}
		ctr++;
	}
	if (isdraw) {
		return 0;
	}
	else {
		if (!skipMove)ttSave(depth, pos.hash, mateToTT(ply, bs), type, bm, pvnode);

		if (incheck) {
			return -mateScore + ply;
		}
		return bs;
	}
}

int aspiration(int lastScore , struct search* s, struct position ogpos, struct position pos, bool pvnode, int hasalpha, int hasbeta, int depth, int ply, struct moveTable* mt, struct QTable* ct, struct historyhash* hh, int ctr, struct move nextMove) {



	int delta = 200;
	int alpha = max(lastScore - delta, -mateScore);
	alpha = max(alpha, hasalpha);
	int beta = min(lastScore + delta, mateScore);
	beta = min(beta, hasbeta);
	int score = 0;
		while (true)
		{
			
				score = -pvs(s, pos, true, -beta, -alpha, depth - 1, ply + 1, mt, ct, hh,0);

				if (s->searching == false) {
					return 0;
				}

				// In case of failing low/high increase aspiration window and
				// re-search, otherwise exit the loop.

				if (score >= hasbeta ||score<=hasalpha) {
					break;
				}

				if (score <= alpha)
				{
					beta = (alpha + beta) / 2;
					alpha = max(score - delta, -mateScore);
					alpha = max(alpha, hasalpha);

				}
				else if (score>= beta)
				{
					alpha = (alpha + beta) / 2;
					beta = min(score + delta, mateScore);
					beta = min(beta, hasbeta);

					//report move
					struct move bm = nextMove;
					s->bff = bm.f;
					s->bft = bm.t;

					ttSave(depth, ogpos.hash, mateToTT(ply,score), 1, bm.f + bm.t * 100, true);
					if (depth >= 12) {
						infoString(nextMove, depth, score, s->nodeCount, &ogpos, s, 1);
					}
				}
				else
					break;
				delta += delta / 4 + 50;
		}
		return score;
}

void mainSearch(struct search* s, struct position* pos, struct historyhash hh) {

	selDepth = 0;

	for (int i = 0; i < 64; i++) {
		for (int e = 0; e < 64; e++) {
			ht[0][i][e] = ht[0][i][e] = 0;
			ht[1][i][e] = ht[1][i][e] = 0;
		}
	}
	initReductionTable();
	//std::cout << pos->hash;
	struct moveTable tb;
	struct moveTable* mt = &tb;
	struct QTable qt;
	struct QTable* ct = &qt;
	genAllMoves(&mt->mvl[0], pos->side, pos);
	int alpha = -mateScore;
	int beta = mateScore;
	int ply = 0;
	inNull = 0;
	int lastScore = 0;
	for (int depth = 2; depth < 100; depth++) {
		int bs = -mateScore;
		struct move bm;

		int ctr = 0;

		scoreMVL(&mt->mvl[ply], &mt->score[ply], 0, pos, depth);

		for (int i = 0; i < mt->mvl[ply].mam; i++) {

			if (ctr == mt->mvl[ply].gcapt) {
				ctr = 20;
			}
			int score = -mateScore;
			struct move nextMove;
			bool quiet = pickMove(&mt->mvl[ply], &mt->score[ply], &nextMove);
			struct position pos2 = makeMove(nextMove, *pos);
			lMove[ply][0] = getPiece(pos, nextMove.f) - 1;
			lMove[ply][1] = nextMove.t;
			if (isLegal(pos->side, &pos2)) {
				if (i == 0) {
					if (depth > 4) {
						score = aspiration(lastScore, s,*pos, pos2, true, alpha, beta, depth, ply, mt, ct, &hh, ctr, nextMove);
					}
					else {
						score = -pvs(s, pos2, true, -beta, -bs, depth - 1, ply + 1, mt, ct, &hh,0);
					}
					//score = -pvs(s, pos2, true, -beta, -bs, depth - 1, ply + 1, mt, ct, &hh);
				}
				else {
					int extension = 0;
					if (!quiet && !isLegal(pos2.side, &pos2)) {
						extension = 1;
					}

					/*int lmr = 0;
					if (depth >= 3 && i >= interesting && i > 0) {
						lmr = sqrt(double(depth - 1)) + sqrt(double(i - 1));
						lmr = min(lmr, 4);
						if (getPiece(pos, mt->mvl[ply].MOVE[ctr].t)) {
							lmr = min(lmr, 2);
						}
					}*/



					int lmr = 0;
					if (depth >= 0 && quiet && !extension&& i > 0) {
						lmr = lmrReductions[depth][i];
						if (getPiece(pos, nextMove.t)) {
							lmr = min(lmr, 2);
						}
					}
					score = -pvs(s, pos2, false, -bs - 1, -bs, depth - 1 - lmr, ply + 1, mt, ct, &hh,0);
					if (score > bs) {
						if (depth > 4) {
							score = aspiration(lastScore, s, *pos, pos2, true,lastScore, beta, depth, ply, mt, ct, &hh, ctr, nextMove);
						}
						else {
							score = -pvs(s, pos2, true, -beta, -bs, depth - 1, ply + 1, mt, ct, &hh,0);
						}
					}
				}


				if (!s->searching) {
					goto end;
				}

				if (score > bs) {
					bs = score;
					lastScore = bs;
					bm = nextMove;
					killers[0][1] = killers[0][0];
					killers[0][0] = nextMove.f + 100 * nextMove.t;
					s->bff = bm.f;
					s->bft = bm.t;
					ttSave(depth, pos->hash, mateToTT(ply, bs), 0, bm.f + bm.t * 100, true);
					if (depth >= 7) {
						infoString(nextMove, depth, bs, s->nodeCount, pos, s, 0);
					}


				}
			}

			ctr++;
		}
		s->bff = bm.f;
		s->bft = bm.t;
		s->reacheddepth = depth;
		if (s->depth <= s->reacheddepth) {

			goto end;
		};
	}

end:
	return;
};

void searchManager(struct search* s) {
	while (1) {
		if (s->searching) {
			unsigned long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

			unsigned long long inc = s->inc;
			unsigned long long sTime = s->sTime;
			unsigned long long UsableTime = s->time;
			unsigned long long moves = s->movesToGo;
			switch (s->timetype) {
			case 0:
				if (s->depth <= s->reacheddepth) {
					s->searching = false;
					printBestMove(s->bff, s->bft, getPositionPointer());
					ageTT();
				}
				break;
			case 1:
				if (currentTime >= sTime + (UsableTime + inc * (moves - 1)) / moves) {
					s->searching = false;
					printBestMove(s->bff, getSearchPointer()->bft, getPositionPointer());
					ageTT();
				}
				break;
			case 2:
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}