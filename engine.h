#pragma once
#include "bitboard.h"

#define WHITE true
#define BLACK false

void initPosition();
void initSearch();

struct search {
	bool searching;
	bool init;
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
};
struct search* getSearchPointer();
struct position* getPositionPointer();

