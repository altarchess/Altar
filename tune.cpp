﻿#include "tune.h"
#include "engine.h"
#include "search.h"
#include "bitboard.h"
#include "uci.h"
#include "eval.h"
#include "movelist.h"

#include <fstream>
#include <math.h>
#include <iostream>
#include <string>
#include <mutex>
#include <thread>

#define WWIN "1-0"
#define BWIN "0-1"
#define DRAW "1/2-1/2"

int dirTable[400];
char wWin[] = WWIN;
char bWin[] = BWIN;
char draw[] = DRAW;
int lr; // learning rate for tuning;
unsigned long long nodeCOUNT;
struct position* p;
struct QTable ct;
double k = 2.5245;
unsigned long long oldCost;
unsigned long long cost;
std::mutex costm;
bool isPartOf(char* w1, char* w2)
{
	int i = 0;
	int j = 0;


	while (w1[i] != '\0') {
		if (w1[i] == w2[j])
		{
			int init = i;
			while (w1[i] == w2[j] && w2[j] != '\0')
			{
				j++;
				i++;
			}
			if (w2[j] == '\0') {
				return true;
			}
			j = 0;
		}
		i++;
	}
	return false;
}
void LoadPositions() {
	if (p != NULL) {
		free(p);
	}
	p = (position*)malloc((703755) * sizeof(position));
	
	if (p == NULL)
	{
		std::cout << "malloc fucked up";
	}
	else
	{
		int index = 0;
		std::ifstream wfile("C:\\Users\\kimka\\Desktop\\Altar\\quiet-labeled.epd");
		if (wfile.is_open()) {
			std::string line;
			while (std::getline(wfile, line)) {
				parseFEN(&line[0]);
				if (hasKings(getPositionPointer()) && isLegal(true, getPositionPointer()) && isLegal(false, getPositionPointer())) {
					p[index] = *getPositionPointer();

					if (isPartOf(&line[0], wWin)) {
						p[index].mov50 = 0;
					}
					if (isPartOf(&line[0], draw)) {
						p[index].mov50 = 1;
					}
					if (isPartOf(&line[0], bWin)) {
						p[index].mov50 = 2;
					}
					index++;
				}
			}
		}
		std::cout << index << " POSITIONS LOADED" << std::endl;
	}


}

struct tuneVector* t;
struct tuneVector best;
void setTuneVector(tuneVector tune) {
	*t = tune;
}
void costFunction(int eval, int score) {
	nodeCOUNT++;
	if (eval < -20000) {
		eval = -20000;
	}
	if (eval > 20000) {
		eval = 20000;
	}
	double ev = eval;
	double wp = 1000.0 / (1.0 + pow(10.0, (-ev*k/4000.0)));


	double rcost = 0;
	switch (score) {
	case WINSCORE:
		rcost = abs(1000.0- wp);
		break;
	case DRAWSCORE:
		rcost = abs(500.0 - wp);
		break;
	case LOSSSCORE:
		rcost = abs(0.0 - wp);
	}
	cost += rcost * rcost;
	return;
}
void LoopWins() {
	for (int i = 0; i < 703755; i++) {
		if (p[i].mov50 != 0) { continue; };
		int score = eval(&p[i]);// Quis(p[i], -mateScore, mateScore, 0, &ct);
		if (!p[i].side) {
			score *= -1;
		}
		costm.lock();
		costFunction(score, WINSCORE);
		costm.unlock();
	}
}
void LoopDraws() {
	for (int i = 0; i < 70355; i++) {
		if (p[i].mov50 != 1) { continue; };
		int score = eval(&p[i]);// Quis(p[i], -mateScore, mateScore, 0, &ct);
		if (!p[i].side) {
			score *= -1;
		}
		costm.lock();
		costFunction(score, DRAWSCORE);
		costm.unlock();
	}
}
void LoopLosses() {
	for (int i = 0; i < 703755; i++) {
		if (p[i].mov50 != 2) { continue; };
		int score = eval(&p[i]);// Quis(p[i], -mateScore, mateScore, 0, &ct);
		if (!p[i].side) {
			score *= -1;
		}
		costm.lock();
		costFunction(score, LOSSSCORE);
		costm.unlock();
	}
}

bool isBetter() {
	std::thread wins(LoopWins);
	std::thread draws(LoopDraws);
	std::thread losses(LoopLosses);
	wins.join();
	draws.join();
	losses.join();
	if (cost < oldCost) {
		oldCost = cost;
		cost = 0;
		return true;
	}
	else {
		cost = 0;
		return false;
	}
};
void randomTuneTable(int index) {
	int y = index / 8;
	int x = index - y * 8;
	unsigned long long rnd = std::rand();
	int mult = 1;
	if (rnd % 2 == 0) {
		//std::cout << "OKASD";
		wqueenPSQT[y*8+x] += lr;
		bqueenPSQT[63 - (y*8+x)] += lr;
		if (isBetter()) {
			return;
		}
		wqueenPSQT[y * 8 + x] -= lr;
		bqueenPSQT[63 - (y * 8 + x)] -= lr;
	}
	else {
		wqueenPSQT[y * 8 + x] -= lr;
		bqueenPSQT[63 - (y * 8 + x)] -= lr;
		if (isBetter()) {
			return;
		}
		wqueenPSQT[y * 8 + x] += lr;
		bqueenPSQT[63 - (y * 8 + x)] += lr;
		int mult = -1;
	}
	if (isBetter()) {
		return;
	}
	return;
};
struct tuneVector randomTune(int index, struct tuneVector tune) {
	tuneVector tune2 = tune;
	unsigned long long rnd = std::rand();
	if (dirTable[index] == 1) {
		tune2.MODIF[index] += lr;
	}
	else {
		tune2.MODIF[index] -= lr;
	}
	setTuneVector(tune2);
	if (isBetter()) {
		best = tune2;
	}
	else {
		dirTable[index] *= -1;
	}
	return best;
};
void tuneK() {
	LoadPositions();
	LoopWins();
	LoopDraws();
	LoopLosses();
	oldCost = cost;
	cost = 0;
	float direction = 0.0001;
	for (int i = 0; i < 100; i++) {
		float oldk = k;
		k = k + direction;
		std::thread wins(LoopWins);
		std::thread draws(LoopDraws);
		std::thread losses(LoopLosses);
		wins.join();
		draws.join();
		losses.join();
		if (cost < oldCost) {
			oldCost = cost;
			cost = 0;
		}
		else {
			direction = -direction;
			cost = 0;
			k = oldk;
		}

		std::cout << "CURRENT K FACTOR IS: " << k;
	}
};
void tuneVal(tuneVector* adress) {
	for (int i = 0; i < 350; i++) {
		dirTable[i] = 1;
	}
	lr = 1;
	LoadPositions();
	nodeCOUNT = 0;
	t = adress;
	best = *t;
	LoopWins();
	LoopDraws();
	LoopLosses();

	oldCost = cost;
	cost = 0;
	std::cout << "Current Cost " << oldCost << std::endl;

	for (int i = 0; i < 1000; i++) {
		/*if (i % 25 == 0 && i!=0) {
			lr = lr / 2;
			std::cout << std::endl;
			std::cout << std::endl;
			std::cout << "LR DROP!!!!!! LR NOW AT" << lr;;
			std::cout << std::endl;
			std::cout << std::endl;
		}*/
		/*for (int e = 0; e < 64; e++) {
			randomTuneTable(e);
			std::cout << "#";
		}
		std::cout << std::endl << "ITERATION " << i << " COST " << oldCost << " NODES " << nodeCOUNT << std::endl;
		for (int e = 0; e < 64; e++) {
			if (e % 8 == 0) {
				std::cout << std::endl;
			}
			std::cout << wqueenPSQT[e] << ", ";
		}

		for (int e = 0; e < 64; e++) {
			if (e % 8 == 0) {
				std::cout << std::endl;
			}
			std::cout << bqueenPSQT[e] << ", ";
		}*/
		for (int e = 0; e < adress->active; e++) {
			*adress = randomTune(e, *adress);
			std::cout << "#";
		}
		std::cout << std::endl << "ITERATION " << i << " COST " << oldCost << " NODES " << nodeCOUNT<<  std::endl;
		for (int e = 0; e < adress->active; e++) {
			std::cout << adress->MODIF[e] << ", ";
		}
	}

};



