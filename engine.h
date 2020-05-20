#pragma once
#include "bitboard.h"

#define WHITE true
#define BLACK false

#define MAX_THREADS 8

void initPosition();
void initSearch();

struct search {
	bool searching;
	bool init;
	int killers[MAX_THREADS][100][2];
	int lMove[MAX_THREADS][200][2];
	unsigned long long time;
	unsigned long long sTime;
	int timetype;
	int inc;
	int movesToGo;
	int depth;
	unsigned long long nodeCount;
	int bff;
	int bft;
	int reacheddepth;
	int threadCount;
	int inNull[MAX_THREADS];
	int smpMaxDepth;
	int smpMaxScore;
};
struct search* getSearchPointer();
struct position* getPositionPointer();

