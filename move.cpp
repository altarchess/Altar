#include "move.h"
#include "bitboard.h"
#include "engine.h"
#include "move.h"
#include "tt.h"

#include <iostream>
#include <immintrin.h>
#include <intrin.h>

void printCords(int cord) {
	int y = cord / 8;
	int x = cord - y * 8;
	switch (x) {
	case 0:
		std::cout << "a";
		break;
	case 1:
		std::cout << "b";
		break;
	case 2:
		std::cout << "c";
		break;
	case 3:
		std::cout << "d";
		break;
	case 4:
		std::cout << "e";
		break;
	case 5:
		std::cout << "f";
		break;
	case 6:
		std::cout << "g";
		break;
	case 7:
		std::cout << "h";
		break;
	}
	switch (y) {
	case 0:
		std::cout << "8";
		break;
	case 1:
		std::cout << "7";
		break;
	case 2:
		std::cout << "6";
		break;
	case 3:
		std::cout << "5";
		break;
	case 4:
		std::cout << "4";
		break;
	case 5:
		std::cout << "3";
		break;
	case 6:
		std::cout << "2";
		break;
	case 7:
		std::cout << "1";
		break;
	}
}


int getPiece(struct position* pos, int cord) {
	for (int i = 0; i < 12; i++) {
		if (pos->bitBoard[i] & getBit(cord)) {
			return i + 1;
		}
	}
	return 0;
};


void printBestMove(int bff, int bft, struct position* pos) {
	std::cout << "bestmove ";
	printCords(bff);
	printCords(bft);
	if ((getPiece(pos, bff) == 5 && bft > 55)) {
		std::cout << "q";
	}
	if ((getPiece(pos, bff) == 8 && bft < 8)) {
		std::cout << "q";
	}
	std::cout << std::endl;
	fflush(stdout);
}

struct position makeMove(struct move MOVE, struct position calcPos) {

	unsigned long long type = MOVE.type;
	unsigned long long from = MOVE.f;
	unsigned long long to = MOVE.t;


	calcPos.hash ^= ttside;
	//std::cout << ttside;
	calcPos.side = !calcPos.side;



	unsigned long long fromPiece = getPiece(&calcPos, from);
	//std::cout << fromPiece;
	unsigned long long toPiece = getPiece(&calcPos, to);

	if (calcPos.enPassant>0) {
		calcPos.hash ^= ttrnde[_tzcnt_u64(calcPos.enPassant) - 48];
	}
	calcPos.enPassant = 0;


	unsigned long long fromY = from / 8;


	int toY = to / 8;


	/*std::cout << std::endl << "TYPE" << type;
	std::cout << std::endl << "FROM" << from;
	std::cout << std::endl << "TO" << to;

	std::cout << std::endl << "PIECEFROM" << calcPos.pieces[from];
	std::cout << std::endl << "PIECETO" << calcPos.pieces[to];*/

	if (type == 0) {
		calcPos.hash ^= ttrndp[fromPiece-1][from]; 
		calcPos.hash ^= ttrndp[fromPiece - 1][to];
		calcPos.bitBoard[fromPiece - 1] &= ~getBit(from);
		calcPos.bitBoard[fromPiece - 1] |= getBit(to);
		if (toPiece!=0) {
			//std::cout << "from:" << fromPiece << "to:" << to <<  "topiece:" << toPiece;
			calcPos.hash ^= ttrndp[toPiece - 1][to];
			calcPos.bitBoard[toPiece - 1] &= ~getBit(to);
			calcPos.mov50 = 0;
		}

		//castling rights
		if (from == 0) {
			if (calcPos.castle[3]) {
				calcPos.hash ^= ttrndc[3];
				calcPos.castle[3] = false;
			}
		}
		if (from == 7) {
			if (calcPos.castle[2]) {
				calcPos.hash ^= ttrndc[2];
				calcPos.castle[2] = false;
			}
		}
		if (from == 56) {
			if (calcPos.castle[1]) {
				calcPos.hash ^= ttrndc[1];
				calcPos.castle[1] = false;
			}
		}
		if (from == 63) {
			if (calcPos.castle[0]) {
				calcPos.hash ^= ttrndc[0];
				calcPos.castle[0] = false;
			}
		}
		if (fromPiece == 6) {
			if (calcPos.castle[2]) {
				calcPos.hash ^= ttrndc[2];
				calcPos.castle[2] = false;
			}
			if (calcPos.castle[3]) {
				calcPos.hash ^= ttrndc[3];
				calcPos.castle[3] = false;
			}
		}
		if (fromPiece == 7) {
			if (calcPos.castle[1]) {
				calcPos.hash ^= ttrndc[1];
				calcPos.castle[1] = false;
			}
			if (calcPos.castle[0]) {
				calcPos.hash ^= ttrndc[0];
				calcPos.castle[0] = false;
			}
		}

		//pawn move special rules;
		if (fromPiece == 5 || fromPiece == 8) {
			calcPos.mov50 = 0;
			if (fromPiece == 5 && to > 55) { // black promo (Always Queen)
				calcPos.bitBoard[fromPiece - 1] &= ~getBit(to);
				calcPos.hash ^= ttrndp[fromPiece-1][to];
				calcPos.bitBoard[0] |= getBit(to);
				calcPos.hash ^= ttrndp[0][to];
			}
			if (fromPiece == 8 && to < 8) { // white promo (Always Queen)
				calcPos.bitBoard[fromPiece - 1] &= ~getBit(to);
				calcPos.hash ^= ttrndp[fromPiece - 1][to];
				calcPos.bitBoard[12 - 1] |= getBit(to);
				calcPos.hash ^= ttrndp[11][to];

			}
			if ((to - from == 9 || to - from == 7 )&&toPiece==0) { // black en passant capture
				calcPos.bitBoard[8 - 1] &= ~getBit(to-8);
				calcPos.hash ^= ttrndp[7][to-8];

			}
			if ((to - from == -9 || to - from == -7) && toPiece == 0) { // white en passant capture
				calcPos.bitBoard[5 - 1] &= ~getBit(to + 8);
				calcPos.hash ^= ttrndp[4][to + 8];


			}
			if (fromY == 6 && toY == 4) {
				calcPos.enPassant |= getBit(63+from-fromY*8-7);
				calcPos.hash ^= ttrnde[_tzcnt_u64(calcPos.enPassant) - 48];
			}
			if (fromY == 1 && toY == 3) {
				calcPos.enPassant |= getBit(63 + from - fromY * 8 - 15);
				calcPos.hash ^= ttrnde[_tzcnt_u64(calcPos.enPassant) - 48];
			}
		}
	}
	else {
		//std::cout << "TYPE:" << type;
		switch (type) {
			calcPos.mov50 = 0;
		case 1:
			calcPos.castle[0] == false;
			calcPos.castle[1] == false;
			calcPos.hash ^= ttrndc[0];
			calcPos.hash ^= ttrndc[1];

			calcPos.bitBoard[6] |= getBit(62);
			calcPos.bitBoard[10] |= getBit(61);
			calcPos.bitBoard[6] &= ~getBit(60);
			calcPos.bitBoard[10] &= ~getBit(63);


			calcPos.hash ^= ttrndp[6][62];
			calcPos.hash ^= ttrndp[10][61];
			calcPos.hash ^= ttrndp[6][60];
			calcPos.hash ^= ttrndp[10][63];

			break;
		case 2:
			calcPos.castle[0] == false;
			calcPos.castle[1] == false;
			calcPos.hash ^= ttrndc[0];
			calcPos.hash ^= ttrndc[1];

			calcPos.bitBoard[6] |= getBit(58);
			calcPos.bitBoard[10] |= getBit(59);
			calcPos.bitBoard[6] &= ~getBit(60);
			calcPos.bitBoard[10] &= ~getBit(56);
			calcPos.hash ^= ttrndp[6][58];
			calcPos.hash ^= ttrndp[10][59];
			calcPos.hash ^= ttrndp[6][60];
			calcPos.hash ^= ttrndp[10][56];
			break;
		case 3:
			calcPos.castle[2] == false;
			calcPos.castle[3] == false;
			calcPos.hash ^= ttrndc[2];
			calcPos.hash ^= ttrndc[3];

			calcPos.bitBoard[5] |= getBit(6);
			calcPos.bitBoard[1] |= getBit(5);
			calcPos.bitBoard[5] &= ~getBit(4);
			calcPos.bitBoard[1] &= ~getBit(7);
			calcPos.hash ^= ttrndp[5][6];
			calcPos.hash ^= ttrndp[1][5];
			calcPos.hash ^= ttrndp[5][4];
			calcPos.hash ^= ttrndp[1][7];
			break;
		case 4:
			calcPos.castle[2] == false;
			calcPos.castle[3] == false;
			calcPos.hash ^= ttrndc[2];
			calcPos.hash ^= ttrndc[3];

			calcPos.bitBoard[5] |= getBit(2);
			calcPos.bitBoard[1] |= getBit(3);
			calcPos.bitBoard[5] &= ~getBit(4);
			calcPos.bitBoard[1] &= ~getBit(0);
			calcPos.hash ^= ttrndp[5][2];
			calcPos.hash ^= ttrndp[1][3];
			calcPos.hash ^= ttrndp[5][4];
			calcPos.hash ^= ttrndp[1][0];
			break;
		}
	}
	calcPos.mov50 += 1;
	return calcPos;
}

void makeNull(struct position* pos) {
	pos->side = !pos->side;
	pos->hash ^= ttside;
}

struct move bufToMove(char* buf) {
	
	int from = getCord(buf[0]) + getCord(buf[1]);
	*buf++; *buf++;
	int to = getCord(buf[0]) + getCord(buf[1]);

	struct position* mainPosition = getPositionPointer();

	struct move MOVE;
	MOVE.type = 0;

	if (mainPosition->bitBoard[5]&getBit(from) && from == 4 && to == 6) { // black kingside castle
		
		MOVE.type = 3;
		MOVE.f = 4;
		MOVE.t = 6;
		return MOVE;
	}

	if (mainPosition->bitBoard[5]&getBit(from) && from == 4 && to == 2) { // black queenside castle
		MOVE.type = 4;
		MOVE.f = 4;
		MOVE.t = 2;
		return MOVE;
	}
	
	if (mainPosition->bitBoard[6]&getBit(from) && from == 60 && to == 62) { // white kingside castle
		MOVE.type = 1;
		MOVE.f = 60;
		MOVE.t = 62;
		return MOVE;
	}

	if (mainPosition->bitBoard[6]&getBit(from) && from == 60 && to == 58) { // white queenside castle
		MOVE.type = 2;
		MOVE.f = 60;
		MOVE.t = 58;
		return MOVE;
	}


	MOVE.f = from;
	MOVE.t = to;

	//printBitBoard(move);

	return MOVE;

}

char* moveToChar(int move) {
	char* x;
	char z = 'x';
	x = &z;
	return x;
}
