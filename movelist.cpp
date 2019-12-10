#include "movelist.h"
#include "bitboard.h"
#include "engine.h"
#include "gen.h"
#include "move.h"

#include <immintrin.h>
#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int cVal[13] = {
	0,13,6,4,3,1,15,15,1,3,4,6,13
};

void genAllMoves(struct moveList* mvl, bool side, struct position* calcPos) {

	mvl->enPassant = calcPos->enPassant;
	mvl->castle[0] = calcPos->castle[0];
	mvl->castle[1] = calcPos->castle[1];
	mvl->castle[2] = calcPos->castle[2];
	mvl->castle[3] = calcPos->castle[3];
	mvl->hashM = 1;
	int hashm = 0;
	int killer1 = 0;
	int killer2 = 0;
	int gcCounter = 0;
	int othersCounter = 0;

	unsigned long long wOcc = calcPos->bitBoard[6] | calcPos->bitBoard[7] | calcPos->bitBoard[8] | calcPos->bitBoard[9] | calcPos->bitBoard[10] | calcPos->bitBoard[11];
	unsigned long long bOcc = calcPos->bitBoard[0] | calcPos->bitBoard[1] | calcPos->bitBoard[2] | calcPos->bitBoard[3] | calcPos->bitBoard[4] | calcPos->bitBoard[5];

	for (int i = 0; i < 100; i++) {
		mvl->MOVE[i].type = 0;
		mvl->MOVE[i].f = 0;
		mvl->MOVE[i].t = 0;
	}

	if (side) {

		unsigned long long wPawns = calcPos->bitBoard[7];
		
		if (calcPos->enPassant) {
			unsigned long long cord = _tzcnt_u64(calcPos->enPassant) + N * 4;
			if (cord == 32) {
				goto noEn;
			}
			int cordy = cord / 8;
			int cordx = cord - cordy*8;
			if (cordx > 0) {
				if (calcPos->bitBoard[7]&getBit(cord+SW)) {
					mvl->MOVE[gcCounter].f = cord + SW;
					mvl->MOVE[gcCounter].t = cord;
					gcCounter++;
				}
			}
			if (cordx < 7) {
				if (calcPos->bitBoard[7] & getBit(cord + SE)) {
					mvl->MOVE[gcCounter].f = cord + SE;
					mvl->MOVE[gcCounter].t = cord;
					gcCounter++;
				}
			}
		}
		noEn:
		int range = __popcnt64(wPawns);
		for (int i = 0; i < range ; i++) {
			unsigned long long cord = _tzcnt_u64(wPawns);
			unsigned long long cordy = cord / 8;
			unsigned long long cordx = cord - cordy*8;
			if (cordy > 0 && !(bOcc & getBit(cord - 8)) && !(wOcc & getBit(cord - 8))) {
				mvl->MOVE[20+othersCounter].f = cord;
				mvl->MOVE[20+othersCounter].t = cord-8;
				othersCounter++;
				if (cordy > 5 && !(bOcc & getBit(cord - 16)) && !(wOcc & getBit(cord - 16))) {
					mvl->MOVE[20 + othersCounter].f = cord;
					mvl->MOVE[20 + othersCounter].t = cord - 16;
					othersCounter++;
				}
			}
			if (cordx > 0 && cordy > 0) {
				if (bOcc& getBit(cord+NW)) {
					mvl->MOVE[gcCounter].f = cord;
					mvl->MOVE[gcCounter].t = cord+NW;
					gcCounter++;
				}
			}
			if (cordx < 7 && cordy > 0) {
				if (bOcc & getBit(cord + NE)) {
					mvl->MOVE[gcCounter].f = cord;
					mvl->MOVE[gcCounter].t = cord + NE;
					gcCounter++;

				}
			}
			wPawns &= ~getBit(cord);
			
		};
		unsigned long long wKnights = calcPos->bitBoard[8];
		range = __popcnt64(wKnights);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wKnights);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;
			
			unsigned long long capt = knightAttack(cord) & bOcc & ~wOcc;
			unsigned long long other = knightAttack(cord) & ~capt & ~wOcc;
			//printBitBoard(other);
			int capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			wKnights &= ~getBit(cord);
		}
		unsigned long long wBishops = calcPos->bitBoard[9];
		range = __popcnt64(wBishops);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wBishops);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = bishopAttack(bOcc|wOcc, cord) & bOcc;
			unsigned long long other = bishopAttack(bOcc|wOcc, cord)&~capt & ~wOcc;
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			wBishops &= ~getBit(cord);
		}
		unsigned long long wRooks = calcPos->bitBoard[10];
		range = __popcnt64(wRooks);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wRooks);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = rookAttack(bOcc | wOcc, cord) & bOcc;
			unsigned long long other = rookAttack(bOcc | wOcc, cord) & ~capt & ~wOcc;
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			wRooks &= ~getBit(cord);

		}
		unsigned long long wQueens = calcPos->bitBoard[11];
		range = __popcnt64(wQueens);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wQueens);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = (rookAttack(bOcc | wOcc, cord) & bOcc)| (bishopAttack(bOcc | wOcc, cord) & bOcc);
			unsigned long long other = (rookAttack(bOcc | wOcc, cord) & ~capt & ~wOcc)| (bishopAttack(bOcc | wOcc, cord) & ~capt & ~wOcc);
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			wQueens &= ~getBit(cord);
		}

		//white king moves

		if (calcPos->castle[0]&&calcPos->bitBoard[6]&getBit(60) && calcPos->bitBoard[10] & getBit(63) && (getBit(61) & (wOcc | bOcc) |getBit(62)&(wOcc|bOcc))==0 & isLegalSQ(true, calcPos, 60) & isLegalSQ(true, calcPos, 61) & isLegalSQ(true, calcPos, 62)) {
			mvl->MOVE[20 + othersCounter].type = 1;
			mvl->MOVE[20 + othersCounter].f = 60;
			mvl->MOVE[20 + othersCounter].t = 62;
			othersCounter++;
		}

		if (calcPos->castle[1] && calcPos->bitBoard[6] & getBit(60) && calcPos->bitBoard[10] & getBit(56) && ((getBit(59) & (wOcc | bOcc)) | (getBit(58) & (wOcc | bOcc)) | (getBit(57) & (wOcc | bOcc))) == 0 & isLegalSQ(true, calcPos, 60) & isLegalSQ(true, calcPos, 59) & isLegalSQ(true, calcPos, 58)) {
			mvl->MOVE[20 + othersCounter].type = 2;
			mvl->MOVE[20 + othersCounter].f = 60;
			mvl->MOVE[20 + othersCounter].t = 58;
			othersCounter++;
		}
		
		unsigned long long wKing = calcPos->bitBoard[6];
		range = __popcnt64(wKing);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wKing);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = kingAttack(cord) & bOcc & ~wOcc;
			unsigned long long other = kingAttack(cord) & ~capt & ~wOcc;

			int capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			wKing &= ~getBit(cord);
		}

	}
	else {
		unsigned long long bPawns = calcPos->bitBoard[4];
		
		if (calcPos->enPassant) {
			unsigned long long cord = _tzcnt_u64(calcPos->enPassant) + N * 2;
			if (cord == 48) {
				goto noEn2;
			}
			int cordy = cord / 8;
			int cordx = cord - cordy*8;
			if (cordx > 0) {
				if (calcPos->bitBoard[4] & getBit(cord + NW)) {
					mvl->MOVE[gcCounter].f = cord + NW;
					mvl->MOVE[gcCounter].t = cord;
					gcCounter++;
				}
			}
			if (cordx < 7) {
				if (calcPos->bitBoard[4] & getBit(cord + NE)) {
					mvl->MOVE[gcCounter].f = cord + NE;
					mvl->MOVE[gcCounter].t = cord;
					gcCounter++;
				}
			}
		}
		noEn2:
		int range = __popcnt64(bPawns);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bPawns);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;
			if (cord < 56 && !(bOcc & getBit(cord + 8)) && !(wOcc & getBit(cord + 8))) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = cord + 8;
				othersCounter++;
				if (cordy < 2 && !(bOcc & getBit(cord + 16)) && !(wOcc & getBit(cord + 16))) {
					mvl->MOVE[20 + othersCounter].f = cord;
					mvl->MOVE[20 + othersCounter].t = cord + 16;
					othersCounter++;
				}
			}
			if (cordx > 0 && cordy < 7) {
				if (wOcc & getBit(cord + SW)) {
					mvl->MOVE[gcCounter].f = cord;
					mvl->MOVE[gcCounter].t = cord + SW;
					gcCounter++;
				}
			}
			if (cordx < 7 && cordy < 7) {
				if (wOcc & getBit(cord + SE)) {
					mvl->MOVE[gcCounter].f = cord;
					mvl->MOVE[gcCounter].t = cord + SE;
					gcCounter++;

				}
			}
			bPawns &= ~getBit(cord);

		};
		unsigned long long bKnights = calcPos->bitBoard[3];
		range = __popcnt64(bKnights);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bKnights);
			int cordy = cord / 8;
			int cordx = cord - cordy*8;

			unsigned long long capt = knightAttack(cord) & wOcc & ~bOcc;
			unsigned long long other = knightAttack(cord) & ~capt & ~bOcc;

			int capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			bKnights &= ~getBit(cord);
		}
		unsigned long long bBishops = calcPos->bitBoard[2];
		range = __popcnt64(bBishops);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bBishops);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = bishopAttack(bOcc | wOcc, cord) & wOcc;
			unsigned long long other = bishopAttack(bOcc | wOcc, cord) & ~capt & ~bOcc;
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			bBishops &= ~getBit(cord);
		}
		unsigned long long bRooks = calcPos->bitBoard[1];
		range = __popcnt64(bRooks);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bRooks);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = rookAttack(bOcc | wOcc, cord) & wOcc;
			unsigned long long other = rookAttack(bOcc | wOcc, cord) & ~capt & ~bOcc;
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			bRooks &= ~getBit(cord);
		}
		unsigned long long bQueens = calcPos->bitBoard[0];
		range = __popcnt64(bQueens);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bQueens);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = (rookAttack(bOcc | wOcc, cord) & wOcc) | (bishopAttack(bOcc | wOcc, cord) & wOcc);
			unsigned long long other = (rookAttack(bOcc | wOcc, cord) & ~capt & ~bOcc) | (bishopAttack(bOcc | wOcc, cord) & ~capt & ~bOcc);
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			bQueens &= ~getBit(cord);
		}

		//black king moves

		if (calcPos->castle[2] && calcPos->bitBoard[5] & getBit(4) && calcPos->bitBoard[1] & getBit(7) && (getBit(5) & (wOcc | bOcc) | getBit(6) & (wOcc | bOcc)) == 0 & isLegalSQ(false, calcPos, 4) & isLegalSQ(false, calcPos, 5) & isLegalSQ(false, calcPos, 6)) {
			mvl->MOVE[20 + othersCounter].type = 3;
			mvl->MOVE[20 + othersCounter].f = 4;
			mvl->MOVE[20 + othersCounter].t = 6;
			othersCounter++;
		}

		if (calcPos->castle[3] && calcPos->bitBoard[5] & getBit(4) && calcPos->bitBoard[1] & getBit(0) && ((getBit(2) & (wOcc | bOcc)) | (getBit(3) & (wOcc | bOcc)) | (getBit(1) & (wOcc | bOcc))) == 0 & isLegalSQ(false, calcPos, 4) & isLegalSQ(false, calcPos, 3) & isLegalSQ(false, calcPos, 2)) {
			mvl->MOVE[20 + othersCounter].type = 4;
			mvl->MOVE[20 + othersCounter].f = 4;
			mvl->MOVE[20 + othersCounter].t = 2;
			othersCounter++;
		}

		unsigned long long bKing = calcPos->bitBoard[5];
		range = __popcnt64(bKing);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bKing);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = kingAttack(cord) & bOcc & ~bOcc;
			unsigned long long other = kingAttack(cord) & ~capt & ~bOcc;

			int capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				mvl->MOVE[gcCounter].f = cord;
				mvl->MOVE[gcCounter].t = _tzcnt_u64(capt);
				gcCounter++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}
			int otherm = __popcnt64(other);
			for (int e = 0; e < otherm; e++) {
				mvl->MOVE[20 + othersCounter].f = cord;
				mvl->MOVE[20 + othersCounter].t = _tzcnt_u64(other);
				othersCounter++;
				other &= ~getBit(_tzcnt_u64(other));
			}

			bKing &= ~getBit(cord);
		}

	}


	mvl->mam = gcCounter+ othersCounter;
	mvl->gcapt = gcCounter;
	return;
};

void genAllCaptures(struct QList* ql,bool side, struct position* calcPos) {

	int ctr = 0;

	unsigned long long wOcc = calcPos->bitBoard[6] | calcPos->bitBoard[7] | calcPos->bitBoard[8] | calcPos->bitBoard[9] | calcPos->bitBoard[10] | calcPos->bitBoard[11];
	unsigned long long bOcc = calcPos->bitBoard[0] | calcPos->bitBoard[1] | calcPos->bitBoard[2] | calcPos->bitBoard[3] | calcPos->bitBoard[4] | calcPos->bitBoard[5];

	//not needed
	/*for (int i = 0; i < 100; i++) {
		ql->MOVE[i].type = 0;
		ql->MOVE[i].f = 0;
		ql->MOVE[i].t = 0;
	}*/

	int bestSoFar = -1000;

	if (side) {

		unsigned long long wPawns = calcPos->bitBoard[7];

		if (calcPos->enPassant) {
			unsigned long long cord = _tzcnt_u64(calcPos->enPassant) + N * 4;
			if (cord == 32) {
				goto noEn;
			}
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;
			if (cordx > 0) {
				if (calcPos->bitBoard[7] & getBit(cord + SW)) {
					ql->MOVE[ctr].f = cord + SW;
					ql->MOVE[ctr].t = cord;
					if (cVal[5]*10 - cVal[8]>bestSoFar) {
						bestSoFar = cVal[5] * 10 - cVal[8];
						ql->MOVE[ctr].f = ql->MOVE[0].f;
						ql->MOVE[ctr].t = ql->MOVE[0].t;
						ql->MOVE[0].f=cord+SW;
						ql->MOVE[0].t=cord;
					}
					ctr++;
				}
			}
			if (cordx < 7) {
				if (calcPos->bitBoard[7] & getBit(cord + SE)) {
					ql->MOVE[ctr].f = cord + SE;
					ql->MOVE[ctr].t = cord;
					if (cVal[5] * 10 - cVal[8] > bestSoFar) {
						bestSoFar = cVal[5] * 10 - cVal[8];
						ql->MOVE[ctr].f = ql->MOVE[0].f;
						ql->MOVE[ctr].t = ql->MOVE[0].t;
						ql->MOVE[0].f = cord + SE;
						ql->MOVE[0].t = cord;
					}
					ctr++;
				}
			}
		}
	noEn:
		int range = __popcnt64(wPawns);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wPawns);
			unsigned long long cordy = cord / 8;
			unsigned long long cordx = cord - cordy * 8;
			if (cordx > 0 && cordy > 0) {
				if (bOcc & getBit(cord + NW)) {
					ql->MOVE[ctr].f = cord;
					ql->MOVE[ctr].t = cord + NW;
					int otherp = getPiece(calcPos,cord+NW);
					if (cVal[otherp] * 10 - cVal[8] > bestSoFar) {
						bestSoFar = cVal[otherp] * 10 - cVal[8];
						ql->MOVE[ctr].f = ql->MOVE[0].f;
						ql->MOVE[ctr].t = ql->MOVE[0].t;
						ql->MOVE[0].f = cord;
						ql->MOVE[0].t = cord+NW;
					}
					ctr++;
				}
			}
			if (cordx < 7 && cordy > 0) {
				if (bOcc & getBit(cord + NE)) {
					ql->MOVE[ctr].f = cord;
					ql->MOVE[ctr].t = cord + NE;
					int otherp = getPiece(calcPos, cord + NE);
					if (cVal[otherp] * 10 - cVal[8] > bestSoFar) {
						bestSoFar = cVal[otherp] * 10 - cVal[8];
						ql->MOVE[ctr].f = ql->MOVE[0].f;
						ql->MOVE[ctr].t = ql->MOVE[0].t;
						ql->MOVE[0].f = cord;
						ql->MOVE[0].t = cord + NE;
					}
					ctr++;

				}
			}
			wPawns &= ~getBit(cord);

		};
		unsigned long long wKnights = calcPos->bitBoard[8];
		range = __popcnt64(wKnights);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wKnights);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = knightAttack(cord) & bOcc & ~wOcc;
			int capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[9] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[9];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}


			wKnights &= ~getBit(cord);
		}
		unsigned long long wBishops = calcPos->bitBoard[9];
		range = __popcnt64(wBishops);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wBishops);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = bishopAttack(bOcc | wOcc, cord) & bOcc;
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[10] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[10];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}


			wBishops &= ~getBit(cord);
		}
		unsigned long long wRooks = calcPos->bitBoard[10];
		range = __popcnt64(wRooks);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wRooks);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = rookAttack(bOcc | wOcc, cord) & bOcc;
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[11] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[11];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}

			wRooks &= ~getBit(cord);

		}
		unsigned long long wQueens = calcPos->bitBoard[11];
		range = __popcnt64(wQueens);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wQueens);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = (rookAttack(bOcc | wOcc, cord) & bOcc) | (bishopAttack(bOcc | wOcc, cord) & bOcc);
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[12] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[12];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}

			wQueens &= ~getBit(cord);
		}

		//white king moves



		unsigned long long wKing = calcPos->bitBoard[6];
		range = __popcnt64(wKing);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(wKing);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = kingAttack(cord) & bOcc & ~wOcc;

			int capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[7] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[7];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}


			wKing &= ~getBit(cord);
		}

	}
	else {
		unsigned long long bPawns = calcPos->bitBoard[4];

		if (calcPos->enPassant) {
			unsigned long long cord = _tzcnt_u64(calcPos->enPassant) + N * 2;
			if (cord == 48) {
				goto noEn2;
			}
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;
			if (cordx > 0) {
				if (calcPos->bitBoard[4] & getBit(cord + NW)) {
					ql->MOVE[ctr].f = cord + NW;
					ql->MOVE[ctr].t = cord;
					if (cVal[8] * 10 - cVal[5] > bestSoFar) {
						bestSoFar = cVal[8] * 10 - cVal[5];
						ql->MOVE[ctr].f = ql->MOVE[0].f;
						ql->MOVE[ctr].t = ql->MOVE[0].t;
						ql->MOVE[0].f = cord + NW;
						ql->MOVE[0].t = cord;
					}
					ctr++;
				}
			}
			if (cordx < 7) {
				if (calcPos->bitBoard[4] & getBit(cord + NE)) {
					ql->MOVE[ctr].f = cord + NE;
					ql->MOVE[ctr].t = cord;
					if (cVal[8] * 10 - cVal[5] > bestSoFar) {
						bestSoFar = cVal[8] * 10 - cVal[5];
						ql->MOVE[ctr].f = ql->MOVE[0].f;
						ql->MOVE[ctr].t = ql->MOVE[0].t;
						ql->MOVE[0].f = cord + NE;
						ql->MOVE[0].t = cord;
					}
					ctr++;
				}
			}
		}
	noEn2:
		int range = __popcnt64(bPawns);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bPawns);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			if (cordx > 0 && cordy < 7) {
				if (wOcc & getBit(cord + SW)) {
					ql->MOVE[ctr].f = cord;
					ql->MOVE[ctr].t = cord + SW;
					int otherp = getPiece(calcPos, cord + SW);
					if (cVal[otherp] * 10 - cVal[5] > bestSoFar) {
						bestSoFar = cVal[otherp] * 10 - cVal[5];
						ql->MOVE[ctr].f = ql->MOVE[0].f;
						ql->MOVE[ctr].t = ql->MOVE[0].t;
						ql->MOVE[0].f = cord;
						ql->MOVE[0].t = cord + SW;
					}
					ctr++;
				}
			}
			if (cordx < 7 && cordy < 7) {
				if (wOcc & getBit(cord + SE)) {
					ql->MOVE[ctr].f = cord;
					ql->MOVE[ctr].t = cord + SE;
					int otherp = getPiece(calcPos, cord + SE);
					if (cVal[otherp] * 10 - cVal[5] > bestSoFar) {
						bestSoFar = cVal[otherp] * 10 - cVal[5];
						ql->MOVE[ctr].f = ql->MOVE[0].f;
						ql->MOVE[ctr].t = ql->MOVE[0].t;
						ql->MOVE[0].f = cord;
						ql->MOVE[0].t = cord + SE;
					}
					ctr++;

				}
			}
			bPawns &= ~getBit(cord);

		};
		unsigned long long bKnights = calcPos->bitBoard[3];
		range = __popcnt64(bKnights);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bKnights);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = knightAttack(cord) & wOcc & ~bOcc;

			int capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[4] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[4];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}


			bKnights &= ~getBit(cord);
		}
		unsigned long long bBishops = calcPos->bitBoard[2];
		range = __popcnt64(bBishops);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bBishops);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = bishopAttack(bOcc | wOcc, cord) & wOcc;
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[3] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[3];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}


			bBishops &= ~getBit(cord);
		}
		unsigned long long bRooks = calcPos->bitBoard[1];
		range = __popcnt64(bRooks);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bRooks);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = rookAttack(bOcc | wOcc, cord) & wOcc;

			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[2] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[2];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}


			bRooks &= ~getBit(cord);
		}
		unsigned long long bQueens = calcPos->bitBoard[0];
		range = __popcnt64(bQueens);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bQueens);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = (rookAttack(bOcc | wOcc, cord) & wOcc) | (bishopAttack(bOcc | wOcc, cord) & wOcc);
			unsigned long long capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[1] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[1];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}


			bQueens &= ~getBit(cord);
		}

		//black king moves



		unsigned long long bKing = calcPos->bitBoard[5];
		range = __popcnt64(bKing);
		for (int i = 0; i < range; i++) {
			unsigned long long cord = _tzcnt_u64(bKing);
			int cordy = cord / 8;
			int cordx = cord - cordy * 8;

			unsigned long long capt = kingAttack(cord) & bOcc & ~bOcc;

			int capm = __popcnt64(capt);
			for (int e = 0; e < capm; e++) {
				ql->MOVE[ctr].f = cord;
				ql->MOVE[ctr].t = _tzcnt_u64(capt);
				int otherp = getPiece(calcPos, _tzcnt_u64(capt));
				if (cVal[otherp] * 10 - cVal[6] > bestSoFar) {
					bestSoFar = cVal[otherp] * 10 - cVal[6];
					ql->MOVE[ctr].f = ql->MOVE[0].f;
					ql->MOVE[ctr].t = ql->MOVE[0].t;
					ql->MOVE[0].f = cord;
					ql->MOVE[0].t = _tzcnt_u64(capt);
				}
				ctr++;
				capt &= ~getBit(_tzcnt_u64(capt));
			}



			bKing &= ~getBit(cord);
		}

	}


	ql->mam = ctr;
	return;

};