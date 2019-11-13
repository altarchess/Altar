#include "tt.h"
#include "move.h"
#include "bitboard.h"
#include <random>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <malloc.h>
#include <immintrin.h>
#include <intrin.h>

unsigned long long ttrndp[12][64];
unsigned long long ttrndc[4];
unsigned long long ttrnde[16];
unsigned long long ttside;
unsigned long long ttSize;
struct ttEntry* tt;


unsigned long long getRnd(){
	unsigned long long n1 = std::rand();
	unsigned long long n2 = std::rand();
	unsigned long long n3 = std::rand();
	unsigned long long n4 = std::rand();
	unsigned long long n5 = std::rand();
	unsigned long long n6 = std::rand();
	unsigned long long n7 = std::rand();
	unsigned long long n8 = std::rand();
	unsigned long long n9 = std::rand();
	return n1 + 1000*n2+1000000*n3+n4*1000000000 + n5 * 1000000000000 + n6 * 1000000000000 + n7 * 1000000000000000 + n8 * 1000000000000000000+n9 * 10000000000000000000;
}

/*unsigned long long getRnd()
{
	unsigned long long rand1 = abs(rand());
	unsigned long long rand2 = abs(rand());
	rand1 = rand1 << (sizeof(int) * 8);
	unsigned long long randULL = (rand1 | rand2);
	return randULL;
}*/

void fillTables() {
	std::srand(std::time(0)); // use current time as seed for random generator

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

unsigned long long getHash(struct position* pos) {
	unsigned long long hash = 0;
	for (int i = 0; i < 64; i++) {
		if (getPiece(pos, i) > 0) {
			hash ^= ttrndp[getPiece(pos, i) - 1][i];
		}
	}
	for (int i = 0; i < 4; i++) {
		if (pos->castle[i]) {
			hash ^= ttrndc[i];
		}
	}

	if (pos->enPassant > 0) {
		hash ^= ttrnde[_tzcnt_u64(pos->enPassant) - 48];
	}
	return hash;
}

void setTTSize(int size) {
	//free(tt);
	
	if (tt != NULL) {
		free(tt);
	}
	tt = (ttEntry*)malloc((size)*sizeof(ttEntry));
	ttSize = size-2;
	if (tt==NULL)
	{
		std::cout << "malloc fucked up";
	}
	else
	{
	}

	for (int i = 0; i < ttSize; i++) {
		tt[i].move = 0;
		tt[i].zHash = 0;
		tt[i].eval = 0;
	}

};

int ttProbe(unsigned long long hash, int depth, int alpha, int beta, int* bm) {
	if (!ttSize)return invalid;


	if (tt[hash % ttSize].zHash == hash) {


		//std::cout << "gobm";
		*bm = tt[hash % ttSize].move;

		if (tt[hash % ttSize].depth >= depth) {

			if (tt[hash % ttSize].type == ttExact)
				return tt[hash % ttSize].eval;

			/*if ((ttTest.type == ttLower) && (ttTest.eval <= alpha))
				return alpha;

			if ((ttTest.type == ttUpper) && (ttTest.eval >= beta)){
				return beta;
			}*/
		}

	}

	return invalid;

}

void ttSave(int depth, unsigned long long hash, int eval, int type, int best) {

	if (!ttSize)return;


	if ((tt[hash % ttSize].zHash == hash) && (tt[hash % ttSize].depth > depth)) { return; };
	//if ((tt[hash % ttSize].type == 0) && (type!= 0)) { return; }; 
	if ((tt[hash % ttSize].type == 0) && (tt[hash % ttSize].depth>depth)) { return; };
	//if (type == 0) { std::cout << eval << std::endl; };

	tt[hash% ttSize].zHash = hash;
	tt[hash% ttSize].eval = eval;
	tt[hash% ttSize].type = type;
	tt[hash% ttSize].depth = depth;
	tt[hash% ttSize].move = best;


	return;
}