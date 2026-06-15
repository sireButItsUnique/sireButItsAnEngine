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
    uint64_t TT_SIZE;
    uint64_t mask;
    TTEntry *table;
}