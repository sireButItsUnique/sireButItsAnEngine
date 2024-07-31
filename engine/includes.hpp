#pragma once
#include <algorithm>
#include <chrono>
#include <intrin.h>
#include <iomanip>
#include <iostream>
#include <locale.h>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <utility>
#include <vector>
using namespace std;

#define pawnWhite 0
#define pawnBlack 1
#define knightWhite 2
#define knightBlack 3
#define bishopWhite 4
#define bishopBlack 5
#define rookWhite 6
#define rookBlack 7
#define queenWhite 8
#define queenBlack 9
#define kingWhite 10
#define kingBlack 11
#define NORTH 0
#define NORTHEAST 1
#define EAST 2
#define SOUTHEAST 3
#define SOUTH 4
#define SOUTHWEST 5
#define WEST 6
#define NORTHWEST 7
#define ll long long
#define white false
#define black true
#define DOUBLE_INF numeric_limits<double>::infinity
#define KING_DEAD 1e10

#define TO_SQUARE(col, row) 8 * (row - '1') + 7 - (col - 'a')
#define TO_ALGEBRA(sq) string(1, 'a' + (7 - (sq % 8))) + string(1, '1' + (sq / 8))
#define BOLD "\e[1m"
#define UNBOLD "\e[0m"
#define GREEN "\x1B[32m"
#define UNCOLOR "\x1B[0m"