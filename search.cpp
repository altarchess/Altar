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

#define RAZOR_DEPTH 3
#define FUTILITY_DEPTH 5
#define IID_DEPTH 5

#define RAZOR_MARGIN 2000
#define FUTILITY_MARGIN 800

int max(int a, int b) {
	return b < a ? a : b;
}

int killers[100][2];
int ht[2][64][64];
int cmh[2][64][64];
int pvTable[100][100];


void resetHistory() {
	for (int i = 0; i < 64; i++) {
		for (int e = 0; e < 64; e++) {
			ht[0][i][e] = 0;
			ht[1][i][e] = 0;
		}
	}
}
int mvVal[13] = {
	0,13,6,4,3,1,1000,1000,1,3,4,6,13
};
int inNull = 0;

int nullStatus(struct position* pos) {
	int wm = __popcnt64(pos->bitBoard[7]) * pawnMiddleGame + __popcnt64(pos->bitBoard[8]) * knightMiddleGame + __popcnt64(pos->bitBoard[9]) * bishopMiddleGame + __popcnt64(pos->bitBoard[10]) * rookMiddleGame + __popcnt64(pos->bitBoard[11]) * queenMiddleGame;
	int bm = __popcnt64(pos->bitBoard[4]) * pawnMiddleGame + __popcnt64(pos->bitBoard[3]) * knightMiddleGame + __popcnt64(pos->bitBoard[2]) * bishopMiddleGame + __popcnt64(pos->bitBoard[1]) * rookMiddleGame + __popcnt64(pos->bitBoard[0]) * queenMiddleGame;

	if ((wm + bm) / 256 > 30) {
		return 1;
	}

	return 0;
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
	
	unsigned long long cmask = 0; //this mask is used so we can remove pieces from movegen in order to see xrays
	unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
	unsigned long long bOcc = pos->bitBoard[0] | pos->bitBoard[1] | pos->bitBoard[2] | pos->bitBoard[3] | pos->bitBoard[4] | pos->bitBoard[5];
	bool side = pos->side;

	int gain[32];
	int d = 0;

	int fp = getPiece(pos, from);
	int tp = getPiece(pos, to);

	gain[d] = mvVal[tp];


	while (true) {
		d++;
		side = !side;
		int newP = getLeastValuableAttacker(pos, wOcc, bOcc, side, &cmask, to);
		if (newP) {
			gain[d] = mvVal[fp] - gain[d - 1];
			fp = newP;
		}
		else {
			break;
		}
		if (max(-gain[d - 1], gain[d]) < 0) { break; }
	}
	while (d--) {
		gain[d - 1] = -max(-gain[d - 1], gain[d]);
	}
	return gain[0];
}

int orderMvl(struct moveList* mvl, int ply, struct position* pos) {
	int interesting = 0;

	int scores[100];
	int hashmove = -1;
	if (tt[pos->hash % ttSize].zHash == pos->hash) {
		hashmove = tt[pos->hash % ttSize].move;
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

		scores[ctr] = ht[pos->side][mvl->MOVE[ctr].f][mvl->MOVE[ctr].t];

		if (ft == 5 && mvl->MOVE[ctr].t >= 4 * 8&&see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t)>=0) {
			scores[ctr] = 3750000 + ht[pos->side][mvl->MOVE[ctr].f][mvl->MOVE[ctr].t];
			interest = true;
		}

		if (ft == 8 && mvl->MOVE[ctr].t < 4 * 8 && see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t) >= 0) {
			scores[ctr] = 3750000 + ht[pos->side][mvl->MOVE[ctr].f][mvl->MOVE[ctr].t];
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
			if (mvVal[ft] <= mvVal[tp]|| see(pos, mvl->MOVE[ctr].f, mvl->MOVE[ctr].t)>0) {
				scores[ctr] = 7500000 + (mvVal[tp] * 8 - mvVal[ft]) * 1000;
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
	int maxo = std::min(interesting+3, mvl->mam);
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
	if (tt[pos.hash % ttSize].zHash == pos.hash) {
		hashmove = tt[pos.hash % ttSize].move;
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

void infoString(struct move MOVE, int depth, int score, int nodes, struct position* pos, struct search* s) {

	unsigned long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	unsigned long long sTime = s->sTime;
	int tTime = currentTime - sTime + 1;

	if (abs(score) > mateScore - 1000) {
		int side = score;
		score =( mateScore - abs(score) )/ 2;
		if (side < 0) {
			score = -1 * score;
		}
		std::cout << "info depth " << depth << " score mate " << score << " nodes " << nodes << " nps " << nodes / tTime << " time " << tTime << " pv ";
	}
	else {
		std::cout << "info depth " << depth << " score cp " << score / 10 << " nodes " << nodes << " nps " << nodes / tTime << " time " << tTime << " pv ";
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
int pvs(struct search* s, struct position pos, bool pvnode, int alpha, int beta, int depth, int ply, struct moveTable* mt, struct QTable* ct, struct historyhash* hh) {

	//check extension seems to loose elo at 30s + 0.3s?
	//if (!isLegal(pos.side, &pos)) { depth += 1; }
	if (depth == 0) {
		if (!isLegal(pos.side, &pos)) { depth += 1; }
		else {
			s->nodeCount++;
			return Quis(pos, alpha, beta, 0, ct);
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

	hh->hh[hh->index + ply - 1] = pos.hash;
	if (!isLegal(!pos.side, &pos)) {
		return mateScore - ply;
	}



	if (pos.hash == tt[pos.hash % ttSize].zHash) {
		if (tt[pos.hash % ttSize].depth >= depth) {
			if (tt[pos.hash % ttSize].type == 0) {
				return  tt[pos.hash % ttSize].eval;
			}
			if (tt[pos.hash % ttSize].type == 2 && tt[pos.hash % ttSize].eval < alpha) {
				return tt[pos.hash % ttSize].eval;
			}
			if (tt[pos.hash % ttSize].type == 1 && tt[pos.hash % ttSize].eval > beta) {
				return tt[pos.hash % ttSize].eval;
			}
		}
	}


	bool isdraw = isLegal(pos.side, &pos);
	bool incheck = !isdraw;

	int staticEval = std::max(alpha,eval(&pos));

	//razoring
	if (!pvnode && depth <= RAZOR_DEPTH && staticEval + RAZOR_MARGIN < beta)
	{
		int score = Quis(pos, alpha, beta, 0, ct);
		if (score < beta) return score;
	}
	//futility pruning
	if (!pvnode && !incheck && depth <= FUTILITY_DEPTH && staticEval >= beta + FUTILITY_MARGIN * depth) {
		return staticEval;
	}
	//NULL MOVE STUFF
	if (!pvnode && !incheck && !inNull && nullStatus(&pos) && staticEval >= beta) {
		makeNull(&pos);
		inNull++;

		int score = alpha;

		if (depth <= 3) {
			score = -Quis(pos, -beta, -beta+1, 0, ct);
		}
		else {
			score = -pvs(s, pos, false, -beta, -beta + 1, depth - 4, ply, mt, ct, hh);
		}

		makeNull(&pos);
		inNull--;

		if (score >= beta) {
			return score;
		}

	}

	//IID
	if (depth > IID_DEPTH && pvnode && pos.hash == tt[pos.hash % ttSize].zHash && tt[pos.hash % ttSize].move <= 0) {
		-pvs(s, pos, true, -beta, -alpha, depth/2, ply, mt, ct, hh);
	}

	int bs = -1999999;
	int bm = 0;
	int type = 2;

	genAllMoves(&mt->mvl[ply], pos.side, &pos);
	int interesting = orderMvl(&mt->mvl[ply], ply, &pos);

	int ctr = 0;
	for (int i = 0; i < mt->mvl[ply].mam; i++) {
		if (ctr == mt->mvl[ply].gcapt) {
			ctr = 20;
		}
		struct position pos2 = makeMove(mt->mvl[ply].MOVE[ctr], pos);
		int score = alpha - 10000;
		if (isLegal(pos.side, &pos2)) {
			isdraw = false;
			incheck = false;
			int extension = 0;
			if (i < interesting && !isLegal(pos2.side, &pos2)) {
				//extension = 1;
			}
			if (pvnode) {
				if (i == 0) {
					score = -pvs(s, pos2, true, -beta, -alpha, depth - 1 + extension, ply + 1, mt, ct, hh);
				}
				else {
					int lmr = 0;
					if (depth > 3 && i >= interesting) {
						lmr = 2;
					}
					score = -pvs(s, pos2, false, -alpha - 1, -alpha, depth - 1-lmr + extension, ply + 1, mt, ct, hh);
					if (score > alpha) {
						score = -pvs(s, pos2, true, -beta, -alpha, depth - 1 + extension, ply + 1, mt, ct, hh);
					}
				}
			}
			else {
				int lmr = 0;
				if (depth > 3 && i >= interesting) {
					lmr = 2;
				}
				score = -pvs(s, pos2, false, -beta, -alpha, depth - 1-lmr + extension, ply + 1, mt, ct, hh);
				if (lmr&&score>alpha){
					score = -pvs(s, pos2, false, -beta, -alpha, depth - 1 + extension, ply + 1, mt, ct, hh);
				}
			}
		}
		if (!s->searching) {
			return 0;
		}
		if (score > bs) {
			bs = score;
			bm = mt->mvl[ply].MOVE[ctr].f + 100 * mt->mvl[ply].MOVE[ctr].t;
		}

		if (bs > alpha) {
			alpha = bs;
			type = 0;
			if (alpha >= beta) {
				ht[pos.side][mt->mvl[ply].MOVE[ctr].f][mt->mvl[ply].MOVE[ctr].t] += depth * depth;
				killers[ply][1] = killers[ply][0];
				killers[ply][0] = bm;
				ttSave(depth, pos.hash, bs, 1, bm);
				return alpha;
			}
		}

		ctr++;
	}
	ttSave(depth, pos.hash, bs, type, bm);
	if (isdraw) {
		return 0;
	}
	else {
		if (incheck) {
			return -mateScore + ply;
		}
		return bs;
	}
}

int aspiration(int lastScore , struct search* s, struct position pos, bool pvnode, int alpha, int beta, int depth, int ply, struct moveTable* mt, struct QTable* ct, struct historyhash* hh) {
	int binc = 50;
	int ainc = 50;
	while (true)
	{
		int score = -pvs(s, pos, true, -lastScore-binc, -lastScore+ainc, depth - 1, ply + 1, mt, ct, hh);
		if (lastScore - ainc < score && score < lastScore + binc) {
			return score;
		}
		else {
			if (score >= lastScore + binc) {
				binc = binc * 2;
				if (binc >= 1500) {
					binc = 1999999;
				}
			}
			if (score <= lastScore - ainc) {
				ainc = ainc * 2;
				if (ainc >= 1500) {
					ainc = 1999999;
				}
			}
		}
	}

}

void mainSearch(struct search* s, struct position* pos, struct historyhash hh) {
	for (int i = 0; i < 64; i++) {
		for (int e = 0; e < 64; e++) {
			ht[0][i][e] = ht[0][i][e]/2;
			ht[1][i][e] = ht[1][i][e] / 2;
		}
	}
	for (int i = 0; i < 100; i++) {
	}
	int interesting = 0;
	//std::cout << pos->hash;
	struct moveTable tb;
	struct moveTable* mt = &tb;
	struct QTable qt;
	struct QTable* ct = &qt;
	genAllMoves(&mt->mvl[0], pos->side, pos);
	int alpha = -1999999;
	int beta = 1999999;
	int ply = 0;
	int lastScore = 0;
	for (int depth = 2; depth < 30; depth++) {

		int bs = -1999999;
		struct move bm;

		int ctr = 0;
		for (int i = 0; i < mt->mvl[ply].mam; i++) {

			if (ctr == mt->mvl[ply].gcapt) {
				ctr = 20;
			}
			int score = -19999999;
			struct position pos2 = makeMove(mt->mvl[ply].MOVE[ctr], *pos);
			if (isLegal(pos->side, &pos2)) {
				if (i == 0) {
					/*if (depth > 4) {
						score = aspiration(lastScore, s, pos2, true, alpha, beta, depth, ply, mt, ct, &hh);
					}
					else {
						score = -pvs(s, pos2, true, -beta, -bs, depth - 1, ply + 1, mt, ct, &hh);
					}*/
					score = -pvs(s, pos2, true, -beta, -bs, depth - 1, ply + 1, mt, ct, &hh);
				}
				else {
					int lmr = 0;
					if (depth >= 3 && i >= interesting) {
						lmr = 2;
					}
					score = -pvs(s, pos2, false, -bs - 1, -bs, depth - 1 - lmr, ply + 1, mt, ct, &hh);
					if (score > bs) {
						score = -pvs(s, pos2, true, -beta, -bs, depth - 1, ply + 1, mt, ct, &hh);
					}
				}


				if (!s->searching) {
					goto end;
				}

				if (score > bs) {
					bs = score;
					bm = mt->mvl[ply].MOVE[ctr];
					ht[pos->side][mt->mvl[ply].MOVE[ctr].f][mt->mvl[ply].MOVE[ctr].t] += depth * depth;
					killers[0][1] = killers[0][0];
					killers[0][0] = mt->mvl[ply].MOVE[ctr].f + 100 * mt->mvl[ply].MOVE[ctr].t;
					s->bff = bm.f;
					s->bft = bm.t;
					ttSave(depth, pos->hash, bs, 0, bm.f + bm.t * 100);
					infoString(mt->mvl[ply].MOVE[ctr], depth, bs, s->nodeCount, pos,s);


				}
			}

			ctr++;
		}
		s->bff = bm.f;
		s->bft = bm.t;
		s->reacheddepth = depth;
		interesting = orderMvl(&mt->mvl[0], 0, pos);
		lastScore = bs;
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
				}
				break;
			case 1:
				if (currentTime >= sTime + (UsableTime + inc * (moves - 1)) / moves) {
					s->searching = false;
					printBestMove(s->bff, getSearchPointer()->bft, getPositionPointer());
				}
				break;
			case 2:
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}