#pragma once
#include "includes.hpp"
typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;
void SPLIT_STRING(string, vector<string>&);
uint64_t RAND_64();

int TO_SQUARE(int, int);
string TO_ALGEBRA(int);
int FENIDX_TO_SQUARE(int idx);

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

#define WHITE_KINGSIDE 0
#define WHITE_QUEENSIDE 1
#define BLACK_KINGSIDE 2
#define BLACK_QUEENSIDE 3

#define NORTH 0
#define NORTHEAST 1
#define EAST 2
#define SOUTHEAST 3
#define SOUTH 4
#define SOUTHWEST 5
#define WEST 6
#define NORTHWEST 7

#define MATE_SCORE 4e7
#define INFINITE_SCORE 9e7

#define TT_EXACT 0
#define TT_LOWER 1
#define TT_UPPER 2