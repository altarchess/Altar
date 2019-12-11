#include "eval.h"
#include "bitboard.h"
#include "gen.h"
#include "search.h"
#include "engine.h"
#include "tune.h"

#include <immintrin.h>
#include <intrin.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <stdio.h>

struct tuneVector tv;

struct tuneVector* getTuneVector() {
	return &tv;
}

//score grain is 1/256 of a pawn.
int center[64] =
{
	-2,-1,-1,-1,-1,-1,-1,-2,
	-1,0,0,0,0,0,0,-1,
	-1,0,1,2,2,1,0,-1,
	-1,0,2,2,2,2,0,-1,
	-1,0,2,2,2,2,0,-1,
	-1,0,1,2,2,1,0,-1,
	-1,0,0,0,0,0,0,-1,
	-2,-1,0,0,0,0,-1,-2
};
int kingendgamecenter[64] =
{
	-5,-4,-3,-2,-2,-3,-4,-5,
	-4,-3,-2,-1,-1,-2,-3,-4,
	-3,-2,-1,0,0,-1,-2,-3,
	-2,-1,0,1,1,0,-1,-2,
	-2,-1,0,1,1,0,-1,-2,
	-3,-2,-1,0,0,-1,-2,-3,
	-4,-3,-2,-1,-1,-2,-3,-4,
	-5,-4,-3,-2,-2,-3,-4,-5
};
int wKingMiddleGame[64] =
{
	-40,-50,-60,-60,-60,-60,-50,-40,
	-40,-50,-60,-60,-60,-60,-50,-40,
	-50,-60,-70,-80,-80,-70,-60,-50,
	-40,-50,-60,-60,-60,-60,-50,-40,
	-40,-50,-50,-50,-50,-50,-50,-40,
	-40,-40,-40,-40,-40,-40,-40,-40,
	0,5,-10,-30,-30,-10,10,0,
	14,18,10,-20,-10,3,18,14
};

int bKingMiddleGame[64] =
{
	14,18,10,-20,-10,3,18,14,
	0,5,-10,-30,-30,-10,10,0,
	-40,-40,-40,-40,-40,-40,-40,-40,
	-40,-50,-50,-50,-50,-50,-50,-40,
	-40,-50,-60,-60,-60,-60,-50,-40,
	-50,-60,-70,-80,-80,-70,-60,-50,
	-40,-50,-60,-60,-60,-60,-50,-40,
	-40,-50,-60,-60,-60,-60,-50,-40,
};
int wpawnendgame[64] =
{
	0,0,0,0,0,0,0,0,
	10,10,10,10,10,10,10,10,
	6,6,6,6,6,6,6,6,
	3,3,3,3,3,3,3,3,
	1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,
	-1,-1,-1,-1,-1,-1,-1,-1,
	0,0,0,0,0,0,0,0
};
int bpawnendgame[64] =
{
	0,0,0,0,0,0,0,0,
	-1,-1,-1,-1,-1,-1,-1,-1,
	0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,
	3,3,3,3,3,3,3,3,
	6,6,6,6,6,6,6,6,
	10,10,10,10,10,10,10,10,
	0,0,0,0,0,0,0,0
};
int wpawnmiddlegame[64] =
{
	0,0,0,0,0,0,0,0,
	10,10,10,10,10,10,10,10,
	6,6,6,6,6,6,6,6,
	1,1,2,5,5,2,1,1,
	0,0,1,3,3,1,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,-2,-2,0,0,0,
	0,0,0,0,0,0,0,0
};
int bpawnmiddlegame[64] =
{
	0,0,0,0,0,0,0,0,
	0,0,0,-2,-2,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,1,3,3,1,0,0,
	1,1,2,5,5,2,1,1,
	6,6,6,6,6,6,6,6,
	10,10,10,10,10,10,10,10,
	0,0,0,0,0,0,0,0
};

int knightPSQT[64] = {
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  0,  0,  0,-20,-40,
-30,  10, 10, 15, 15, 10,  0,-30,
-30,  15, 15, 20, 20, 15,  5,-30,
-30,  15, 15, 20, 20, 15,  0,-30,
-30,  10, 10, 15, 15, 10,  5,-30,
-40,-20,  0,  5,  5,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50
};

unsigned long long rowMask[8] = {
	9259542123273814144,
	4629771061636907072,
	2314885530818453536,
	1157442765409226768,
	578721382704613384,
	289360691352306692,
	144680345676153346,
	72340172838076673
};

//eval Tables;
unsigned long long isolaniMask[64];
unsigned long long doubledMask[64];

struct evalVector {
	int gamePhase;
	int mgMob[2];
	int egMob[2];
	int attCnt[2];
	int attWeight[2];
	int kingShield[2];
	int materialAdjustment[2];
	int blockages[2];
	int positionalThemes[2];
};

struct evalVector v;



void fillEvalTables() {
	for (int i = 0; i < 64; i++) {
		isolaniMask[i] = 0;
		int x = i % 8;
		if (x > 0) {
			isolaniMask[i] |= rowMask[7-x-1];
		}
		if (x < 7) {
			isolaniMask[i] |= rowMask[7-x+1];
		}
	}
	for (int i = 0; i < 64; i++) {
		int x = i % 8;
		doubledMask[i] = rowMask[7-x];
	}
	/*for (int i = 0; i < 64; i++) {
		isolaniMask[i] = 0;
		int x = i % 8;
		if (x > 0) {
			isolaniMask[i] |= rowMask[x - 1];
		}
		if (x < 7) {
			isolaniMask[i] |= rowMask[x + 1];
		}
	}
	for (int i = 0; i < 64; i++) {
		int x = i % 8;
		doubledMask[i] = rowMask[x];
	}*/
}

int materialDraw(struct position* pos) {

	int bq = __popcnt64(pos->bitBoard[0]);
	int br = __popcnt64(pos->bitBoard[1]);
	int bb = __popcnt64(pos->bitBoard[2]);
	int bn = __popcnt64(pos->bitBoard[3]);

	int wq = __popcnt64(pos->bitBoard[11]);
	int wr = __popcnt64(pos->bitBoard[10]);
	int wb = __popcnt64(pos->bitBoard[9]);
	int wn = __popcnt64(pos->bitBoard[8]);


	if (!wr && !br && !wq && !bq) {
		if (!bb && !wb) {
			if (wn < 3 && bn < 3)
				return true;
		}
		else if (!wn && !bn) {
			if (abs(wb - bb) < 2)
				return true;
		}
		else if ((wn < 3 && !wb) || (wb == 1 && !wn)) {
			if ((bn < 3 && !bb) || (bb == 1 && !bn))
				return true;
		}
	}
	else if (!wq && !bq) {
		if (wr == 1 && br == 1) {
			if ((wn + wb) < 2 && (bn + bb) < 2)
				return true;
		}
		else if (wr == 1 && !br) {
			if ((wn + wb == 0) && (((bn + bb) == 1) || ((bn + bb) == 2)))
				return true;
		}
		else if (br == 1 && !wr) {
			if ((bn + bb == 0) && (((wn + wb) == 1) || ((wn + wb) == 2)))
				return true;
		}
	}
	return false;
}

int evals(struct position* pos) {

	if (!pos->bitBoard[5]) {
		if (pos->side) {
			return wMateScore - 100;
		}
		else {
			return bMateScore + 100;
		}
	}
	if (!pos->bitBoard[6]) {
		if (pos->side) {
			return bMateScore + 100;
		}
		else {
			return wMateScore - 100;
		}
	}

	int wm = __popcnt64(pos->bitBoard[6]) * 100000 + __popcnt64(pos->bitBoard[7]) * pawnMiddleGame + __popcnt64(pos->bitBoard[8]) * knightMiddleGame + __popcnt64(pos->bitBoard[9]) * bishopMiddleGame + __popcnt64(pos->bitBoard[10]) * rookMiddleGame + __popcnt64(pos->bitBoard[11]) * queenMiddleGame;
	int bm = __popcnt64(pos->bitBoard[5]) * 100000 + __popcnt64(pos->bitBoard[4]) * pawnMiddleGame + __popcnt64(pos->bitBoard[3]) * knightMiddleGame + __popcnt64(pos->bitBoard[2]) * bishopMiddleGame + __popcnt64(pos->bitBoard[1]) * rookMiddleGame + __popcnt64(pos->bitBoard[0]) * queenMiddleGame;



	if (__popcnt64(pos->bitBoard[4]) == 0 && __popcnt64(pos->bitBoard[7]) == 0 && materialDraw(pos)) {
		return 0;
	}

	int gamePhase = __popcnt64(pos->bitBoard[8]) * knigthPhase + __popcnt64(pos->bitBoard[3]) * knigthPhase + __popcnt64(pos->bitBoard[9]) * bishopPhase + __popcnt64(pos->bitBoard[2]) * bishopPhase
		+ __popcnt64(pos->bitBoard[10]) * rookPhase + __popcnt64(pos->bitBoard[1]) * rookPhase + __popcnt64(pos->bitBoard[11]) * queenPhase + __popcnt64(pos->bitBoard[0]) * queenPhase;


	unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
	unsigned long long bOcc = pos->bitBoard[5] | pos->bitBoard[4] | pos->bitBoard[3] | pos->bitBoard[2] | pos->bitBoard[1] | pos->bitBoard[0];

	int wdefenders = __popcnt64(wOcc & kingAttack(_tzcnt_u64(pos->bitBoard[6])));
	int bdefenders = __popcnt64(bOcc & kingAttack(_tzcnt_u64(pos->bitBoard[5])));

	unsigned long long wKingClose = kingAttack(_tzcnt_u64(pos->bitBoard[6])) | getBit(_tzcnt_u64(pos->bitBoard[6]));
	unsigned long long bKingClose = kingAttack(_tzcnt_u64(pos->bitBoard[5])) | getBit(_tzcnt_u64(pos->bitBoard[5]));



	int wAttacks = 0;
	int bAttacks = 0;

	int wSpace = 0;
	int bSpace = 0;

	int solid = 0;

	int wEndGameSpace = 4 * kingendgamecenter[_tzcnt_u64(pos->bitBoard[6])];
	int bEndGameSpace = 4 * kingendgamecenter[_tzcnt_u64(pos->bitBoard[5])];

	unsigned long long wpawn = 0;
	unsigned long long bpawn = 0;

	//pawn eval
	unsigned long long p = pos->bitBoard[7];
	int range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = wPawnAttack(_tzcnt_u64(p));
		wpawn |= attack;
		wSpace += center[_tzcnt_u64(p)];
		wSpace += 2 * wpawnendgame[_tzcnt_u64(p)];
		wEndGameSpace += wpawnendgame[_tzcnt_u64(p)];
		wAttacks += __popcnt64(bKingClose & attack);
		solid += 2 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		if (__popcnt64(isolaniMask[_tzcnt_u64(p)] & pos->bitBoard[7]) < 1) {
			solid -= 60;
		}
		if (__popcnt64(doubledMask[_tzcnt_u64(p)] & pos->bitBoard[7]) > 1) {
			solid -= 30; //only half the penalty since the other pawn will also trigger
		}
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[4];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bPawnAttack(_tzcnt_u64(p));
		bpawn |= attack;
		bSpace += center[_tzcnt_u64(p)];
		bSpace += 2 * bpawnendgame[_tzcnt_u64(p)];
		bEndGameSpace += bpawnendgame[_tzcnt_u64(p)];
		bAttacks += __popcnt64(wKingClose & attack);
		solid -= 2 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		if (__popcnt64(isolaniMask[_tzcnt_u64(p)] & pos->bitBoard[4]) < 1) {
			solid += 60;
		}
		if (__popcnt64(doubledMask[_tzcnt_u64(p)] & pos->bitBoard[4]) > 1) {
			solid += 30; //only half the penalty since the other pawn will also trigger
		}
		p &= ~getBit(_tzcnt_u64(p));
	}

	//knight eval
	p = pos->bitBoard[8];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = knightAttack(_tzcnt_u64(p));
		wSpace += 2 * __popcnt64(attack & centerMask & ~bpawn & ~wOcc);
		wSpace += 2 * center[_tzcnt_u64(p)];
		wAttacks += __popcnt64(bKingClose & attack);
		solid += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[3];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = knightAttack(_tzcnt_u64(p));
		bSpace += 2 * __popcnt64(attack & centerMask & ~wpawn & ~bOcc);
		bSpace += 2 * center[_tzcnt_u64(p)];
		bAttacks += __popcnt64(wKingClose & attack);
		solid -= 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}

	//bishop eval
	p = pos->bitBoard[9];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p));
		wSpace += 5 * __popcnt64(attack & centerMask & ~bpawn);
		wAttacks += __popcnt64(bKingClose & attack);
		solid += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[2];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p));
		bSpace += 5 * __popcnt64(attack & centerMask & ~wpawn);
		bAttacks += __popcnt64(wKingClose & attack);
		solid -= 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}

	//rook is 5 pawns no matter what
	p = pos->bitBoard[10];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		wSpace += 2 * __popcnt64(attack & ~bpawn);
		wAttacks += __popcnt64(bKingClose & attack);
		solid += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[1];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		bSpace += 2 * __popcnt64(attack & ~wpawn);
		bAttacks += __popcnt64(wKingClose & attack);
		solid -= 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}

	//Queen eval
	p = pos->bitBoard[11];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p)) | rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		wSpace += 1 * __popcnt64(attack & centerMask & ~bpawn);
		wAttacks += 2 * __popcnt64(bKingClose & attack);
		solid += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[0];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p)) | rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		bSpace += 1 * __popcnt64(attack & centerMask & ~wpawn);
		bAttacks += 2 * __popcnt64(wKingClose & attack);
		solid -= 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}


	int endGamePhase = (2 * 100000 + 20000 - wm - bm) / 256;
	int middleGamePhase = (wm + bm - 2 * 100000 - 8240) / 256;
	if (middleGamePhase < 0) {
		middleGamePhase = 0;
	}


	int safety = __popcnt64(bishopAttack(bOcc, _tzcnt_u64(pos->bitBoard[5])) | rookAttack(bOcc, _tzcnt_u64(pos->bitBoard[5]))) - __popcnt64(bishopAttack(wOcc, _tzcnt_u64(pos->bitBoard[6])) | rookAttack(wOcc, _tzcnt_u64(pos->bitBoard[6])));
	safety += wKingMiddleGame[_tzcnt_u64(pos->bitBoard[6])];
	safety -= bKingMiddleGame[_tzcnt_u64(pos->bitBoard[5])];
	safety += 4 * wAttacks;
	safety -= 4 * bAttacks;

	int evalmult = 1;
	if (!pos->side) {
		evalmult = -1;
	}

	return 5 + evalmult * (wm - bm + 3 * wSpace - 3 * bSpace + solid + safety * middleGamePhase / 10 + endGamePhase * (wEndGameSpace - bEndGameSpace) / 20);
}

//used for quick materialCount for qsearch cutoffs;
int materialEval(struct position* pos) {

	if (!pos->bitBoard[5]) {
		if (pos->side) {
			return wMateScore - 100;
		}
		else {
			return bMateScore + 100;
		}
	}
	if (!pos->bitBoard[6]) {
		if (pos->side) {
			return bMateScore + 100;
		}
		else {
			return wMateScore - 100;
		}
	}

	int wm = __popcnt64(pos->bitBoard[6]) * 100000 + __popcnt64(pos->bitBoard[7]) * pawnMiddleGame + __popcnt64(pos->bitBoard[8]) * knightMiddleGame + __popcnt64(pos->bitBoard[9]) * bishopMiddleGame + __popcnt64(pos->bitBoard[10]) * rookMiddleGame + __popcnt64(pos->bitBoard[11]) * queenMiddleGame;
	int bm = __popcnt64(pos->bitBoard[5]) * 100000 + __popcnt64(pos->bitBoard[4]) * pawnMiddleGame + __popcnt64(pos->bitBoard[3]) * knightMiddleGame + __popcnt64(pos->bitBoard[2]) * bishopMiddleGame + __popcnt64(pos->bitBoard[1]) * rookMiddleGame + __popcnt64(pos->bitBoard[0]) * queenMiddleGame;

	int evalmult = 1;
	if (!pos->side) {
		evalmult = -1;
	}
	return evalmult * (wm - bm);
}

int eval(struct position* pos) {
	if (!pos->bitBoard[5]) {
		if (pos->side) {
			return wMateScore - 100;
		}
		else {
			return bMateScore + 100;
		}
	}
	if (!pos->bitBoard[6]) {
		if (pos->side) {
			return bMateScore + 100;
		}
		else {
			return wMateScore - 100;
		}
	}

	//add tt probing here

	if (__popcnt64(pos->bitBoard[4]) == 0 && __popcnt64(pos->bitBoard[7]) == 0 && materialDraw(pos)) {
		return 0;
	}

	//piece occ for attackset generation & other bitboards
	unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
	unsigned long long bOcc = pos->bitBoard[5] | pos->bitBoard[4] | pos->bitBoard[3] | pos->bitBoard[2] | pos->bitBoard[1] | pos->bitBoard[0];
	unsigned long long wKingClose = kingAttack(_tzcnt_u64(pos->bitBoard[6])) | getBit(_tzcnt_u64(pos->bitBoard[6]));
	unsigned long long bKingClose = kingAttack(_tzcnt_u64(pos->bitBoard[5])) | getBit(_tzcnt_u64(pos->bitBoard[5]));

	//material scores
	int wmm = __popcnt64(pos->bitBoard[6]) * 100000 + __popcnt64(pos->bitBoard[7]) * tv.MODIF[20] + __popcnt64(pos->bitBoard[8]) * tv.MODIF[21] + __popcnt64(pos->bitBoard[9]) * tv.MODIF[22] + __popcnt64(pos->bitBoard[10]) * tv.MODIF[23] + __popcnt64(pos->bitBoard[11]) * tv.MODIF[24];
	int bmm = __popcnt64(pos->bitBoard[5]) * 100000 + __popcnt64(pos->bitBoard[4]) * tv.MODIF[20] + __popcnt64(pos->bitBoard[3]) * tv.MODIF[21] + __popcnt64(pos->bitBoard[2]) * tv.MODIF[22] + __popcnt64(pos->bitBoard[1]) * tv.MODIF[23] + __popcnt64(pos->bitBoard[0]) * tv.MODIF[24];

	int wme = __popcnt64(pos->bitBoard[6]) * 100000 + __popcnt64(pos->bitBoard[7]) * tv.MODIF[25] + __popcnt64(pos->bitBoard[8]) * tv.MODIF[26] + __popcnt64(pos->bitBoard[9]) * tv.MODIF[27] + __popcnt64(pos->bitBoard[10]) * tv.MODIF[28] + __popcnt64(pos->bitBoard[11]) * tv.MODIF[29];
	int bme = __popcnt64(pos->bitBoard[5]) * 100000 + __popcnt64(pos->bitBoard[4]) * tv.MODIF[25] + __popcnt64(pos->bitBoard[3]) * tv.MODIF[26] + __popcnt64(pos->bitBoard[2]) * tv.MODIF[27] + __popcnt64(pos->bitBoard[1]) * tv.MODIF[28] + __popcnt64(pos->bitBoard[0]) * tv.MODIF[29];
	

	v.gamePhase = 0;
	v.mgMob[0] = 0;
	v.mgMob[1] = 0;
	v.egMob[0] = 0;
	v.egMob[1] = 0;
	v.attCnt[0] = 0;
	v.attCnt[1] = 0;
	v.attWeight[0] = 0;
	v.attWeight[1] = 0;
	v.kingShield[0] = 0;
	v.kingShield[1] = 0;
	v.materialAdjustment[0] = 0;
	v.materialAdjustment[1] = 0;
	v.blockages[0] = 0;
	v.blockages[1] = 0;
	v.positionalThemes[0] = 0;
	v.positionalThemes[1] = 0;

	int phase = totalPhase;
	phase -= __popcnt64(pos->bitBoard[8]) * knigthPhase;
	phase -= __popcnt64(pos->bitBoard[3]) * knigthPhase;
	phase -= __popcnt64(pos->bitBoard[9]) * bishopPhase;
	phase -= __popcnt64(pos->bitBoard[2]) * bishopPhase;
	phase -= __popcnt64(pos->bitBoard[10]) * rookPhase;
	phase -= __popcnt64(pos->bitBoard[1]) * rookPhase;
	phase -= __popcnt64(pos->bitBoard[11]) * queenPhase;
	phase -= __popcnt64(pos->bitBoard[0]) * queenPhase;


	//phase count;


	//pawn Structure eval
	unsigned long long wpawn = 0;
	unsigned long long bpawn = 0;

	unsigned long long p = pos->bitBoard[7];
	int range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = wPawnAttack(_tzcnt_u64(p));
		wpawn |= attack;
		v.egMob[0] += tv.MODIF[0]*wpawnendgame[_tzcnt_u64(p)];
		v.mgMob[0] += tv.MODIF[1] * wpawnmiddlegame[_tzcnt_u64(p)];
		v.attCnt[0] += __popcnt64(bKingClose & attack);
		v.positionalThemes[0] += 2 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		if (__popcnt64(isolaniMask[_tzcnt_u64(p)] & pos->bitBoard[7]) < 1) {
			v.positionalThemes[0] -= tv.MODIF[2];
		}
		if (__popcnt64(doubledMask[_tzcnt_u64(p)] & pos->bitBoard[7]) > 1) {
			v.positionalThemes[0] -= tv.MODIF[3];
		}
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[4];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bPawnAttack(_tzcnt_u64(p));
		bpawn |= attack;
		v.egMob[1] += tv.MODIF[0] *bpawnendgame[_tzcnt_u64(p)];
		v.mgMob[1] += tv.MODIF[1] * bpawnmiddlegame[_tzcnt_u64(p)];
		v.attCnt[1] += __popcnt64(wKingClose & attack);
		v.positionalThemes[1] += 2 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		if (__popcnt64(isolaniMask[_tzcnt_u64(p)] & pos->bitBoard[4]) < 1) {
			v.positionalThemes[1] -= tv.MODIF[2];
		}
		if (__popcnt64(doubledMask[_tzcnt_u64(p)] & pos->bitBoard[4]) > 1) {
			v.positionalThemes[1] -= tv.MODIF[3];
		}
		p &= ~getBit(_tzcnt_u64(p));
	}



	//middle Game mobility & attacks

		//bishops Mobility;
	p = pos->bitBoard[9];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc|bOcc, _tzcnt_u64(p));
		v.mgMob[0] += tv.MODIF[4] * (__popcnt64(attack & centerMask & ~bpawn) +  __popcnt64(attack & ~bpawn));
		v.egMob[0] += tv.MODIF[5] * __popcnt64(attack);
		v.attCnt[0] += __popcnt64(bKingClose & attack);
		v.positionalThemes[0] += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[2];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p));
		v.mgMob[1] += tv.MODIF[4] * (__popcnt64(attack & centerMask & ~wpawn)+ __popcnt64(attack & ~wpawn));
		v.egMob[1] += tv.MODIF[5] * __popcnt64(attack);
		v.attCnt[1] += __popcnt64(wKingClose & attack);
		v.positionalThemes[1] += 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	//knight eval psqt * valid squares
	p = pos->bitBoard[8];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = knightAttack(_tzcnt_u64(p));
		v.mgMob[0] += tv.MODIF[6] * __popcnt64(attack & centerMask & ~bpawn & ~wOcc);
		v.mgMob[0] += tv.MODIF[7] * knightPSQT[_tzcnt_u64(p)]/2;
		v.egMob[0] += tv.MODIF[8] * knightPSQT[_tzcnt_u64(p)] / 2;
		v.attCnt[0] += __popcnt64(bKingClose & attack);
		v.positionalThemes[0] += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[3];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = knightAttack(_tzcnt_u64(p));
		v.mgMob[1] += tv.MODIF[6] * __popcnt64(attack & centerMask & ~wpawn & ~bOcc);
		v.mgMob[1] += tv.MODIF[7] * knightPSQT[_tzcnt_u64(p)]/2;
		v.egMob[1] += tv.MODIF[8] * knightPSQT[_tzcnt_u64(p)] / 2;
		v.attCnt[1] += __popcnt64(wKingClose & attack);
		v.positionalThemes[1] += 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	//rook eval
	p = pos->bitBoard[10];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		v.mgMob[0] += tv.MODIF[9] * __popcnt64(attack & ~bpawn);
		v.egMob[0] += tv.MODIF[10] * __popcnt64(attack);
		v.attCnt[0] += 2 * __popcnt64(bKingClose & attack);
		v.positionalThemes[0] += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[1];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		v.mgMob[1] += tv.MODIF[9] * __popcnt64(attack & ~wpawn);
		v.egMob[1] += tv.MODIF[10] *  __popcnt64(attack);
		v.attCnt[1] += 2 * __popcnt64(wKingClose & attack);
		v.positionalThemes[1] += 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	//Queen eval
	p = pos->bitBoard[11];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p)) | rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		v.mgMob[0] += tv.MODIF[11] * __popcnt64(attack & centerMask & ~bpawn);
		v.egMob[0] += tv.MODIF[12] * __popcnt64(attack);
		v.attCnt[0] += 2 * __popcnt64(bKingClose & attack);
		v.positionalThemes[0] += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[0];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p)) | rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		v.mgMob[1] += tv.MODIF[11] * __popcnt64(attack & centerMask & ~wpawn);
		v.egMob[1] += tv.MODIF[12] * __popcnt64(attack);
		v.attCnt[1] += 2 * __popcnt64(wKingClose & attack);
		v.positionalThemes[1] += 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}

	//bishop pair bonus and knight pair penalty
	if (__popcnt64(pos->bitBoard[9]) > 1) {
		v.materialAdjustment[0] += tv.MODIF[13];
	}
	if (__popcnt64(pos->bitBoard[2]) > 1) {
		v.materialAdjustment[1] +=  tv.MODIF[13];
	}
	if (__popcnt64(pos->bitBoard[8]) > 1) {
		v.materialAdjustment[0] -=  tv.MODIF[14];
	}
	if (__popcnt64(pos->bitBoard[3]) > 1) {
		v.materialAdjustment[1] -= tv.MODIF[14];
	}

	//king safety and king eval
	v.kingShield[0] += tv.MODIF[15] * __popcnt64(wKingClose & wOcc);
	v.kingShield[1] += tv.MODIF[15] * __popcnt64(bKingClose & bOcc);

	v.kingShield[0] -= tv.MODIF[15] * __popcnt64(bishopAttack(pos->bitBoard[7], _tzcnt_u64(pos->bitBoard[6])) | rookAttack(pos->bitBoard[7], _tzcnt_u64(pos->bitBoard[6])));
	v.kingShield[1] -= tv.MODIF[15] * __popcnt64(bishopAttack(pos->bitBoard[4], _tzcnt_u64(pos->bitBoard[5])) | rookAttack(pos->bitBoard[4], _tzcnt_u64(pos->bitBoard[5])));



	v.mgMob[0] += tv.MODIF[17] * wKingMiddleGame[_tzcnt_u64(pos->bitBoard[6])];
	v.mgMob[1] += tv.MODIF[17] * bKingMiddleGame[_tzcnt_u64(pos->bitBoard[5])];
	v.egMob[0] += tv.MODIF[18] *kingendgamecenter[_tzcnt_u64(pos->bitBoard[6])];
	v.egMob[1] += tv.MODIF[18] *kingendgamecenter[_tzcnt_u64(pos->bitBoard[5])];

	v.gamePhase = phase;
	//evalmult = always return eval relative to side
	int evalmult = 1;
	if (!pos->side) {
		evalmult = -1;
	}
	int totalSafetyScore = tv.MODIF[19] * v.attCnt[0] - tv.MODIF[19] * v.attCnt[1];
	int mgScore = (wmm - bmm) + v.positionalThemes[0] - v.positionalThemes[1] + v.kingShield[0] - v.kingShield[1] + v.mgMob[0] - v.mgMob[1] + totalSafetyScore + v.materialAdjustment[0] - v.materialAdjustment[1];
	int egScore = (wme - bme) + v.positionalThemes[0] - v.positionalThemes[1] + v.egMob[0] - v.egMob[1] + v.materialAdjustment[0] - v.materialAdjustment[1];

	return 100+evalmult * ((egScore * phase) + (mgScore * (totalPhase-phase))) / totalPhase;

}

void showStatic() {
	int ev = eval(getPositionPointer());
	std::cout << "STATIC EVAL: " << ev << std::endl;
	std::cout << "pawn structure & positional themes: " << v.positionalThemes[0] - v.positionalThemes[1] << std::endl;
	std::cout << "middle game mobility: " << v.mgMob[0] - v.mgMob[1] << std::endl;
	std::cout << "endgame factors: " << v.egMob[0] - v.egMob[1] << std::endl;
	std::cout << "material imbalances: " << v.materialAdjustment[0] - v.materialAdjustment[1] << std::endl;
	std::cout << "kingshield: " << v.kingShield[0] - v.kingShield[1] << std::endl;
	std::cout << "attacker count: " << v.attCnt[0] - v.attCnt[1] << std::endl;
	std::cout << "phase: " << v.gamePhase << std::endl;
}
void testf(int i ) {
	printBitBoard(isolaniMask[i]);
	printBitBoard(isolaniMask[1]);
	printBitBoard(isolaniMask[2]);
	printBitBoard(isolaniMask[3]);
	printBitBoard(isolaniMask[4]);
	printBitBoard(isolaniMask[5]);
}