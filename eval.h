#pragma once
#include "tune.h"
#define mateScore 100000
#define wMateScore 100000
#define bMateScore -100000

/*#define pawnMiddleGame 354
#define knightMiddleGame 2680
#define bishopMiddleGame 2710
#define rookMiddleGame 3264
#define queenMiddleGame 6513

#define pawnEndGame 625
#define knightEndGame 1289
#define bishopEndGame 1466
#define rookEndGame 2548
#define queenEndGame 5613*/

#define pawnMiddleGame 870
#define knightMiddleGame 3516
#define bishopMiddleGame 3089
#define rookMiddleGame 4305
#define queenMiddleGame 10466

#define pawnEndGame 924
#define knightEndGame 3783
#define bishopEndGame 3637
#define rookEndGame 6310
#define queenEndGame 10754


//redundant terms
#define pawnPhase 0
#define knigthPhase 1
#define bishopPhase 1
#define rookPhase 2
#define queenPhase 4

#define totalPhase 24


#define centerMask 66229406269440

struct tuneVector* getTuneVector();
void fillEvalTables();
void showStatic();
int eval(struct position* pos);
int evals(struct position* pos);
int materialEval(struct position* pos);
extern int wpawnendgame[64];
extern int bpawnendgame[64];
extern int wpawnmiddlegame[64];
extern int bpawnmiddlegame[64];
extern int wknightPSQT[64];
extern int bknightPSQT[64];
extern int wKingMiddleGame[64];
extern int bKingMiddleGame[64];
extern int wkingendgamecenter[64];
extern int bkingendgamecenter[64];
void testf(int);

/*

0 == PAWNMIDDLEGAMESPACE
1 == PAWNENDGAMESPACE

2 == ISOLANIEFFECT
3 == DOUBLEDEFFECT

4 == BISHOP MIDDLEGAME MOBILITY
5 == BISHOP ENDGAME MOBILITY

6 == KNIGHT PSQT MIDDLEGAME
7 == KNIGHT MOBILITY MIDDLEGAME
8 == KNIGHT PSQT ENDGAME

9 == ROOK MIDDLEGAME MOB
10 == ROOK ENDGAME MOB

9 == ROOK MIDDLEGAME MOB
10 == ROOK ENDGAME MOB

11 == QUEEN MIDDLEGAME MOB
12 == QUEEN ENDGAME MOB

13 == BISHOP PAIR BONUS
14 == KNIGHT PAIR BONUS

15 == king defenders count

16 == kingShield count

17 == kingPSQTMIDDLEGAME

18 == kingPSQTENDGAME

19 == atcounter
*/
