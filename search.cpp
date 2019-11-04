#include "search.h"
#include "engine.h"
#include "bitboard.h"
#include "eval.h"
#include "movelist.h"
#include "move.h"
#include "tt.h"

#include <immintrin.h>
#include <intrin.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

int killers[100][2];
int ht[64][64];

int mvVal[13] = {
	0,12,6,4,3,1,1000,1000,1,3,4,6,12
};

void orderMvl(struct moveList* mvl, int ply, struct position* pos) {
	int scores[100];
	int hashmove = tt[pos->hash%ttSize].move;

	int ctr = 0;
	for (int i = 0; i < mvl->mam; i++) {
		if (ctr == mvl->gcapt) {
			ctr = 20;
		}
		scores[ctr] = ht[mvl->MOVE[ctr].f][mvl->MOVE[ctr].t];



		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == killers[ply][1]) {
			scores[ctr] = 500001;
		}

		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == killers[ply][0]) {
			scores[ctr] = 500002;
		}

		int tp = getPiece(pos, mvl->MOVE[ctr].t);
		if (tp != 0) {
			int ft = getPiece(pos, mvl->MOVE[ctr].f);
			if (mvVal[ft] <= mvVal[tp]) {
				scores[ctr] = 750000 + (mvVal[tp] * 8 - mvVal[ft]) * 1000;
			}
		}

		if (mvl->MOVE[ctr].f + 100 * mvl->MOVE[ctr].t == hashmove) {
			scores[ctr] = 1000003;
			//std::cout << "h" << hashmove;
		}
		ctr++;
	}
	int maxo = 5;
	if (mvl->mam <= maxo+1) {
		maxo = mvl->mam - 1;
	}
	if (maxo <= 3) {
		return;
	}
	ctr = 0;
	for (int i = 0; i < maxo; i++) {
		if (ctr == mvl->gcapt) {
			ctr = 20;
		}
		int bestid = 0;
		int bests = 0;
		int ctr2 = ctr;
		for (int e = 0; e < mvl->mam-i; e++) {
			if (ctr2 == mvl->gcapt) {
				ctr2 = 20;
			}

			if (bests <= scores[ctr2]) {
				bestid = ctr2;
				bests = scores[ctr2];
			}
			ctr2++;
		}

		int bf = mvl->MOVE[bestid].f;
		int bt = mvl->MOVE[bestid].t;
		int type = mvl->MOVE[bestid].type;


		mvl->MOVE[bestid] = mvl->MOVE[ctr];
		mvl->MOVE[ctr].f = bf;
		mvl->MOVE[ctr].t = bt;
		mvl->MOVE[ctr].type = type;
		scores[bestid] = scores[ctr];
		scores[ctr] = bests;

		ctr++;
	}
}

void printCord(int cord) {
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

void infoString(struct move MOVE, int depth, int score, int nodes) {

	std::cout << "info depth " << depth << " score cp " << score << " nodes " << nodes << " pv ";


	int f = MOVE.f;
	int t = MOVE.t;
	if (MOVE.type!=0) {
		switch (MOVE.type) {
		case 1:
			f = 60;
			t = 62;
			break;
		case 2:
			f = 60;
			t = 58;
			break;
		case 3:
			f = 4;
			t = 7;
			break;
		case 4:
			f = 4;
			t = 2;
			break;
		}
	}

	printCord(f);
	printCord(t);

	std::cout << std::endl;

	fflush(stdout);

	return;
}

int Qui(struct position pos, int alpha, int beta, int ply, struct QTable* ct) {
	if (ply >= 5) {
		return eval(&pos);
	}

	if (pos.side) {
		int staticEval = eval(&pos);
		if (staticEval >= alpha) {
			alpha = staticEval;
		}
		if (alpha >= beta) {
			return alpha;
		}
		genAllCaptures(&ct->QL[ply], pos.side, &pos);
		for (int i = 0; i < ct->QL[ply].mam; i++) {
			int score = Qui(makeMove(ct->QL[ply].MOVE[i], pos), alpha, beta, ply + 1, ct);
			if (score > staticEval) {
				staticEval = score;
				if (staticEval >= alpha) {
					alpha = staticEval;
				}
				if (alpha >= beta) {
					return alpha;
				}
			}
		}
		return staticEval;
	}
	else {
		int staticEval = eval(&pos);
		if (staticEval <= beta) {
			beta = staticEval;
		}
		if (alpha >= beta) {
			return beta;
		}
		genAllCaptures(&ct->QL[ply], pos.side, &pos);
		for (int i = 0; i < ct->QL[ply].mam; i++) {
			int score = Qui(makeMove(ct->QL[ply].MOVE[i], pos), alpha, beta, ply + 1, ct);
			if (score < staticEval) {
				staticEval = score;
				if (staticEval <= beta) {
					beta = staticEval;
				}
				if (alpha >= beta) {
					return beta;
				}
			}
		}
		return staticEval;
	}
}
int pvs(struct search* s, struct position pos, bool pvnode, int alpha, int beta, int depth, int ply, struct moveTable* mt, struct QTable* ct) {
	return 1;
}
void search(struct search* s, struct position* pos) {

}
int AlphaBeta(struct search* s, struct position pos, bool pvnode, int alpha, int beta, int depth, int ply, struct moveTable* mt, struct QTable* ct) {
	
	if (!__popcnt64(pos.bitBoard[6])) {
		return bMateScore + ply;
	}
	if (!__popcnt64(pos.bitBoard[5])) {
		return mateScore - ply;
	}

	if (depth == 0) {
		s->nodeCount++;
		return Qui(pos, alpha, beta, 0, ct);
		//return eval(&pos);
	}
	
	if (!s->searching) {
		return 0;
	}



	if (pos.side) {
		if (pos.hash == tt[pos.hash % ttSize].zHash) {
			if (tt[pos.hash % ttSize].depth>=depth) {
				if (tt[pos.hash % ttSize].type == 0) {
					return  tt[pos.hash % ttSize].eval;
				}
				if (tt[pos.hash % ttSize].type ==2 && tt[pos.hash % ttSize].eval < alpha) {
					return tt[pos.hash % ttSize].eval;
				}
				if (tt[pos.hash % ttSize].type == 1 && tt[pos.hash % ttSize].eval > beta) {
					return tt[pos.hash % ttSize].eval;
				}
			}
		}

		int bs = -1999999;
		int bm = 0;
		int type = 2;
		genAllMoves(&mt->mvl[ply], pos.side, &pos);
		if (depth > 0) {
			orderMvl(&mt->mvl[ply], ply, &pos);
		}
		
		int ctr = 0;
		for (int i = 0; i < mt->mvl[ply].mam; i++) {


			if (ctr == mt->mvl[ply].gcapt) {
				ctr = 20;
			}
			int score = AlphaBeta(s, makeMove(mt->mvl[ply].MOVE[ctr], pos), false, alpha, beta, depth - 1, ply + 1, mt, ct);

			if (score > bs) {
				bs = score;
				bm = mt->mvl[ply].MOVE[ctr].f + 100 * mt->mvl[ply].MOVE[ctr].t;
			}

			if (bs > alpha) {
				alpha = bs;
				type = 0;
				if (alpha >= beta) {
					ht[mt->mvl[ply].MOVE[ctr].f][mt->mvl[ply].MOVE[ctr].t] += depth * depth;
					killers[ply][1] = killers[ply][0];
					killers[ply][0] = bm;
					ttSave(depth, pos.hash, bs, 1, bm);
					return alpha;
				}
			}

			ctr++;
		}
		ttSave(depth, pos.hash, bs, type, bm);
		return bs;
	}
	else {

		if (pos.hash == tt[pos.hash % ttSize].zHash) {
			if (tt[pos.hash % ttSize].depth >= depth) {
				if (tt[pos.hash % ttSize].type == 0) {
					return  tt[pos.hash % ttSize].eval;
				}
				if (tt[pos.hash % ttSize].type == 2 && tt[pos.hash % ttSize].eval < alpha) {
					return tt[pos.hash % ttSize].eval;
				}
				if (tt[pos.hash % ttSize].type == 1 && tt[pos.hash % ttSize].eval > beta) {
					return tt[pos.hash % ttSize].eval;
				}
			}
		}
		int bs = 1999999;
		int bm = 0;
		int type = 1;
		genAllMoves(&mt->mvl[ply], pos.side, &pos);
		if (depth > 0) {
			orderMvl(&mt->mvl[ply], ply, &pos);
		}
		int ctr = 0;
		for (int i = 0; i < mt->mvl[ply].mam; i++) {


			if (ctr == mt->mvl[ply].gcapt) {
				ctr = 20;
			}

			int score = AlphaBeta(s, makeMove(mt->mvl[ply].MOVE[ctr], pos), false, alpha, beta, depth - 1, ply + 1, mt, ct);

			if (score < bs) {
				bs = score;
				bm = mt->mvl[ply].MOVE[ctr].f + 100 * mt->mvl[ply].MOVE[ctr].t;
			}

			if (bs < beta) {
				beta = bs;
				type = 0;
				if (alpha >= beta) {
					ht[mt->mvl[ply].MOVE[ctr].f][mt->mvl[ply].MOVE[ctr].t] += depth * depth;
					killers[ply][1] = killers[ply][0];
					killers[ply][0] = bm;
					ttSave(depth, pos.hash, bs, 2, bm);

					return beta;
				}
			}


			ctr++;
		}

		ttSave(depth, pos.hash, bs, type, bm);

		return bs;
	}


}

void mainSearch(struct search* s, struct position* pos) {
	for (int i = 0; i < 64; i++) {
		for (int e = 0; e < 64; e++) {
			ht[i][e] = 0;
		}
	}
	pos->hash = getHash(pos);
	//std::cout << pos->hash;
	struct moveTable tb;
	struct moveTable* mt = &tb;
	struct QTable qt;
	struct QTable* ct = &qt;
	genAllMoves(&mt->mvl[0],pos->side, pos);
	int alpha = -1999999;
	int beta = 1999999;
	int ply = 0;
	if (pos->side) {
		for (int depth = 2; depth < 30; depth++) {
			
			int bs = -1999999;
			struct move bm;

			int ctr = 0;
			for (int i = 0; i < mt->mvl[ply].mam; i++) {

				if (ctr == mt->mvl[ply].gcapt) {
					ctr = 20;
				}
				int score = AlphaBeta(s, makeMove(mt->mvl[ply].MOVE[ctr], *pos), false, bs, beta, depth - 1, ply + 1, mt, ct);
				
				if (!s->searching) {
					goto end;
				}

				if (score > bs) {
					bs = score;
					infoString(mt->mvl[ply].MOVE[ctr], depth, bs, s->nodeCount);
					bm = mt->mvl[ply].MOVE[ctr];
					ht[mt->mvl[ply].MOVE[ctr].f][mt->mvl[ply].MOVE[ctr].t] += depth*depth;
					killers[0][1] = killers[0][0];
					killers[0][0] = mt->mvl[ply].MOVE[ctr].f+ 100*mt->mvl[ply].MOVE[ctr].t;
					s->bff = bm.f;
					s->bft = bm.t;
				}


				ctr++;
			}
			ttSave(depth, pos->hash, bs, 0, bm.f+bm.t*100);
			s->bff = bm.f;
			s->bft = bm.t;
			s->reacheddepth = depth;
			orderMvl(&mt->mvl[0], 0, pos);
			if (s->depth <= s->reacheddepth) {
				goto end;
			};
		}
	}
	else {

		for (int depth = 2; depth < 30; depth++) {
			int bs = 1999999;
			struct move bm;
			int ctr = 0;
			for (int i = 0; i < mt->mvl[ply].mam; i++) {


				if (ctr == mt->mvl[ply].gcapt) {
					ctr = 20;
				}

				int score = AlphaBeta(s, makeMove(mt->mvl[ply].MOVE[ctr], *pos), false, alpha, bs, depth - 1, ply + 1, mt, ct);

				if (!s->searching) {
					goto end;
				}

				if (score < bs) {
					bs = score;
					infoString(mt->mvl[ply].MOVE[ctr], depth, -bs, s->nodeCount);
					bm = mt->mvl[ply].MOVE[ctr];
					ht[mt->mvl[ply].MOVE[ctr].f][mt->mvl[ply].MOVE[ctr].t] += depth * depth;
					killers[0][1] = killers[0][0];
					killers[0][0] = mt->mvl[ply].MOVE[ctr].f + 100 * mt->mvl[ply].MOVE[ctr].t;
					s->bff = bm.f;
					s->bft = bm.t;
				}


				ctr++;
			}
			ttSave(depth, pos->hash, bs, 0, bm.f + bm.t * 100);
			s->bff = bm.f;
			s->bft = bm.t;
			s->reacheddepth = depth;
			orderMvl(&mt->mvl[0], 0, pos);
			if (s->depth <= s->reacheddepth) {
				goto end;
			};
		}
	}
end:
	return;
};

void searchManager(struct search* s) {
	while (1) {
		if (s->searching) {
			unsigned long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

			unsigned long long inc = s->inc;
			unsigned long long sTime = s->sTime;
			unsigned long long UsableTime = s->time;
			unsigned long long moves = s->movesToGo;
			switch (s->timetype) {
			case 0:
				if (s->depth <= s->reacheddepth) {
					s->searching = false;
					printBestMove(s->bff, s->bft);
				}
				break;
			case 1:
				if (currentTime + 1 >= sTime + (UsableTime + inc * (moves - 1)) / moves) {
					s->searching = false;
					printBestMove(s->bff, getSearchPointer()->bft);
				}
				break;
			case 2:
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
