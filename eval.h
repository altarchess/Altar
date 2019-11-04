#pragma once
#define wSpaceMask 4294967295
#define bSpaceMask 18446744069414584320
#define mateScore 100000
#define wMateScore 100000
#define bMateScore -100000
#define pawnMiddleGame 256
#define knightMiddleGame 768
#define bishopMiddleGame 800
#define rookMiddleGame 1280
#define queenMiddleGame 2304



int eval(struct position* pos);
