#pragma once
#define ttExact 0
#define ttUpper 1
#define ttLower 2
#define invalid 666666;


#define ageMask 1
#define typeMask 6
#define moveMask 65528
#define depthMask 134152192
#define ageShift 0
#define typeShift 1
#define moveShift 3
#define depthShift 16
struct ttEntry {
	unsigned long long zHash;
	int eval;
	int type;
	int depth;
	int move;
	int age;
};

struct ttEntryCompressed {
	unsigned long long zHash;
	int eval;
	int depthmovetypeage;
};
extern unsigned long long ttrndp[12][64];
extern unsigned long long ttrndc[4];
extern unsigned long long ttrnde[16];
extern unsigned long long ttside;
extern unsigned long long ttSize;
extern struct ttEntryCompressed* tt;

unsigned long long getHash(struct position* pos);
struct ttEntry ttProbe(unsigned long long);
void ttSave(int, unsigned long long, int, int, int, bool pvnode);
void setTTSize(int);
void fillTables();
void ageTT();