#pragma once
#define N -8
#define NE -7
#define E +1
#define SE +9
#define S +8
#define SW +7
#define W -1
#define NW -9

void initBits();
int getCord(char letter);

unsigned long long getBit(int index);
bool isLegal(bool side, struct position*);
bool isLegalSQ(bool side, struct position*, int cord);
void printBoard();
void printBitBoard(unsigned long long board);
bool hasKings(struct position* pos);
struct position {
	bool side;
	unsigned long long bitBoard[12];
	unsigned long long enPassant;
	bool castle[4];
	unsigned long long hash;
	int mov50;
};


