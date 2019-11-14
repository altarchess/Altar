#pragma once

struct historyhash {
	unsigned long long hh[600];
	int index;
};

void mainSearch(struct search*, struct position*, struct historyhash);
void searchManager(struct search*);
void resetHistory();
int see(struct position* pos, int from, int to);
