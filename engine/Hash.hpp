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
    extern uint32_t TT_SIZE;
    extern uint64_t TT_LEN;
    extern uint64_t mask;
    extern TTEntry *table;

    /**
     * @brief Sets a transposition table entry.
     * @param key The Zobrist key for the position.
     * @param eval The evaluated score of the position.
     * @param depth The depth of the search at which this entry was created.
     * @param move The best move associated with this entry.
     * @param flag The flag associated with this entry.
     */
    inline void set(uint64_t key, int32_t eval, int depth, uint64_t move, uint8_t flag) {
        TTEntry *entry = TT::table + (key & TT::mask);

        // Write the new entry
        if (depth >= entry->depth || entry->key != key) {
            entry->key = key;
            entry->eval = eval;
            entry->depth = depth;
            entry->move = move;
            entry->flag = flag;
        }
    }



    /**
     * @brief Gets a transposition table entry.
     * @param key The Zobrist key for the position.
     * @return A pointer to the TTEntry if found, otherwise nullptr.
     */
    inline TTEntry* get(uint64_t key) {
        TTEntry *entry = TT::table + (key & TT::mask);
        
        // Check if the entry is valid
        if (entry->key == key) return entry;
        
        return nullptr; // No valid entry found
    }


    /**
     * @brief Initializes the transposition table with the given size in MB
     * @param size The size of the transposition table (number of entries)
     */
    inline void init(uint32_t size) {
        uint64_t maxEntries = (size << 20) / sizeof(TTEntry);

        // max amount of entry that fits in given size
        int n = 1;
        while ((1ULL << n) <= maxEntries) n++;
        n--; 
        uint32_t numEntries = 1ULL << n;

        if (table != nullptr) delete[] table;

        TT_SIZE = size;
        TT_LEN = numEntries;
        mask = TT_LEN - 1;
        table = new TTEntry[TT_LEN];
        memset(table, 0, sizeof(TTEntry) * TT_LEN);
    }
}