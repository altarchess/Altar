#include "engine.h"
#include "bitboard.h"
#include "fen.h"

struct search mainSearch;
struct position mainPosition;

struct search* getSearchPointer() {
	return &mainSearch;
};

struct position* getPositionPointer () {
	return &mainPosition;
};

void stopSearch() {
	return;
}
void startSearch() {
	return;
}
void initSearch() {
	getSearchPointer()->searching = false;
	getSearchPointer()->init = true;
	getSearchPointer()->time = -1;
	getSearchPointer()->sTime = -1;
	getSearchPointer()->nodeCount = 0;
	getSearchPointer()->inc = 0;
	getSearchPointer()->movesToGo = 1;
	getSearchPointer()->depth = 0;
	getSearchPointer()->timetype = 0;

	getSearchPointer()->reacheddepth = 0;
	getSearchPointer()->bff = 0;
	getSearchPointer()->bft = 0;

	return;
}
void initPosition() {
	getPositionPointer()->hash = 0;
	getPositionPointer()->enPassant = 0;
	getPositionPointer()->castle[0] = false;
	getPositionPointer()->castle[1] = false;
	getPositionPointer()->castle[2] = false;
	getPositionPointer()->castle[3] = false;
	getPositionPointer()->side = WHITE;
	getPositionPointer()->mov50 = 0;
	for (int i = 0; i < 12; i++) {
		getPositionPointer()->bitBoard[i] = 0;
	}

}