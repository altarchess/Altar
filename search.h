#pragma once
extern int RAZOR_DEPTH;
extern int FUTILITY_DEPTH;
extern int IID_DEPTH ;
extern int LMP_DEPTH ;
extern int RAZOR_MARGIN;
extern int FUTILITY_MARGIN ;
extern int HISTORYDIV ;
extern int reductionDiv ;
struct historyhash {
	unsigned long long hh[600];
	int index;
};
int max(int, int);
int min(int a, int b);
void launchThreads(struct search* s, struct position* pos, struct historyhash hh);
void mainSearch(struct search*, struct position*, struct historyhash, int threadID);
void searchManager(struct search*);
void resetHistory();
int see(struct position* pos, int from, int to);
int Quis(struct position pos, int alpha, int beta, int ply, struct QTable* ct); //for tuning