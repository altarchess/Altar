#include "tt.h"
#include <random>
#include <iostream>
#include <cstdlib>
#include <ctime>


unsigned long long ttrndp[12][64];
unsigned long long ttrndc[4];
unsigned long long ttrnde[16];
unsigned long long ttside;

struct ttEntry* tt;

int ttSize = 0;


unsigned long long getRnd(){
	unsigned long long n1 = std::rand();
	unsigned long long n2 = std::rand();
	return n1 + n2 << 32;
}

void fillTables() {
	std::srand(std::time(nullptr)); // use current time as seed for random generator

	for (int i = 0; i < 12; i++) {
		for (int e = 0; e < 64; e++) {
			ttrndp[i][e] = getRnd();
		}
	}


	for (int i = 0; i < 4; i++) {
		ttrndc[i] = getRnd();
	}

	for (int i = 0; i < 16; i++) {
		ttrnde[i] = getRnd();
	}

	ttrnde[16] = 0;

	ttside = getRnd();
	return;
}

void setTTSize(int size) {
	free(tt);
	ttSize = size;
	tt = (ttEntry*)malloc(size);
};

int ttProbe(unsigned long long hash, int depth, int alpha, int beta, int* bm) {

	if (!ttSize)return invalid;

	struct ttEntry ttTest = tt[hash / ttSize];

	if (ttTest.zHash == hash) {


		*bm = ttTest.move;


		if (ttTest.depth >= depth) {

			if (ttTest.type == ttExact)
				return ttTest.eval;

			if ((ttTest.type == ttLower) && (ttTest.eval <= alpha))
				return alpha;

			if ((ttTest.type == ttUpper) && (ttTest.eval >= beta))
				return beta;

		}

	}

	return invalid;

}

void ttSave(int depth, unsigned long long hash, int eval, int type, int best) {

	if (!ttSize)return;

	ttEntry* ttSave = &tt[hash / ttSize];

	if ((ttSave->zHash == hash) && (ttSave->depth > depth)) return;

	ttSave->zHash = hash;
	ttSave->eval = eval;
	ttSave->type = type;
	ttSave->depth = depth;
	ttSave->move = best;

	return;
}