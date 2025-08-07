#pragma once
#include "includes.hpp"
#include "helper.hpp"

namespace Zobrist {
    extern uint64_t PIECES[12][64];
    extern uint64_t CASTLING[8];
    extern uint64_t TURN[2];

    /**
     * @brief Initializes the Zobrist hashing tables.
     */
    void init();
}

struct TTEntry {

    /**
     * @brief The Zobrist key for the position. Used to make sure we don't get entry for the wrong position in case of hash collisions.
     */
    uint64_t key;

    /**
     * @brief The evaluated score of the position.
     */
	int32_t eval;

    /**
     * @brief The depth of the search at which this entry was created.
     * This is used to determine if the entry is still valid for the current search depth.
     */
	int depth;

    /**
     * @brief The best move associated with this entry.
     */
	uint64_t move;

    /**
     * @brief The flag associated with this entry.
     * This is used to determine the type of entry (exact, lower bound, upper bound).
     */
	uint8_t flag;
};

namespace TT {
    extern const int TT_SIZE;
    extern TTEntry table[1 << 22];

    /**
     * @brief Sets a transposition table entry.
     * @param key The Zobrist key for the position.
     * @param eval The evaluated score of the position.
     * @param depth The depth of the search at which this entry was created.
     * @param move The best move associated with this entry.
     * @param flag The flag associated with this entry.
     */
    void set(uint64_t key, int32_t eval, int depth, uint64_t move, uint8_t flag);

    /**
     * @brief Gets a transposition table entry.
     * @param key The Zobrist key for the position.
     * @return A pointer to the TTEntry if found, otherwise nullptr.
     */
    TTEntry* get(uint64_t key);
}