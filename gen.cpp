#include "gen.h"
#include "bitboard.h"

#include <immintrin.h>
#include <intrin.h>
#include <iostream>

unsigned long long arrRookAttacks[102400];
unsigned long long arrBishopAttacks[5248];
unsigned long long arrKnightAttacks[64];
unsigned long long arrKingAttacks[64];
unsigned long long arrWPawnAttacks[64];
unsigned long long arrBPawnAttacks[64];

int arrRookBase[64];
int arrBishopBase[64];

unsigned long long arrRookMask[64];
unsigned long long arrBishopMask[64];

unsigned long long genRookMask(int sq) {
	
	unsigned long long mask = 0;

	int y = sq / 8;
	int x = sq - y * 8;

	//Nray
	while (y > 1) {
		y--;
		mask |= getBit(y*8+x);
	}

	y = sq / 8;
	x = sq - y * 8;

	//Sray
	while (y < 6) {
		y++;
		mask |= getBit(y * 8 + x);
	}

	y = sq / 8;
	x = sq - y * 8;

	//Wray
	while (x > 1) {
		x--;
		mask |= getBit(y * 8 + x);
	}

	y = sq / 8;
	x = sq - y * 8;

	//Eray
	while (x < 6) {
		x++;
		mask |= getBit(y * 8 + x);
	}

	return mask;

}

unsigned long long genBishopMask(int sq) {
	
	unsigned long long mask = 0;

	int y = sq / 8;
	int x = sq - y * 8;

	//NWray
	while (y > 1 && x > 1) {
		y--;
		x--;
		mask |= getBit(y * 8 + x);
	}

	y = sq / 8;
	x = sq - y * 8;

	//SWray
	while (y < 6 && x > 1) {
		y++;
		x--;
		mask |= getBit(y * 8 + x);
	}

	y = sq / 8;
	x = sq - y * 8;

	//SEray
	while (y < 6 && x < 6) {
		y++;
		x++;
		mask |= getBit(y * 8 + x);
	}

	y = sq / 8;
	x = sq - y * 8;

	//NEray
	while (y > 1 && x < 6) {
		y--;
		x++;
		mask |= getBit(y * 8 + x);
	}

	return mask;

}

unsigned long long genSliding(int sq, int direction, unsigned long long occ) {
	
	unsigned long long set = 0;

	int y = sq / 8;
	int x = sq - y * 8;

	bool flag = true;
	while (flag) {
		switch (direction) {
		case N:
			if (y > -1) {
				if (occ & getBit(x + y * 8)) {
					flag = false;
				}
				set |= getBit(x+y*8);
				y--;
			}
			else {
				flag = false;
			}
			break;
		case NE:
			if (y > -1 && x < 8) {
				if (occ & getBit(x + y * 8)) {
					flag = false;
				}
				set |= getBit(x + y * 8);
				y--;
				x++;
			}
			else {
				flag = false;
			}
			break;
		case E:
			if (x < 8) {
				if (occ & getBit(x + y * 8)) {
					flag = false;
				}
				set |= getBit(x + y * 8);
				x++;
			}
			else {
				flag = false;
			}
			break;
		case SE:
			if (y<8 && x < 8) {
				if (occ & getBit(x + y * 8)) {
					flag = false;
				}
				set |= getBit(x + y * 8);
				x++;
				y++;
			}
			else {
				flag = false;
			}
			break;
		case S:
			if (y < 8) {
				if (occ & getBit(x + y * 8)) {
					flag = false;
				}
				set |= getBit(x + y * 8);
				y++;
			}
			else {
				flag = false;
			}
			break;
		case SW:
			if (y < 8 && x > -1) {
				if (occ & getBit(x + y * 8)) {
					flag = false;
				}
				set |= getBit(x + y * 8);
				x--;
				y++;
			}
			else {
				flag = false;
			}
			break;
		case W:
			if (x > -1) {
				if (occ & getBit(x + y * 8)) {
					flag = false;
				}
				set |= getBit(x + y * 8);
				x--;
			}
			else {
				flag = false;
			}
			break;
		case NW:
			if (y > -1 && x > -1) {
				if (occ & getBit(x + y * 8)) {
					flag = false;
				}
				set |= getBit(x + y * 8);
				x--;
				y--;
			}
			else {
				flag = false;
			}
			break;
		} 
	}
	return set;

}

unsigned long long genRookAttack(int sq, unsigned long long i){
	unsigned long long attacks = 0;
	unsigned long long mask = arrRookMask[sq];
	unsigned long long board = _pdep_u64(i, mask);
	return (genSliding(sq, N, board) | genSliding(sq, E, board) | genSliding(sq, S, board) | genSliding(sq, W, board))&~getBit(sq);
};

unsigned long long genBishopAttack(int sq, unsigned long long i) {
	unsigned long long attacks = 0;
	unsigned long long mask = arrBishopMask[sq];
	unsigned long long board = _pdep_u64(i, mask);
	return (genSliding(sq, NE, board) | genSliding(sq, SE, board) | genSliding(sq, SW, board) | genSliding(sq, NW, board)) & ~getBit(sq);
};

unsigned long long genKnightAttack(int sq) {

	unsigned long long attack = 0;

	int y = sq / 8;
	int x = sq - y * 8;

	if (y >	1) {
		if (x > 0) {
			attack |= getBit((y-2)*8+(x-1));
		}
	}

	if (y > 1) {
		if (x < 7) {
			attack |= getBit((y - 2) * 8 + (x + 1));
		}
	}

	if (y > 0) {
		if (x > 1) {
			attack |= getBit((y - 1) * 8 + (x - 2));
		}
	}

	if (y > 0) {
		if (x < 6) {
			attack |= getBit((y - 1) * 8 + (x + 2));
		}
	}

	if (y < 6) {
		if (x > 0) {
			attack |= getBit((y + 2) * 8 + (x - 1));
		}
	}

	if (y < 6) {
		if (x < 7) {
			attack |= getBit((y + 2) * 8 + (x + 1));
		}
	}

	if (y < 7) {
		if (x > 1) {
			attack |= getBit((y + 1) * 8 + (x - 2));
		}
	}

	if (y < 7) {
		if (x < 6) {
			attack |= getBit((y + 1) * 8 + (x + 2));
		}
	}

	return attack;
}

unsigned long long genKingAttack(int sq) {
	unsigned long long attack = 0;

	int y = sq / 8;
	int x = sq - y * 8;

	if (y > 0) {
		if (x > 0) {
			attack |= getBit((y - 1) * 8 + (x - 1));
		}
		if (x < 7) {
			attack |= getBit((y - 1) * 8 + (x + 1));
		}
		attack |= getBit((y - 1) * 8 + (x));
	}
	if (y < 7) {
		if (x > 0) {
			attack |= getBit((y + 1) * 8 + (x - 1));
		}
		if (x < 7) {
			attack |= getBit((y + 1) * 8 + (x + 1));
		}
		attack |= getBit((y + 1) * 8 + (x));
	}
	if (x > 0) {
		attack |= getBit((y) * 8 + (x - 1));
	}
	if (x < 7) {
		attack |= getBit((y) * 8 + (x + 1));
	}
	return attack;
}

void genPawnAttacks(int sq) {
	arrWPawnAttacks[sq] = 0;
	arrBPawnAttacks[sq] = 0;
	int y = sq / 8;
	int x =  sq-y*8;
	if (y < 7) {
		if (x > 0) {
			arrWPawnAttacks[sq] |= getBit(y * 8 + x + 8 - 1);
		}
		if (x < 7) {
			arrWPawnAttacks[sq] |= getBit(y * 8 +x  + 8 + 1);
		}
	}
	if (y > 0) {
		if (x > 0) {
			arrBPawnAttacks[sq] |= getBit(y * 8 +x - 8 - 1);
		}
		if (x < 7) {
			arrBPawnAttacks[sq] |= getBit(y * 8 +x  - 8 + 1);
		}
	}
}

unsigned long long rookAttack(unsigned long long occ, int sq) {
	return arrRookAttacks[arrRookBase[sq] + _pext_u64(occ, arrRookMask[sq])];
}

unsigned long long bishopAttack(unsigned long long occ, int sq) {
	//std::cout << std::endl <<"SQUARE "<< sq<< "OCC" << std::endl << std::endl;
	//printBitBoard(arrBishopAttacks[arrBishopBase[sq] + _pext_u64(occ, arrBishopMask[sq])]);

	return arrBishopAttacks[arrBishopBase[sq] + _pext_u64(occ, arrBishopMask[sq])];
}

unsigned long long knightAttack(int sq) {
	return arrKnightAttacks[sq];
}

unsigned long long kingAttack(int sq) {
	return arrKingAttacks[sq];
};

unsigned long long wPawnAttack(int sq) {
	return arrWPawnAttacks[sq];
}
unsigned long long bPawnAttack(int sq) {
	return arrBPawnAttacks[sq];
}


void initArrays() {
	
	int rookOffset = 0;
	int bishopOffset = 0;
	
	for (int sq = 0; sq < 64; sq++) {
		arrRookMask[sq] = genRookMask(sq);
		arrBishopMask[sq] = genBishopMask(sq);
		arrKnightAttacks[sq] = genKnightAttack(sq);
		arrKingAttacks[sq] = genKingAttack(sq);
		genPawnAttacks(sq);
	}

	for (int sq = 0; sq < 64; sq++) {
		arrRookBase[sq] = rookOffset;
		for (int o = 0; o < pow(2, __popcnt64(arrRookMask[sq])); o++) {
			arrRookAttacks[rookOffset] = genRookAttack(sq, o);
			rookOffset++;
		}
	}

	for (int sq = 0; sq < 64; sq++) {
		arrBishopBase[sq] = bishopOffset;
		for (int o = 0; o < pow(2,__popcnt64(arrBishopMask[sq])); o++) {
			arrBishopAttacks[bishopOffset] = genBishopAttack(sq, o);
			bishopOffset++;
		}
	}
	//std::cout << "......" << rookOffset << "......";
	//std::cout << "......" << bishopOffset << "......";
}
