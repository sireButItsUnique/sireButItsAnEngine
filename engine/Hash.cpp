#include "Hash.hpp"

namespace Zobrist {
    uint64_t PIECES[12][64];
    uint64_t CASTLING[8];
    uint64_t TURN[2];
};

void Zobrist::init() {
    srand(287818); // ifykyk
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 64; ++j) {
            Zobrist::PIECES[i][j] = RAND_64();
        }
    }
    for (int i = 0; i < 8; ++i) {
        Zobrist::CASTLING[i] = RAND_64();
    }
    for (int i = 0; i < 2; ++i) {
        Zobrist::TURN[i] = RAND_64();
    }
}

namespace TT {
    const int TT_SIZE = 1 << 20;
    TTEntry table[TT_SIZE]; // 1M entries
}

void TT::set(uint64_t key, int32_t eval, int depth, uint64_t move, uint8_t flag) {
    TTEntry *entry = TT::table + (key % TT_SIZE);

    // Don't overwrite a better entry
    if (entry->depth >= depth) return;

    // Write the new entry
    entry->key = key;
    entry->eval = eval;
    entry->depth = depth;
    entry->move = move;
    entry->flag = flag;
}

TTEntry* TT::get(uint64_t key) {
    TTEntry *entry = TT::table + (key % TT_SIZE);
    
    // Check if the entry is valid
    if (entry->key == key) return entry;
    
    return nullptr; // No valid entry found
}