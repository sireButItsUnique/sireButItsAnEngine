#pragma once
#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"

namespace MoveGen {
    /**
	 * @brief stores precomputed ray attacks for empty board
	 *
	 * @param square what square the piece is on
	 * @param direction north starts at 0, numbers increase w/ clockwise rotation
	 */
    extern uint64_t rayAttacks[64][8];
	extern uint64_t rookRays[64];
	extern uint64_t bishopRays[64];

	extern uint64_t rookLookupOffsets[64];
	extern uint64_t bishopLookupOffsets[64];

	extern uint64_t rookLookup[102400];
	extern uint64_t bishopLookup[5248];
	extern uint64_t knightLookup[64];
	extern uint64_t kingLookup[64];
    
    /**
     * @brief Initializes the ray attacks and lookup tables
     * This function should be called once at the start of the program.
     */
    void init();

    /**
     * @brief Generates pseudo-legal moves for the given board and color.
     *
     * @param board The current board state.
     * @param moves Vector to store the generated moves.
     * @param color The color for which to generate moves (WHITE or BLACK).
     */
    void genMoves(Board& board, vector<uint32_t>& moves, bool color);

    void genPawnMoves(Board& board, vector<uint32_t>& moves, bool color);
    void genKnightMoves(Board& board, vector<uint32_t>& moves, bool color);
    void genBishopMoves(Board& board, vector<uint32_t>& moves, bool color);
    void genRookMoves(Board& board, vector<uint32_t>& moves, bool color);
    void genQueenMoves(Board& board, vector<uint32_t>& moves, bool color);
    void genKingMoves(Board& board, vector<uint32_t>& moves, bool color);
    void genCastlingMoves(Board& board, vector<uint32_t>& moves, bool color);

    // @brief auto generates lookup tables for ray attacks
    // struct Initializer {
	// 	Initializer() {
	// 		MoveGen::init();  // calls your existing init function
	// 	}
	// };

    // static Initializer initializer;
}
