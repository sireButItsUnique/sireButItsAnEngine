#include "Hash.hpp"

namespace Zobrist {
    uint64_t PIECES[12][64];
    uint64_t CASTLING[8];
    uint64_t TURN[2];
};

void Zobrist::init() {
    mt19937_64 rng(283818); // ifykyk
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 64; ++j) {
            Zobrist::PIECES[i][j] = rng();
        }
    }
    for (int i = 0; i < 8; ++i) {
        Zobrist::CASTLING[i] = rng();
    }
    for (int i = 0; i < 2; ++i) {
        Zobrist::TURN[i] = rng();
    }
}

namespace TT {
    const int TT_SIZE = 1 << 22; // 4M entries
    TTEntry table[TT_SIZE]; // 4M entries
}

void TT::set(uint64_t key, int32_t eval, int depth, uint64_t move, uint8_t flag) {
    TTEntry *entry = TT::table + (key % TT_SIZE);

    // Write the new entry
    if (depth >= entry->depth || entry->key != key) {
        entry->key = key;
        entry->eval = eval;
        entry->depth = depth;
        entry->move = move;
        entry->flag = flag;
    }
}

TTEntry* TT::get(uint64_t key) {
    TTEntry *entry = TT::table + (key % TT_SIZE);
    
    // Check if the entry is valid
    if (entry->key == key) return entry;
    
    return nullptr; // No valid entry found
}