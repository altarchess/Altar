#pragma once
#include "move.h"

struct moveList {
	unsigned long long enPassant;
	unsigned long long castle[4];
	struct move MOVE[100];
	bool hashM;
	bool killer1;
	bool killer2;
	int gcapt;
	int mam;
};
struct scores {
	int score[100];
	bool quiet[100];
};

struct QList {
	struct move MOVE[25];
	int mam;
};

struct moveTable {
	struct moveList mvl[100];
	struct scores score[100];
};

struct QTable {
	struct QList QL[25];
};

void genAllMoves(struct moveList*, bool side, struct position*);
void genAllCaptures(struct QList*,bool side, struct position*);