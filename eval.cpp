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
int wkingendgamecenter[64] =
{
	-4,-2,-1,-2,-2,-1,-2,-4,
	-1,0,0,0,0,0,0,-1,
	-1,1,1,0,0,1,1,-1,
	-2,0,1,1,1,1,0,-2,
	-3,-1,0,0,0,0,-1,-3,
	-3,-1,0,0,0,0,-1,-3,
	-4,-3,-1,-1,-1,-1,-3,-4,
	-6,-4,-3,-3,-3,-3,-4,-6
};
int bkingendgamecenter[64] =
{
	-6,-4,-3,-3,-3,-3,-4,-6,
	-4,-3,-1,-1,-1,-1,-3,-4,
	-3,-1,0,0,0,0,-1,-3,
	-3,-1,0,0,0,0,-1,-3,
	-2,0,1,1,1,1,0,-2,
	-1,1,1,0,0,1,1,-1,
	-1,0,0,0,0,0,0,-1,
	-4,-2,-1,-2,-2,-1,-2,-4
};
int wKingMiddleGame[64] =
{
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	4,16,-15,-46,-46,-15,16,4,
	4,30,-4,0,0,-4,30,4
};

int bKingMiddleGame[64] =
{
	4,30,-4,0,0,-4,30,4,
	4,16,-15,-46,-46,-15,16,4,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-36
};
int wpawnendgame[64] =
{
	0,0,0,0,0,0,0,0,
	21,21,17,17,17,17,21,21,
	9,10,8,7,7,8,10,9,
	3,2,1,0,0,1,2,3,
	1,0,-1,-1,-1,-1,0,1,
	0,0,-1,0,0,-1,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};
int bpawnendgame[64] =
{
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	1,0,-1,-1,-1,-1,0,1,
	3,2,1,0,0,1,2,3,
	9,10,7,7,7,7,10,9,
	22,20,16,16,16,16,20,22,
	0,0,0,0,0,0,0,0
};
int wpawnmiddlegame[64] =
{
	0,0,0,0,0,0,0,0,
	1,14,10,15,15,10,14,1,
	0,2,8,7,7,8,2,0,
	-2,2,3,5,5,3,2,-2,
	-3,-1,1,3,3,1,-1,-3,
	0,2,1,0,0,1,2,0,
	-1,1,0,-4,-4,0,1,-1,
	0,0,0,0,0,0,0,0
};
int bpawnmiddlegame[64] =
{
	0,0,0,0,0,0,0,0,
	-1,1,0,-4,-4,0,1,-1,
	0,2,1,0,0,1,2,0,
	-3,-1,1,3,3,1,-1,-3,
	-2,2,3,5,5,3,2,-2,
	0,2,8,7,7,8,2,0,
	1,14,10,15,15,10,14,1,
	0,0,0,0,0,0,0,0
};

int wknightPSQT[64] = {
	-115,-59,-37,-28,-28,-37,-59,-115,
	-53,-20,-7,-3,-3,-7,-20,-53,
	-31,7,23,20,20,23,7,-31,
	-12,3,29,25,25,39,3,-12,
	-30,1,17,20,20,17,1,-30,
	-38,-3,12,14,14,12,-3,-38,
	-49,-35,-13,-6,-6,-13,-35,-49,
	-68,-37,-34,-31,-31,-34,-37,-68
};
int bknightPSQT[64] = {
	-68,-37,-34,-31,-31,-34,-37,-68,
	-49,-35,-13,-6,-6,-13,-35,-49,
	-38,-3,12,14,14,12,-3,-38,
	-30,1,17,20,20,17,1,-30,
	-12,3,29,25,25,39,3,-12,
	-31,7,23,20,20,23,7,-31,
	-53,-20,-7,-3,-3,-7,-20,-53,
	-115,-59,-37,-28,-28,-37,-59,-115,
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
unsigned long long wPasserMask[64];
unsigned long long bPasserMask[64];

unsigned long long wSide = 4294967295Ui64;
unsigned long long bSide = 18446744069414584320Ui64;

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
	for (int i = 0; i < 64; i++) {
		wPasserMask[i] = 0;
		bPasserMask[i] = 0;
		int y = i / 8;
		int x = i-y*8;
		x--;
		y--;
		while ((x>=0&&x<=7)&&(y>=0&&y<=7)) {
			wPasserMask[i] |= getBit(y*8+x);
			y--;
		}
		y = i / 8;
		x = i % 8;
		x++;
		y--;
		while ((x >= 0 && x <= 7) && (y >= 0 && y <= 7)) {
			wPasserMask[i] |= getBit(y * 8 + x);
			y--;
		}
		y = i / 8;
		x = i % 8;
		y--;
		while ((x >= 0 && x <= 7) && (y >= 0 && y <= 7)) {
			wPasserMask[i] |= getBit(y * 8 + x);
			y--;
		}
		y = i / 8;
		x = i % 8;
		x--;
		y++;
		while ((x >= 0 && x <= 7) && (y >= 0 && y <= 7)) {
			bPasserMask[i] |= getBit(y * 8 + x);
			y++;
		}
		y = i / 8;
		x = i % 8;
		x++;
		y++;
		while ((x >= 0 && x <= 7) && (y >= 0 && y <= 7)) {
			bPasserMask[i] |= getBit(y * 8 + x);
			y++;
		}
		y = i / 8;
		x = i % 8;
		y++;
		while ((x >= 0 && x <= 7) && (y >= 0 && y <= 7)) {
			bPasserMask[i] |= getBit(y * 8 + x);
			y++;
		}
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
		int cord = _tzcnt_u64(p);
		unsigned long long attack = wPawnAttack(cord);
		wpawn |= attack;
		v.egMob[0] += tv.MODIF[0]*wpawnendgame[cord];
		v.mgMob[0] += tv.MODIF[1] * wpawnmiddlegame[cord];
		v.attCnt[0] += __popcnt64(bKingClose & attack);
		v.positionalThemes[0] += tv.MODIF[33] * __popcnt64(bPawnAttack(cord) & pos->bitBoard[7]);
		if (__popcnt64(isolaniMask[cord] & pos->bitBoard[7]) < 1) {
			v.positionalThemes[0] -= tv.MODIF[2];
		}
		if (__popcnt64(doubledMask[cord] & pos->bitBoard[7]) > 1) {
			v.positionalThemes[0] -= tv.MODIF[3];
		}
		if (!(wPasserMask[cord] & pos->bitBoard[4])) {
			v.egMob[0] += tv.MODIF[35];
		}
		if (cord > 7) {
			if (!(getBit(cord - 8) & (wOcc | bOcc)) && !(bPasserMask[cord]&pos->bitBoard[7]) && __popcnt64(wPawnAttack(cord - 8) & pos->bitBoard[4]) > __popcnt64(bPawnAttack(cord - 8) & pos->bitBoard[7])) {
				v.positionalThemes[0] -= tv.MODIF[30];

				if (!(doubledMask[cord] & pos->bitBoard[4])) {
					v.positionalThemes[0] -= tv.MODIF[31]; 
					if (pos->bitBoard[1]) {
						v.positionalThemes[0] -= tv.MODIF[32];
					}
				}
			}
		}
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[4];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		int cord = _tzcnt_u64(p);
		unsigned long long attack = bPawnAttack(cord);
		bpawn |= attack;
		v.egMob[1] += tv.MODIF[0] *bpawnendgame[cord];
		v.mgMob[1] += tv.MODIF[1] * bpawnmiddlegame[cord];
		v.attCnt[1] += __popcnt64(wKingClose & attack);
		v.positionalThemes[1] += tv.MODIF[33] * __popcnt64(wPawnAttack(cord) & pos->bitBoard[4]);
		if (__popcnt64(isolaniMask[cord] & pos->bitBoard[4]) < 1) {
			v.positionalThemes[1] -= tv.MODIF[2];
		}
		if (__popcnt64(doubledMask[cord] & pos->bitBoard[4]) > 1) {
			v.positionalThemes[1] -= tv.MODIF[3];
		}
		if (!(bPasserMask[cord] & pos->bitBoard[7])) {
			v.egMob[1] += tv.MODIF[35];
		}
		if (cord < 55) {
			if (!(getBit(cord + 8) & (wOcc | bOcc)) && !(wPasserMask[cord] & pos->bitBoard[4]) && __popcnt64(bPawnAttack(cord + 8) & pos->bitBoard[7]) > __popcnt64(wPawnAttack(cord + 8) & pos->bitBoard[4])) {
				v.positionalThemes[1] -= tv.MODIF[30];
				if (!(doubledMask[cord] & pos->bitBoard[7])) {
					v.positionalThemes[1] -= tv.MODIF[31];
				}
				if (pos->bitBoard[10]) {
					v.positionalThemes[1] -= tv.MODIF[32];
				}
			}
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
		v.mgMob[0] += tv.MODIF[34] * __popcnt64(wSide&attack);
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
		v.mgMob[1] += tv.MODIF[34] * __popcnt64(bSide & attack);
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
		v.mgMob[0] += tv.MODIF[7] * wknightPSQT[_tzcnt_u64(p)]/2;
		v.egMob[0] += tv.MODIF[8] * wknightPSQT[_tzcnt_u64(p)] / 2;
		v.attCnt[0] += __popcnt64(bKingClose & attack);
		v.positionalThemes[0] += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[3];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = knightAttack(_tzcnt_u64(p));
		v.mgMob[1] += tv.MODIF[6] * __popcnt64(attack & centerMask & ~wpawn & ~bOcc);
		v.mgMob[1] += tv.MODIF[7] * bknightPSQT[_tzcnt_u64(p)]/2;
		v.egMob[1] += tv.MODIF[8] * bknightPSQT[_tzcnt_u64(p)] / 2;
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
	v.egMob[0] += tv.MODIF[18] *wkingendgamecenter[_tzcnt_u64(pos->bitBoard[6])];
	v.egMob[1] += tv.MODIF[18] *bkingendgamecenter[_tzcnt_u64(pos->bitBoard[5])];

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
	printBitBoard(wPasserMask[i]);
	printBitBoard(bPasserMask[i]);
}