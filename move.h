#pragma once
#include "move.h"
//move 32 bits movetype 3 bit 6 bits from 6 bits to 4 bits from piece 4 bits to piece
#define fromShift 3
#define toShift 9
#define pieceType 15
#define otherpcord 21
#define otherPieceType 27
#define moveTypeMask 7
#define moveCordMask 63
#define wEnPassantMask 65280
#define bEnPassantMask 255
struct move {
	int type;
	int f;
	int t;
};

struct mres {
	int p;
	int otherp;
	int othercord;
};
void printBestMove(int, int, struct position*);
int getPiece(struct position*, int);
struct position makeMove(struct move, struct position);
void makeNull(struct position* pos);
char* moveToChar(int mov);
struct move bufToMove(char* buf);
struct move ttMoveToMove(int mov);