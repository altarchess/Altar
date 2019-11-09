#include "uci.h"
#include "engine.h"
#include "bitboard.h"
#include "fen.h"
#include "gen.h"
#include "move.h"
#include "movelist.h"
#include "test.h"
#include "search.h"
#include "tt.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <immintrin.h>
#include <intrin.h>
#include <thread>

#define uci_Command            "uci"
#define go_Command			   "go"
#define	quit_Command		   "quit"
#define stop_Command		   "stop"
#define ready_Command		   "isready"
#define newgame_Command        "ucinewgame"
#define position_Command       "position fen"
#define startpos_Command       "position startpos"

#define CMD_PERFT              "perft"
#define CMD_TEST               "test"

#define READ_BUFFER_SIZE       65536

#define Version				   "191026"
#define Author                 "Kim Kahre, Markus Kahre"

#define test_Command		   "test"


struct historyhash hh;

int starts_With(char* buf, const char* str)
{
	return !strncmp(buf, str, strlen(str));
}

int compareCommand (char** buf, const char* cmd_str)
{
	if (starts_With(*buf, cmd_str))
	{
		*buf += strlen(cmd_str) + 1;
		return 1;
	}
	return 0;
}
void parseGO(char* buf) {

	getSearchPointer()->movesToGo = 30;
	getSearchPointer()->depth = 30;
	getSearchPointer()->timetype = 0;
	getSearchPointer()->inc = 0;
	getSearchPointer()->time = 0;
	
	getSearchPointer()->reacheddepth = 0;
	getSearchPointer()->bff = 0;
	getSearchPointer()->bft = 0;
	getSearchPointer()->nodeCount = 0;


	char* t;


	for (t = strtok(buf, " "); t; t = strtok(NULL, " "))
	{
		if ((!strcmp(t, "wtime") && getPositionPointer()->side == WHITE) ||
			(!strcmp(t, "btime") && getPositionPointer()->side == BLACK))
		{
			t = strtok(NULL, " ");
			getSearchPointer()->time = atoi(t);
			getSearchPointer()->timetype = 1;
		}
		if ((!strcmp(t, "winc") && getPositionPointer()->side == WHITE) ||
			(!strcmp(t, "binc") && getPositionPointer()->side == BLACK))
		{
			t = strtok(NULL, " ");
			getSearchPointer()->inc = atoi(t);
			getSearchPointer()->timetype = 1;
		}
		else if (!strcmp(t, "movestogo"))
		{
			t = strtok(NULL, " ");
			getSearchPointer()->movesToGo = atoi(t);
		}
		else if (!strcmp(t, "depth"))
		{
			t = strtok(NULL, " ");
			getSearchPointer()->depth = atoi(t);
			getSearchPointer()->timetype = 0;
		}
		/*else if (starts_With(buf, "ponder"))
		{
			//getSearchPointer()->ponder = true;
		}*/
		else if (starts_With(buf, "infinite"))
		{
			getSearchPointer()->timetype = 2;
		}
		else if (!strcmp(t, "movetime"))
		{
			t = strtok(NULL, " ");
			getSearchPointer()->time = atoi(t);
			getSearchPointer()->movesToGo = 1;
			getSearchPointer()->timetype = 1;
		}
	}


};
void parseFEN(char* buf) {

	struct position* mainPosition = getPositionPointer();
	struct search* mainSearch = getSearchPointer();
	initPosition();
	int x = 0;
	int y = 0;
	int stage = 0;
	int epx = 0;
	int mov50 = 0;

	for (int i = 0; i < strlen(buf); i++)
	{
		switch (stage) {
		case 0:
			switch (buf[i]) {
			case '/':
				y++;
				x = 0;
				break;
			case '8':
				x++;
			case '7':
				x++;
			case '6':
				x++;
			case '5':
				x++;
			case '4':
				x++;
			case '3':
				x++;
			case '2':
				x++;
			case '1':
				x++;
				break;
			case 'q':
				mainPosition->bitBoard[0] |= getBit(x + y * 8);
				x++;
				break;
			case 'r':
				mainPosition->bitBoard[1] |= getBit(x + y * 8);
				x++;
				break;
			case 'b':
				mainPosition->bitBoard[2] |= getBit(x + y * 8);
				x++;
				break;
			case 'n':
				mainPosition->bitBoard[3] |= getBit(x + y * 8);
				x++;
				break;
			case 'p':
				mainPosition->bitBoard[4] |= getBit(x + y * 8);
				x++;
				break;
			case 'k':
				mainPosition->bitBoard[5] |= getBit(x + y * 8);
				x++;
				break;
			case 'K':
				mainPosition->bitBoard[6] |= getBit(x + y * 8);
				x++;
				break;
			case 'P':
				mainPosition->bitBoard[7] |= getBit(x + y * 8);
				x++;
				break;
			case 'N':
				mainPosition->bitBoard[8] |= getBit(x + y * 8);
				x++;
				break;
			case 'B':
				mainPosition->bitBoard[9] |= getBit(x + y * 8);
				x++;
				break;
			case 'R':
				mainPosition->bitBoard[10] |= getBit(x + y * 8);
				x++;
				break;
			case 'Q':
				mainPosition->bitBoard[11] |= getBit(x + y * 8);
				x++;
				break;
			case ' ':
				stage = 1;
				break;
			}
			break;
		case 1:
			switch (buf[i]) {
			case 'w':
				mainPosition->side = WHITE;
				break;
			case  'b':
				mainPosition->side = BLACK;
				break;
			case ' ':
				stage = 2;
				break;
			}
			break;
		case 2:			
			switch (buf[i]) {
			case 'K':
				mainPosition->castle[0] = true;
				break;
			case  'Q':
				mainPosition->castle[1] = true;
				break;
			case 'k':
				mainPosition->castle[2] = true;
				break;
			case 'q':
				mainPosition->castle[3] = true;
				break;
			case ' ':
				stage = 3;
				break;
			}
			break;
		case 3:
			switch (buf[i]) {
			case 'a':
				epx = 0;
				break;
			case 'b':
				epx = 1;
				break;
			case 'c':
				epx = 2;
				break;
			case 'd':
				epx = 3;
				break;
			case 'e':
				epx = 4;
				break;
			case 'f':
				epx = 5;
				break;
			case 'g':
				epx = 6;
				break;
			case 'h':
				epx = 7;
				break;
			case '6':
				mainPosition->enPassant |= getBit(63 + epx - 15);
				break;
			case '3':
				mainPosition->enPassant |= getBit(63 + epx - 7);
				break;
			case ' ':
				stage = 4;
				break;
			}
			break;
		case 4:
			if (buf[i] == ' ') {
				stage = 5;
			}
			else {
				mov50 = mov50 * 10;
				mov50 += (int)buf[i];
			}
			break;
		case 5:
			if (buf[i] == ' ') {
				stage = 6;
			}
			break;
		}
	}

	hh.hh[0] = getHash(getPositionPointer());
	hh.index = 1;

	buf = strstr(buf, "moves");
	if (buf == NULL) return;

	while ((buf = strchr(buf, ' ')))
	{
		while (*buf == ' ') buf++;
		*getPositionPointer() = makeMove(bufToMove(buf), *getPositionPointer());
		hh.hh[hh.index] = getPositionPointer()->hash;
		hh.index += 1;
	}
	return;
}

void startPos(char* buf) {
	char initialFEN[] = starting_Position;
	parseFEN(initialFEN);


	buf = strstr(buf, "moves");
	if (buf == NULL) return;

	while ((buf = strchr(buf, ' ')))
	{
		while (*buf == ' ') buf++;
		struct move M = bufToMove(buf);
		*getPositionPointer() = makeMove(M, *getPositionPointer());
		hh.hh[hh.index] = getPositionPointer()->hash;
		hh.index += 1;
	}
}

void uci() {
	setTTSize(10000000);
	char* buf, input_buf[READ_BUFFER_SIZE];
	fflush(stdout);
	printf("Altar %s by Kim Kahre\n", Version);	fflush(stdout);

	std::thread searchManager(searchManager, getSearchPointer());
	searchManager.detach();
	while (1) {
		fflush(stdout);
		if (fgets(input_buf, READ_BUFFER_SIZE, stdin) == NULL)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));	
			continue;
		}


		buf = input_buf;

		
		if (compareCommand(&buf, uci_Command))
		{
			printf("id name Altar %s\n", Version);
			printf("id Author %s", Author);
			printf("\n");
			printf("uciok\n");
			fflush(stdout);
			/*std::cout << "id name Altar " << Version << "\n";
			std::cout << "id author " << Author << "\n";
			std::cout << "\n";
			std::cout << "uciok\n";*/
		}

		if (compareCommand(&buf, ready_Command))
		{
			printf("readyok\n");
			fflush(stdout);
		}

		if (compareCommand(&buf, quit_Command))
		{
			getSearchPointer()->searching = false;
			break;
		}

		if (compareCommand(&buf, CMD_PERFT))
		{
			struct moveTable tb;

			std::cout << perft(*getPositionPointer(),&tb, 5);
			printBoard();
		}

		if (compareCommand(&buf, go_Command))
		{
			if (getSearchPointer()->searching) {
				getSearchPointer()->searching = false;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}			
			getSearchPointer()->sTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

			getSearchPointer()->searching = true;

			parseGO(buf);

			std::thread mSearch(mainSearch, getSearchPointer(), getPositionPointer(), hh);
			mSearch.detach();
		}

		if (compareCommand(&buf, stop_Command))
		{
			if (getSearchPointer()->searching) {
				printBestMove(getSearchPointer()->bff, getSearchPointer()->bft, getPositionPointer());
			}
			getSearchPointer()->searching = false;
			//na
		}

		if (compareCommand(&buf, position_Command))
		{
			parseFEN(buf);
			fflush(stdout);

		}

		if (compareCommand(&buf, startpos_Command))
		{
			startPos(buf);
			fflush(stdout);
		}
		
		
		if (compareCommand(&buf, test_Command))
		{
			//na
			printBitBoard(rookAttack(getPositionPointer()->bitBoard[4] ,28)| bishopAttack(getPositionPointer()->bitBoard[4], 28)); // queen attackset test
		}


	};
	free(tt);
};
