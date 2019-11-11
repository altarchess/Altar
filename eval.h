#pragma once
#define mateScore 100000
#define wMateScore 100000
#define bMateScore -100000

#define pawnMiddleGame 198
#define knightMiddleGame 817
#define bishopMiddleGame 836
#define rookMiddleGame 1270
#define queenMiddleGame 2521

#define pawnEndGame 258
#define knightEndGame 846
#define bishopEndGame 857
#define rookEndGame 1278
#define queenEndGame 2558

#define centerMask 66229406269440
void fillEvalTables();
int eval(struct position* pos);
int evals(struct position* pos);
