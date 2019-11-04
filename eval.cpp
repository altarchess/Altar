#include "eval.h"
#include "bitboard.h"
#include "gen.h"

#include <immintrin.h>
#include <intrin.h>
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

int eval(struct position* pos) {
	
	if (!pos->bitBoard[5]) {
		return wMateScore-100;
	}
	if (!pos->bitBoard[6]) {
		return bMateScore + 100;
	}
	int wm = __popcnt64(pos->bitBoard[6]) * 100000 + __popcnt64(pos->bitBoard[7]) * pawnMiddleGame + __popcnt64(pos->bitBoard[8]) * knightMiddleGame + __popcnt64(pos->bitBoard[9]) * bishopMiddleGame + __popcnt64(pos->bitBoard[10]) * rookMiddleGame + __popcnt64(pos->bitBoard[11]) * queenMiddleGame;
	int bm = __popcnt64(pos->bitBoard[5]) * 100000 + __popcnt64(pos->bitBoard[4]) * pawnMiddleGame + __popcnt64(pos->bitBoard[3]) * knightMiddleGame + __popcnt64(pos->bitBoard[2]) * bishopMiddleGame + __popcnt64(pos->bitBoard[1]) * rookMiddleGame + __popcnt64(pos->bitBoard[0]) * queenMiddleGame;

	
	unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
	unsigned long long bOcc = pos->bitBoard[5] | pos->bitBoard[4] | pos->bitBoard[3] | pos->bitBoard[2] | pos->bitBoard[1] | pos->bitBoard[0];
	
	int wdefenders = __popcnt64(wOcc&kingAttack(_tzcnt_u64(pos->bitBoard[6])));
	int bdefenders = __popcnt64(bOcc & kingAttack(_tzcnt_u64(pos->bitBoard[5])));

	unsigned long long wKingClose = kingAttack(_tzcnt_u64(pos->bitBoard[6])) |getBit(_tzcnt_u64(pos->bitBoard[6]));
	unsigned long long bKingClose = kingAttack(_tzcnt_u64(pos->bitBoard[5])) |getBit( _tzcnt_u64(pos->bitBoard[5]));

	

	int wAttacks = 0;
	int bAttacks = 0;

	int wSpace = 0;
	int bSpace = 0;

	int solid = 0;

	int wEndGameSpace = kingendgamecenter[_tzcnt_u64(pos->bitBoard[7])];
	int bEndGameSpace = kingendgamecenter[_tzcnt_u64(pos->bitBoard[6])];
	//pawn eval
	unsigned long long p = pos->bitBoard[7];
	int range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = wPawnAttack(_tzcnt_u64(p));
		wSpace += 2 * __popcnt64(attack & bSpaceMask)+__popcnt64(attack);
		wSpace += center[_tzcnt_u64(p)];
		wEndGameSpace += wpawnendgame[_tzcnt_u64(p)];
		wAttacks += __popcnt64(bKingClose & attack);
		solid += 2 *__popcnt64(bPawnAttack(_tzcnt_u64(p))&pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[4];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bPawnAttack(_tzcnt_u64(p));
		bSpace += 2 * __popcnt64(attack & wSpaceMask) + __popcnt64(attack);
		bSpace += center[_tzcnt_u64(p)];
		bEndGameSpace += bpawnendgame[_tzcnt_u64(p)];
		bAttacks += __popcnt64(wKingClose & attack);
		solid -= 2 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}

	//knight eval
	p = pos->bitBoard[8];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = knightAttack(_tzcnt_u64(p));
		wSpace += 2 * __popcnt64(attack & bSpaceMask) + __popcnt64(attack);
		wSpace += 4*center[_tzcnt_u64(p)];
		wAttacks += __popcnt64(bKingClose & attack);
		solid += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[3];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = knightAttack(_tzcnt_u64(p));
		bSpace += 2 * __popcnt64(attack & wSpaceMask) + __popcnt64(attack);
		bSpace += 4*center[_tzcnt_u64(p)];
		bAttacks += __popcnt64(wKingClose & attack);
		solid -= 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}

	//bishop eval
	p = pos->bitBoard[9];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc|bOcc,_tzcnt_u64(p));
		wSpace += 2 * __popcnt64(attack & bSpaceMask) + __popcnt64(attack);
		wAttacks += __popcnt64(bKingClose & attack);
		solid += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[2];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p));
		bSpace += 2 * __popcnt64(attack & wSpaceMask) + __popcnt64(attack);
		bAttacks += __popcnt64(wKingClose & attack);
		solid -= 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}

	//rook is 5 pawns no matter what
	p = pos->bitBoard[10];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		wSpace +=  __popcnt64(attack & bSpaceMask) + __popcnt64(attack);
		wAttacks += __popcnt64(bKingClose & attack);
		solid += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[1];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		bSpace += __popcnt64(attack & wSpaceMask) + __popcnt64(attack);
		bAttacks += __popcnt64(wKingClose & attack);
		solid -= 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}

	//Queen eval
	p = pos->bitBoard[11];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p))| rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		wSpace += 1 * __popcnt64(attack & bSpaceMask) + __popcnt64(attack);
		wAttacks += 2*__popcnt64(bKingClose & attack);
		solid += 1 * __popcnt64(bPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[7]);
		p &= ~getBit(_tzcnt_u64(p));
	}
	p = pos->bitBoard[0];
	range = __popcnt64(p);
	for (int i = 0; i < range; i++) {
		unsigned long long attack = bishopAttack(wOcc | bOcc, _tzcnt_u64(p)) | rookAttack(wOcc | bOcc, _tzcnt_u64(p));
		bSpace += 1 * __popcnt64(attack & wSpaceMask) + __popcnt64(attack);
		bAttacks += 2*__popcnt64(wKingClose & attack);
		solid -= 1 * __popcnt64(wPawnAttack(_tzcnt_u64(p)) & pos->bitBoard[4]);
		p &= ~getBit(_tzcnt_u64(p));
	}


	int endGamePhase =( 2* 100000+20000-wm-bm)/256;
	int middleGamePhase = (wm + bm -2 * 100000-10240) / 256;
	if (middleGamePhase < 0) {
		middleGamePhase = 0;
	}

	int side = -5;
	if (pos->side) {
		side = 5;
	}

	int safety = 4 * __popcnt64(bishopAttack(bOcc, _tzcnt_u64(pos->bitBoard[5])) | rookAttack(bOcc, _tzcnt_u64(pos->bitBoard[5]))) - 4 * __popcnt64(bishopAttack(wOcc, _tzcnt_u64(pos->bitBoard[6])) | rookAttack(wOcc, _tzcnt_u64(pos->bitBoard[6])));
	return wm - bm + wSpace - bSpace + 3 * solid+side+safety*middleGamePhase/60 +endGamePhase * (wEndGameSpace - bEndGameSpace) / 70;
}