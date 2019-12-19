#pragma once
#define ttExact 0
#define ttUpper 1
#define ttLower 2
#define invalid 666666;

struct ttEntry {
	unsigned long long zHash;
	int eval;
	int type;
	int depth;
	int move;
	int age;
};

extern unsigned long long ttrndp[12][64];
extern unsigned long long ttrndc[4];
extern unsigned long long ttrnde[16];
extern unsigned long long ttside;
extern unsigned long long ttSize;
extern struct ttEntry* tt;

unsigned long long getHash(struct position* pos);
int ttProbe(unsigned long long, int , int, int , int*);
void ttSave(int, unsigned long long, int, int, int, bool pvnode);
void setTTSize(int);
void fillTables();
void ageTT();