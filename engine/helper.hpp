#pragma once
#include "includes.hpp"
void SPLIT_STRING(string, vector<string>&);

int TO_SQUARE(int, int);
string TO_ALGEBRA(int);

#define PAWN 0
#define KNIGHT 2
#define BISHOP 4
#define ROOK 6
#define QUEEN 8
#define KING 10

#define WHITE false
#define BLACK true

#define QUEENSIDE true
#define KINGSIDE false

#define NORTH 0
#define NORTHEAST 1
#define EAST 2
#define SOUTHEAST 3
#define SOUTH 4
#define SOUTHWEST 5
#define WEST 6
#define NORTHWEST 7

#define MATE_SCORE 900000