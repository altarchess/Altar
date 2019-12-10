#pragma once

struct historyhash {
	unsigned long long hh[600];
	int index;
};
int max(int, int);
void mainSearch(struct search*, struct position*, struct historyhash);
void searchManager(struct search*);
void resetHistory();
int see(struct position* pos, int from, int to);
int Quis(struct position pos, int alpha, int beta, int ply, struct QTable* ct); //for tuning