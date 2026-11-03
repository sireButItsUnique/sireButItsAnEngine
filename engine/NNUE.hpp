#pragma once

#include "includes.hpp"
#include "Board.hpp"

#define INPUT_SIZE 768
#define ACC_SIZE 128
#define OUTPUT_SIZE 1
#define QA 255
#define QB 64
#define SCALE 400

/**
 * @brief Hidden layer for both perspectives: [0, ACC_SIZE) = white, [ACC_SIZE, 2 * ACC_SIZE) = black
 */
struct Accumulator {
    alignas(32) int16_t vals[2 * ACC_SIZE];
};

/**
 * @brief A piece appearing on / disappearing from a square
 */
struct AccDelta {
    int16_t piece;
    int16_t square;
};

namespace NNUE {
    extern int16_t acc_weights[INPUT_SIZE][ACC_SIZE];
    extern int16_t out_weights[OUTPUT_SIZE][2][ACC_SIZE * 2];

    extern int16_t acc_bias[ACC_SIZE];
    extern int16_t out_bias[OUTPUT_SIZE];

    void init();

    /**
     * @brief Builds the accumulator from scratch (bias + every piece on the board)
     */
    void refresh(Board& board, Accumulator& acc);

    /**
     * @brief child = parent - subs + adds, only touches the few features that changed
     */
    void update(const Accumulator& parent, Accumulator& child, const AccDelta* subs, int numSubs, const AccDelta* adds, int numAdds);

    /**
     * @brief Runs the output layer on an already up to date accumulator
     */
    int32_t evaluate(const Accumulator& acc, bool turn);

    int32_t evalBoard(Board& board);
}