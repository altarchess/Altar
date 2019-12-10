#pragma once

#define WINSCORE 0
#define DRAWSCORE 1
#define LOSSSCORE 2

struct tuneVector {
	int MODIF[400];
	int active;
};

void tuneVal(tuneVector* v);
void tuneK();