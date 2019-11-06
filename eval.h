#pragma once
#define mateScore 100000
#define wMateScore 100000
#define bMateScore -100000
#define pawnMiddleGame 256
#define knightMiddleGame 768
#define bishopMiddleGame 800
#define rookMiddleGame 1280
#define queenMiddleGame 2304

#define centerMask 66229406269440
void fillEvalTables();
int eval(struct position* pos);
