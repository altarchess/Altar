#pragma once
#define wSpaceMask 4294967295
#define bSpaceMask 18446744069414584320
#define mateScore 1000000
#define wMateScore 1000000
#define bMateScore -1000000
#define pawnMiddleGame 256
#define knightMiddleGame 768
#define bishopMiddleGame 800
#define rookMiddleGame 1280
#define queenMiddleGame 2304
int eval(struct position* pos);
