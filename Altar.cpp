// Altar.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "uci.h"
#include "engine.h"
#include "bitboard.h"
#include "gen.h"

int main()
{
	initBits();
	initPosition();
	initSearch();
	initArrays();
	uci();
}

