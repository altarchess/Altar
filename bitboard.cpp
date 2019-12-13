#include "bitboard.h"
#include "engine.h"
#include <intrin.h>
#include <iostream>
#include "gen.h"


unsigned long long bits[65];
char pieces[] = { 'q','r','b','n','p','k','K','P','N','B','R','Q' };
int getCord(char letter) {
	switch (letter) {
	case 'a':
		return 0;
	case 'b':
		return 1;
	case 'c':
		return 2;
	case 'd':
		return 3;
	case 'e':
		return 4;
	case 'f':
		return 5;
	case 'g':
		return 6;
	case 'h':
		return 7;
	case '1':
		return 56;
	case '2':
		return 48;
	case '3':
		return 40;
	case '4':
		return 32;
	case '5':
		return 24;
	case '6':
		return 16;
	case '7':
		return 8;
	case '8':
		return 0;
	}
	return 0;
}
bool hasKings(struct position* pos) {
	if (pos->bitBoard[6] && pos->bitBoard[5]) {
		return true;
	}
	return false;
}
bool isLegal(bool side, struct position* pos) {
	if (side) {
		unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
		unsigned long long bOcc = pos->bitBoard[0] | pos->bitBoard[1] | pos->bitBoard[2] | pos->bitBoard[3] | pos->bitBoard[4] | pos->bitBoard[5];
		int cord = _tzcnt_u64(pos->bitBoard[6]);
		if (knightAttack(cord) & (pos->bitBoard[3])) {
			return false;
		}
		if (bishopAttack(wOcc|bOcc, cord) & (pos->bitBoard[2]|pos->bitBoard[0])) {
			return false;
		}
		if (rookAttack(wOcc | bOcc, cord) & (pos->bitBoard[1] | pos->bitBoard[0])) {
			return false;
		}
		if (wPawnAttack(cord)&pos->bitBoard[4]) {
			return false;
		}
		if (kingAttack(cord) & pos->bitBoard[5]) {
			return false;
		}
		return true;
	}
	else {
		unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
		unsigned long long bOcc = pos->bitBoard[0] | pos->bitBoard[1] | pos->bitBoard[2] | pos->bitBoard[3] | pos->bitBoard[4] | pos->bitBoard[5];
		int cord = _tzcnt_u64(pos->bitBoard[5]);
		if (knightAttack(cord) & (pos->bitBoard[8])) {
			return false;
		}
		if (bishopAttack(wOcc | bOcc, cord) & (pos->bitBoard[9] | pos->bitBoard[11])) {
			return false;
		}
		if (rookAttack(wOcc | bOcc, cord) & (pos->bitBoard[10] | pos->bitBoard[11])) {
			return false;
		}
		if (bPawnAttack(cord) & pos->bitBoard[7]) {
			return false;
		}
		if (kingAttack(cord) & pos->bitBoard[6]) {
			return false;
		}
		return true;
	}
}
bool isLegalSQ(bool side, struct position* pos, int cord) {
	if (side) {
		unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
		unsigned long long bOcc = pos->bitBoard[0] | pos->bitBoard[1] | pos->bitBoard[2] | pos->bitBoard[3] | pos->bitBoard[4] | pos->bitBoard[5];
		if (knightAttack(cord) & (pos->bitBoard[3])) {
			return false;
		}
		if (bishopAttack(wOcc | bOcc, cord) & (pos->bitBoard[2] | pos->bitBoard[0])) {
			return false;
		}
		if (rookAttack(wOcc | bOcc, cord) & (pos->bitBoard[1] | pos->bitBoard[0])) {
			return false;
		}
		if (bPawnAttack(cord) & pos->bitBoard[4]) {
			return false;
		}
		return true;
	}
	else {
		unsigned long long wOcc = pos->bitBoard[6] | pos->bitBoard[7] | pos->bitBoard[8] | pos->bitBoard[9] | pos->bitBoard[10] | pos->bitBoard[11];
		unsigned long long bOcc = pos->bitBoard[0] | pos->bitBoard[1] | pos->bitBoard[2] | pos->bitBoard[3] | pos->bitBoard[4] | pos->bitBoard[5];
		if (knightAttack(cord) & (pos->bitBoard[8])) {
			return false;
		}
		if (bishopAttack(wOcc | bOcc, cord) & (pos->bitBoard[9] | pos->bitBoard[11])) {
			return false;
		}
		if (rookAttack(wOcc | bOcc, cord) & (pos->bitBoard[10] | pos->bitBoard[11])) {
			return false;
		}
		if (wPawnAttack(cord) & pos->bitBoard[7]) {
			return false;
		}
		return true;
	}
}
void initBits() {
	unsigned long long value = 1;
	for (int i = 0; i < 64; i++) {
		bits[i] = value;
		value = value<<1;
	};
	bits[64] = 0;
	return;
};
unsigned long long getBit(int index) {
	return bits[index];
}
void printBoard() {
	struct position pos = *getPositionPointer();
	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0) {
			std::cout << "\n" << i / 8 + 1 << " ";
		}
		bool flag = false;
		for (int e = 0; e < 12; e++) {
			if (pos.bitBoard[e] & getBit(i)) {
				flag = true;
				std::cout << pieces[e];
				//std::cout << e;
			}
		}
		if (!flag) {
			if ((1 + i + i / 8) % 2 == 0) {
				std::cout << "#";
			}
			else {
				std::cout << " ";
			}
		}
	}


}
void printBitBoard(unsigned long long board) {
	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0) {
			std::cout << "\n" << i / 8 + 1 << " ";
		}
		if (board & getBit(i)) {
				std::cout << 1;
		}
		else {
			std::cout << 0;
		}
	}
	return;
}