#include "test.h"
#include "movelist.h"
#include "engine.h"
#include "move.h"
#include "tt.h"
#include "bitboard.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>




unsigned long long perft(struct position pos, struct moveTable* mt, int depth) {


	if (depth == 0) {
		//printBoard();
		return 1;
	}


	genAllMoves(&mt->mvl[depth], pos.side, &pos);
	unsigned long long counter = 0;
	int ctr = 0;
	struct ttEntry ttEnt = ttProbe(pos.hash);
	if (pos.hash == ttEnt.zHash) {
		if (ttEnt.depth == depth) {
			if (ttEnt.type == 0) {
				return ttEnt.eval;
			}
		}
	}

	for (int i = 0; i < mt->mvl[depth].mam; i++) {

		if (ctr == mt->mvl[depth].gcapt) {

			ctr = 20;
		}

		struct position pos2 = makeMove(mt->mvl[depth].MOVE[ctr], pos);
		if (isLegal(pos.side, &pos2)) {
			/*if (depth == 7) {
				std::cout << std::endl << "f" << mt->mvl[depth].MOVE[ctr].f << "t" << mt->mvl[depth].MOVE[ctr].t;
				std::cout << std::endl << "ctr : " << perft(pos2, mt, depth - 1);
			}*/
			counter += perft(pos2, mt, depth - 1);
		}


		ctr++;

	}
	ttSave(depth, pos.hash, counter, 0, 0, true);
	return counter;
}