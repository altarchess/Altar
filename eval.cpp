#include "eval.h"
#include "bitboard.h"
#include "gen.h"

#include <immintrin.h>
#include <intrin.h>
#include <stdlib.h>
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


int rowMask[8] = {
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

void fillEvalTables() {
	for (int i = 0; i < 64; i++) {
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
	}
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
			solid -= 100;
		}
		if (__popcnt64(doubledMask[_tzcnt_u64(p)] & pos->bitBoard[7]) > 1) {
			solid -= 50; //only half the penalty since the other pawn will also trigger
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
			solid += 100;
		}
		if (__popcnt64(doubledMask[_tzcnt_u64(p)] & pos->bitBoard[4]) > 1) {
			solid += 50; //only half the penalty since the other pawn will also trigger
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

	return 5+evalmult * (wm - bm + 3 * wSpace - 3 * bSpace + solid + safety * middleGamePhase / 10 + endGamePhase * (wEndGameSpace - bEndGameSpace) / 20);
}

int middleGameEval() {

}

int endGameEval() {

}
int evals2() {

}