#pragma once
void initArrays();
unsigned long long genRookAttack(int sq, unsigned long long i);
unsigned long long genBishopAttack(int sq, unsigned long long i);
unsigned long long rookAttack(unsigned long long occ, int sq);
unsigned long long bishopAttack(unsigned long long occ, int sq);
unsigned long long knightAttack(int sq);
unsigned long long kingAttack(int sq);
unsigned long long wPawnAttack(int sq);
unsigned long long bPawnAttack(int sq);
extern unsigned long long arrBishopPinMap[64];
extern unsigned long long arrRookPinMap[64];