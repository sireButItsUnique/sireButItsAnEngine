#include "MoveGen.hpp"
using namespace MoveGen;

namespace MoveGen {
	uint64_t rayAttacks[64][8];
	uint64_t rookRays[64];
	uint64_t bishopRays[64];

	uint64_t rookLookupOffsets[64];
	uint64_t bishopLookupOffsets[64];

	uint64_t rookLookup[102400];
	uint64_t bishopLookup[5248];
	uint64_t knightLookup[64];
	uint64_t kingLookup[64];
}

/***
Sliding Attack Generation Functions (note: probably backwards because of endian, but it works bc symmetry)
***/
uint64_t southAttacks(uint64_t rooks, uint64_t blockers) {
	uint64_t flood = rooks;
	uint64_t empty = ~blockers;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= rooks = (rooks >> 8) & empty;
	flood |= (rooks >> 8) & empty;
	return flood >> 8;
}

uint64_t northAttacks(uint64_t rooks, uint64_t blockers) {
	uint64_t flood = rooks;
	uint64_t empty = ~blockers;
	flood |= rooks = (rooks << 8) & empty;
	flood |= rooks = (rooks << 8) & empty;
	flood |= rooks = (rooks << 8) & empty;
	flood |= rooks = (rooks << 8) & empty;
	flood |= rooks = (rooks << 8) & empty;
	flood |= (rooks << 8) & empty;
	return flood << 8;
}

uint64_t eastAttacks(uint64_t rooks, uint64_t blockers) {
	const uint64_t notA = 0xfefefefefefefefeULL;
	uint64_t flood = rooks;
	uint64_t empty = ~blockers;
	empty &= notA;
	flood |= rooks = (rooks << 1) & empty;
	flood |= rooks = (rooks << 1) & empty;
	flood |= rooks = (rooks << 1) & empty;
	flood |= rooks = (rooks << 1) & empty;
	flood |= rooks = (rooks << 1) & empty;
	flood |= (rooks << 1) & empty;
	return (flood << 1) & notA;
}

uint64_t noEaAttacks(uint64_t bishops, uint64_t blockers) {
	const uint64_t notA = 0xfefefefefefefefeULL;
	uint64_t flood = bishops;
	uint64_t empty = ~blockers;
	empty &= notA;
	flood |= bishops = (bishops << 9) & empty;
	flood |= bishops = (bishops << 9) & empty;
	flood |= bishops = (bishops << 9) & empty;
	flood |= bishops = (bishops << 9) & empty;
	flood |= bishops = (bishops << 9) & empty;
	flood |= (bishops << 9) & empty;
	return (flood << 9) & notA;
}

uint64_t soEaAttacks(uint64_t bishops, uint64_t blockers) {
	const uint64_t notA = 0xfefefefefefefefeULL;
	uint64_t flood = bishops;
	uint64_t empty = ~blockers;
	empty &= notA;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= bishops = (bishops >> 7) & empty;
	flood |= (bishops >> 7) & empty;
	return (flood >> 7) & notA;
}

uint64_t westAttacks(uint64_t rooks, uint64_t blockers) {
	const uint64_t notH = 0x7f7f7f7f7f7f7f7fULL;
	uint64_t flood = rooks;
	uint64_t empty = ~blockers;
	empty &= notH;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= rooks = (rooks >> 1) & empty;
	flood |= (rooks >> 1) & empty;
	return (flood >> 1) & notH;
}

uint64_t soWeAttacks(uint64_t bishops, uint64_t blockers) {
	const uint64_t notH = 0x7f7f7f7f7f7f7f7fULL;
	uint64_t flood = bishops;
	uint64_t empty = ~blockers;
	empty &= notH;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= bishops = (bishops >> 9) & empty;
	flood |= (bishops >> 9) & empty;
	return (flood >> 9) & notH;
}

uint64_t noWeAttacks(uint64_t bishops, uint64_t blockers) {
	const uint64_t notH = 0x7f7f7f7f7f7f7f7fULL;
	uint64_t flood = bishops;
	uint64_t empty = ~blockers;
	empty &= notH;
	flood |= bishops = (bishops << 7) & empty;
	flood |= bishops = (bishops << 7) & empty;
	flood |= bishops = (bishops << 7) & empty;
	flood |= bishops = (bishops << 7) & empty;
	flood |= bishops = (bishops << 7) & empty;
	flood |= (bishops << 7) & empty;
	return (flood << 7) & notH;
}

void MoveGen::init() {
    
	// check time
	auto start = chrono::high_resolution_clock::now();

    /***
    Initialize ray attacks for each square on the board.
    ***/

    // north
	uint64_t north = 0x101010101010100ULL;
	for (int square = 0; square < 64; square++) {
		north &= 0xffffffffffffff;
		rayAttacks[square][NORTH] = north;
		north <<= 1;
	}

	// northeast
	uint64_t northeast = 0x102040810204000ULL;
	for (int col = 7; col >= 0; col--) {
		uint64_t ne = northeast;
		for (int r8 = 0; r8 < 8 * 8; r8 += 8, ne <<= 8) {
			rayAttacks[r8 + col][NORTHEAST] = ne;
		}
		northeast = ((northeast >> 1) & 0x7f7f7f7f7f7f7f7f);
	}

	// east
	uint64_t ogEast = 0xffULL;
	for (int col = 7; col >= 0; col--) {
		if (ogEast) { // prevents int overflow
			ogEast -= (1ULL << col);
		}
		ogEast &= 0xfefefefefefefefe;
		uint64_t east = ogEast;

		for (int row = 0; row < 8; row++) {
			rayAttacks[row * 8 + col][EAST] = east;
			east <<= 8;
		}
	}

	// south east
	uint64_t southeast = 0x40201008040201ULL;
	for (int col = 7; col >= 0; col--) {
		uint64_t se = southeast;
		for (int row = 7; row >= 0; row--, se >>= 8) {
			rayAttacks[row * 8 + col][SOUTHEAST] = se;
		}
		southeast = ((southeast >> 1) & 0x7f7f7f7f7f7f7f7f);
	}

	// south
	uint64_t south = 0x0080808080808080ULL;
	for (int square = 63; square >= 0; square--) {
		south &= 0xffffffffffffff00;
		rayAttacks[square][SOUTH] = south;
		south >>= 1;
	}

	// south west
	uint64_t southwest = 0x2040810204080ULL;
	for (int col = 0; col < 8; col++) {
		uint64_t sw = southwest;
		for (int row = 7; row >= 0; row--, sw >>= 8) {
			rayAttacks[row * 8 + col][SOUTHWEST] = sw;
		}
		southwest = ((southwest << 1) & 0xfefefefefefefefe);
	}

	// west
	uint64_t ogWest = 0xffULL;
	for (int col = 0; col < 8; col++) {
		if (ogWest) { // prevents int overflow
			ogWest -= (1ULL << col);
		}
		ogWest &= 0x7f7f7f7f7f7f7f7f;
		uint64_t west = ogWest;

		for (int row = 0; row < 8; row++) {
			rayAttacks[row * 8 + col][WEST] = west;
			west <<= 8;
		}
	}

	// north west
	uint64_t northwest = 0x8040201008040200ULL;
	for (int col = 0; col < 8; col++) {
		uint64_t nw = northwest;
		for (int r8 = 0; r8 < 8 * 8; r8 += 8, nw <<= 8) {
			rayAttacks[r8 + col][NORTHWEST] = nw;
		}
		northwest = ((northwest << 1) & 0xfefefefefefefefe);
	}

	/***
    Initializes Rook Attacks and Lookup Tables
    ***/

	for (int i = 0; i < 64; i++) {
		rookRays[i] = rayAttacks[i][NORTH] | rayAttacks[i][SOUTH] | rayAttacks[i][WEST] | rayAttacks[i][EAST];
	}
	rookLookupOffsets[0] = 0;
	for (int i = 1; i < 64; i++) {
		rookLookupOffsets[i] = rookLookupOffsets[i - 1] + (1ULL << __popcnt64(rookRays[i - 1]));
	}

    for (int square = 0; square < 64; square++) {
		uint64_t maxPos = 1ULL << __popcnt64(rookRays[square]);
		uint64_t rook = 1ULL << square;
		for (uint64_t i = 0; i < maxPos; i++) {
			uint64_t blockers = _pdep_u64(i, rookRays[square]);

			rookLookup[rookLookupOffsets[square] + i] = northAttacks(rook, blockers);
			rookLookup[rookLookupOffsets[square] + i] |= southAttacks(rook, blockers);
			rookLookup[rookLookupOffsets[square] + i] |= eastAttacks(rook, blockers);
			rookLookup[rookLookupOffsets[square] + i] |= westAttacks(rook, blockers);
		}
	}

	/***
    Initializes Bishop Attacks and Lookup Tables
    ***/

	for (int i = 0; i < 64; i++) {
		bishopRays[i] = rayAttacks[i][NORTHEAST] | rayAttacks[i][NORTHWEST] | rayAttacks[i][SOUTHEAST] | rayAttacks[i][SOUTHWEST];
		bishopRays[i] &= 0x7e7e7e7e7e7e00;
	}
	bishopLookupOffsets[0] = 0;
	for (int i = 1; i < 64; i++) {
		bishopLookupOffsets[i] = bishopLookupOffsets[i - 1] + (1ULL << __popcnt64(bishopRays[i - 1]));
	}

    for (int square = 0; square < 64; square++) {
		uint64_t maxPos = 1ULL << __popcnt64(bishopRays[square]);
		uint64_t bishop = 1ULL << square;
		for (uint64_t i = 0; i < maxPos; i++) {
			uint64_t blockers = _pdep_u64(i, bishopRays[square]);

			bishopLookup[bishopLookupOffsets[square] + i] = noEaAttacks(bishop, blockers);
			bishopLookup[bishopLookupOffsets[square] + i] |= noWeAttacks(bishop, blockers);
			bishopLookup[bishopLookupOffsets[square] + i] |= soEaAttacks(bishop, blockers);
			bishopLookup[bishopLookupOffsets[square] + i] |= soWeAttacks(bishop, blockers);
		}
	}

    /***
    Initializes Knight and King Attacks
    ***/
    for (int square = 0; square < 64; square++) {
		uint64_t knightPos = (1ULL << square);
		uint64_t horizontal1 = ((knightPos << 1) & 0xfefefefefefefefe) | ((knightPos >> 1) & 0x7f7f7f7f7f7f7f7f);
		uint64_t horizontal2 = ((knightPos << 2) & 0xfcfcfcfcfcfcfcfc) | ((knightPos >> 2) & 0x3f3f3f3f3f3f3f3f);
		knightLookup[square] = (horizontal2 << 8) | (horizontal2 >> 8) | (horizontal1 << 16) | (horizontal1 >> 16);
	}

    for (int square = 0; square < 64; square++) {
		uint64_t kingPos = (1ULL << square);
		kingPos |= ((kingPos << 1) & 0xfefefefefefefefe) | ((kingPos >> 1) & 0x7f7f7f7f7f7f7f7f);
		kingPos |= (kingPos << 8) | (kingPos >> 8);
		kingLookup[square] = kingPos & ~(1ULL << square);
	}

	// check time
	auto end = chrono::high_resolution_clock::now();
	double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
	time_taken *= 1e-9;
	cout << "info string Initiated move generator in " << fixed << time_taken << setprecision(9) << " secs" << endl;
}