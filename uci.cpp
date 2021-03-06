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
#include "eval.h"
#include "tune.h"

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
#define staticEval_Command	   "staticeval"
#define tune_Command	   "autotune"
#define tune_kCommand	   "ktuner"
#define option_Command "setoption name"

#define CLOPValue_Command "setvalue"

#define CMD_PERFT              "perft"
#define CMD_TEST               "test"

#define READ_BUFFER_SIZE       65536

#define Version				   "200221 x64"
#define Author                 "Kim Kahre"

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

	getSearchPointer()->movesToGo = 50;
	getSearchPointer()->depth = 50;
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

void parseTest(char* buf) {
		std::cout << "OKOKKOKOK";
		while (*buf == ' ') buf++;
		struct move M = bufToMove(buf);
		int seeScore = see(getPositionPointer(), M.f, M.t);
		std::cout << "seeSCORE:" << seeScore;
		fflush(stdout);
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
	setTTSize();
	/*

0 == PAWNENDGAMESPACE
1 == PAWNMIDDLEGAMESPACE

2 == ISOLANIEFFECT
3 == DOUBLEDEFFECT
4 == BISHOP MIDDLEGAME MOBILITY
5 == BISHOP ENDGAME MOBILITY
6 == BISHOP ENDGAME MOBILITY
7 == KNIGHT MOBILITY MIDDLEGAME
8 == KNIGHT PSQT ENDGAME
9 == ROOK MIDDLEGAME MOB
10 == ROOK ENDGAME MOB
9 == ROOK MIDDLEGAME MOB
10 == ROOK ENDGAME MOB
11 == QUEEN MIDDLEGAME MOB
12 == QUEEN ENDGAME MOB
13 == BISHOP PAIR BONUS
14 == KNIGHT PAIR BONUS
15 == king defenders count
16 == kingShield count
17 == kingPSQTMIDDLEGAME
18 == kingPSQTENDGAME
19 == atcounter
20 == pawnMiddle
21 == knightMiddle
22 == bishopMiddle
23 == rookMiddle
24 == queenMiddle
25 == pawnEnd
26 == knightEnd
27 == bishopEnd
28 == rookEnd
29 == queenEnd
*/
	/*getTuneVector()->MODIF[0] = 0;// PAWNENDGAMESPACE
	getTuneVector()->MODIF[1] = 0;//PAWNMIDDLEGAMESPACE
	getTuneVector()->MODIF[2] = 0;//ISOLANIEFFECT
	getTuneVector()->MODIF[3] = 0;//DOUBLEDEFFECT
	getTuneVector()->MODIF[4] = 0;//BISHOP MIDDLEGAME MOBILITY
	getTuneVector()->MODIF[5] = 0;//BISHOP ENDGAME MOBILITY
	getTuneVector()->MODIF[6] = 0;//BISHOP ENDGAME MOBILITY
	getTuneVector()->MODIF[7] = 0;//KNIGHT MOBILITY MIDDLEGAME
	getTuneVector()->MODIF[8] = 0;//KNIGHT PSQT ENDGAME
	getTuneVector()->MODIF[9] = 0;// ROOK MIDDLEGAME MOB
	getTuneVector()->MODIF[10] = 0;//ROOK ENDGAME MOB
	getTuneVector()->MODIF[11] = 0;//QUEEN MIDDLEGAME MOB
	getTuneVector()->MODIF[12] = 0;//QUEEN ENDGAME MOB
	getTuneVector()->MODIF[13] = 0;//BISHOP PAIR BONUS
	getTuneVector()->MODIF[14] = 0; //KNIGHT PAIR BONUS
	getTuneVector()->MODIF[15] = 0;// king defenders count
	getTuneVector()->MODIF[16] = 0;//kingShield count
	getTuneVector()->MODIF[17] = 0;//kingPSQTMIDDLEGAME
	getTuneVector()->MODIF[18] = 0;// kingPSQTENDGAME
	getTuneVector()->MODIF[19] = 0; //atcounter
	getTuneVector()->MODIF[20] = pawnMiddleGame;
	getTuneVector()->MODIF[21] = knightMiddleGame;
	getTuneVector()->MODIF[22] = bishopMiddleGame;
	getTuneVector()->MODIF[23] = rookMiddleGame;
	getTuneVector()->MODIF[24] = queenMiddleGame;
	getTuneVector()->MODIF[25] = pawnEndGame;
	getTuneVector()->MODIF[26] = knightEndGame;
	getTuneVector()->MODIF[27] = bishopEndGame;
	getTuneVector()->MODIF[28] = rookEndGame;
	getTuneVector()->MODIF[29] = queenEndGame;
	getTuneVector()->active = 30;*/
	getTuneVector()->MODIF[0] = 134;// PAWNENDGAMESPACE
	getTuneVector()->MODIF[1] = 42;//PAWNMIDDLEGAMESPACE
	getTuneVector()->MODIF[2] = 118;//ISOLANIEFFECT
	getTuneVector()->MODIF[3] = 80;//DOUBLEDEFFECT
	getTuneVector()->MODIF[4] = 38;//BISHOP MIDDLEGAME MOBILITY
	getTuneVector()->MODIF[5] = 64;//BISHOP ENDGAME MOBILITY
	getTuneVector()->MODIF[6] = 114;//BISHOP ENDGAME MOBILITY
	getTuneVector()->MODIF[7] = 15;//KNIGHT MOBILITY MIDDLEGAME
	getTuneVector()->MODIF[8] = 20;//KNIGHT PSQT ENDGAME
	getTuneVector()->MODIF[9] = 52;// ROOK MIDDLEGAME MOB
	getTuneVector()->MODIF[10] = 45;//ROOK ENDGAME MOB
	getTuneVector()->MODIF[11] = 2;//QUEEN MIDDLEGAME MOB
	getTuneVector()->MODIF[12] = 134;//QUEEN ENDGAME MOB
	getTuneVector()->MODIF[13] = 408;//BISHOP PAIR BONUS
	getTuneVector()->MODIF[14] = -31; //KNIGHT PAIR BONUS
	getTuneVector()->MODIF[15] = 82;// king defenders count
	getTuneVector()->MODIF[16] = 57;//kingShield count
	getTuneVector()->MODIF[17] = 8;//kingPSQTMIDDLEGAME
	getTuneVector()->MODIF[18] = 143;// kingPSQTENDGAME
	getTuneVector()->MODIF[19] = 193; //atcounter
	getTuneVector()->MODIF[20] = pawnMiddleGame;
	getTuneVector()->MODIF[21] = knightMiddleGame;
	getTuneVector()->MODIF[22] = bishopMiddleGame;
	getTuneVector()->MODIF[23] = rookMiddleGame;
	getTuneVector()->MODIF[24] = queenMiddleGame;
	getTuneVector()->MODIF[25] = pawnEndGame;
	getTuneVector()->MODIF[26] = knightEndGame;
	getTuneVector()->MODIF[27] = bishopEndGame;
	getTuneVector()->MODIF[28] = rookEndGame;
	getTuneVector()->MODIF[29] = queenEndGame;
	getTuneVector()->MODIF[30] = 28; //weakPenalty
	getTuneVector()->MODIF[31] = 33; //openfileweakPenalty
	getTuneVector()->MODIF[32] = 56; //openfileweakPenaltyRook
	getTuneVector()->MODIF[33] = 89; //protectedPawn
	getTuneVector()->MODIF[34] = 71; //bishopEnemyTerritory
	getTuneVector()->MODIF[35] = 18; //QueenEnemyTerritory
	getTuneVector()->MODIF[36] = 17; //bishopOwnTerritory
	getTuneVector()->MODIF[37] = 240; //PassedPawn
	getTuneVector()->MODIF[38] = 59; //BishopPawnColor

	//new Rook terms
	getTuneVector()->MODIF[39] = 111; //rookOpenFileMiddleGame
	getTuneVector()->MODIF[40] = 173; //rookOnlyOwnPawnsMiddleGame
	getTuneVector()->MODIF[41] = 186; //rookOnlyEnemyPawnsMiddleGame
	getTuneVector()->MODIF[42] = -221; //rookOpenFileEndGame
	getTuneVector()->MODIF[43] = 80; //rookOnlyOwnPawnsEndGame
	getTuneVector()->MODIF[44] = 264; //rookOnlyEnemyPawnsEndGame
	getTuneVector()->MODIF[45] = 589; //RookOposingKingBonus (applied to 39)
	getTuneVector()->MODIF[46] = 124; //RookOposingKingBonusWithEnemyPawns (applied to 40)

	//phase tune
	getTuneVector()->MODIF[47] = 13; //knightPhase
	getTuneVector()->MODIF[48] = 15; //bishopPhase
	getTuneVector()->MODIF[49] = 25; //rookPhase
	getTuneVector()->MODIF[50] = 62; //QueenPhase
	getTuneVector()->MODIF[51] = 284; //totalPhase

	//pins
	getTuneVector()->MODIF[52] = 108; //pawnBishopPinPenalty
	getTuneVector()->MODIF[53] = -14; //pawnRookPinPenalty
	getTuneVector()->MODIF[54] = 378; //knightBishopPinPenalty
	getTuneVector()->MODIF[55] = 257; //knightRookPinPenalty
	getTuneVector()->MODIF[56] = 262; //bishopRookPinPenalty
	getTuneVector()->MODIF[57] = 1936; //rookBishopPinPenalty
	getTuneVector()->MODIF[58] = 1675; //QueenRookPinPenalty
	getTuneVector()->MODIF[59] = 702; //QueenBishopPinPenalty

	getTuneVector()->MODIF[60] = 383; //small piece attacks biggerp;
	getTuneVector()->MODIF[61] = 98; //small piece attacks biggerpeg;

	//Piece to enemy king tropism
	getTuneVector()->MODIF[62] = 10; //Knight manhattan distance
	getTuneVector()->MODIF[63] = 30; //Queen manhattan distance
	getTuneVector()->MODIF[64] = -8; //Bishop manhattan distance
	getTuneVector()->MODIF[65] = 2; //Rook manhattan distance
	getTuneVector()->MODIF[66] = 4; //pawn manhattan Distance
	getTuneVector()->MODIF[67] = -20; //pawn manhattan Distance endGame passer
	getTuneVector()->MODIF[68] = 3; //weak Pawn
	getTuneVector()->MODIF[69] = -27; //pawn manhattan Distance endGame any pawn
	/*//soft pins ??

*/
	getTuneVector()->MODIF[70] = -122; //Attacked hanging piece
	getTuneVector()->MODIF[71] = -9; //attacked hanging pawns
	
	getTuneVector()->MODIF[72] = 9; //OutPostSq
	getTuneVector()->MODIF[73] = 205; //Knight on OutPost
	getTuneVector()->MODIF[74] = 246; //Bishop on OutPost

	getTuneVector()->MODIF[75] = 92; //Opposite color bishop endings
	getTuneVector()->MODIF[76] = 91; //Opposite color bishop middleGames
	getTuneVector()->MODIF[77] = 74; //Opposite color bishop middleGames

	getTuneVector()->MODIF[78] = 306; //side to move bonus


	getTuneVector()->active = 79;

	int params[79] = { 137, 46, 85, 59, 40, 62, 124, 16, 24, 61, 41, 12, 144, 435, -43, 57, 52, 9, 159, 169, 877, 3626, 3137, 4427, 10581, 868, 3874, 3740, 6507, 10970, 32, 28, 54, 120, 84, 14, 9, 235, 56, 118, 168, 201, -237, 90, 269, 656, 140, 12, 14, 24, 62, 279, 96, -36, 398, 242, 251, 1955, 1620, 587, 582, 249, 12, 27, -9, 7, 1, -24, 6, -28, -187, -52, 16, 239, 269, 96, 93, 81, 358};

	for (int i = 0; i < getTuneVector()->active; i++)getTuneVector()->MODIF[i] = params[i];
	
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
			printf("id author %s", Author);
			printf("\n");
			printf("\n");
			printf("option name Hash type spin default 160 min 10 max 131072");
			printf("\n");
			printf("option name Threads type spin default 1 min 1 max 8");
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

		if (compareCommand(&buf, newgame_Command))
		{
			resetHistory();
			setTTSize();
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

			std::thread mSearch(launchThreads, getSearchPointer(), getPositionPointer(), hh);
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
			testf(25);
			//parseTest(buf);
		}		
		
		if (compareCommand(&buf, staticEval_Command))
		{
			//na
			showStatic();
		}

		if (compareCommand(&buf, tune_Command))
		{
			//na
			tuneVal(getTuneVector());
		}
		if (compareCommand(&buf, tune_kCommand))
		{
			//na
			tuneK();
		}
		if (compareCommand(&buf, option_Command))
		{
			if (compareCommand(&buf, "Hash value"))
			{
				ttSetting = atoi(buf) * 1000000 / sizeof(ttEntryCompressed);
				setTTSize();
			}
			if (compareCommand(&buf, "Threads value"))
			{
				getSearchPointer()->threadCount = atoi(buf);
			}
		}
		if (compareCommand(&buf, CLOPValue_Command))
		{
			if (compareCommand(&buf, "futilityMargin"))
			{
				FUTILITY_MARGIN = atoi(buf);
				continue;
			}
			if (compareCommand(&buf, "razorMargin"))
			{
				RAZOR_MARGIN = atoi(buf);
				continue;
			}
			if (compareCommand(&buf, "reductionDiv"))
			{
				reductionDiv = atoi(buf);
				continue;
			}
			if (compareCommand(&buf, "historyDiv"))
			{
				HISTORYDIV = atoi(buf);
				continue;
			}
			if (compareCommand(&buf, "futilityDepth"))
			{
				FUTILITY_DEPTH = atoi(buf);
				continue;
			}
		}

	};
	free(tt);
};
