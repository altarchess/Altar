#include "tt.h"
#include <random>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <malloc.h>

unsigned long long ttrndp[12][64];
unsigned long long ttrndc[4];
unsigned long long ttrnde[16];
unsigned long long ttside;
struct ttEntry* tt;

int ttSize = 0;


/*unsigned long long getRnd(){
	unsigned long long n1 = std::rand();
	unsigned long long n2 = std::rand();
	return n1 + n2 << 32;
}*/
unsigned long long getRnd()
{
	unsigned long long rand1 = abs(rand());
	unsigned long long rand2 = abs(rand());
	rand1 = rand1 << (sizeof(int) * 8);
	unsigned long long randULL = (rand1 | rand2);
	return randULL;
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


	ttside = getRnd();
	return;
}

void setTTSize(int size) {
	free(tt);
	tt = (ttEntry*)malloc((size+1)*sizeof(ttEntry));
	ttSize = size;
	/*if (tt==nullptr)
	{
		std::cout << "malloc fucked up";
	}
	else
	{
		std::cout << "malloc succesfull";
	}*/

	for (int i = 0; i < ttSize; i++) {
		tt[i].depth = 0;
		tt[i].type = 0;
		tt[i].zHash = 0;
		tt[i].eval = 0;
	}
};

int ttProbe(unsigned long long hash, int depth, int alpha, int beta, int* bm) {
	if (!ttSize)return invalid;

	struct ttEntry ttTest = tt[hash % ttSize];

	if (ttTest.zHash == hash) {


		//std::cout << "gobm";
		*bm = ttTest.move;

		if (ttTest.depth >= depth) {

			if (ttTest.type == ttExact)
				return ttTest.eval;

			if ((ttTest.type == ttLower) && (ttTest.eval <= alpha))
				return alpha;

			if ((ttTest.type == ttUpper) && (ttTest.eval >= beta)){
				return beta;
			}
		}

	}

	return invalid;

}

void ttSave(int depth, unsigned long long hash, int eval, int type, int best) {

	if (!ttSize)return;

	ttEntry* ttSave2 = &tt[hash % ttSize];

	if ((ttSave2->zHash == hash) && (ttSave2->depth >= depth)) return;

	ttSave2->zHash = hash;
	ttSave2->eval = eval;
	ttSave2->type = type;
	ttSave2->depth = depth;
	ttSave2->move = best;

	return;
}