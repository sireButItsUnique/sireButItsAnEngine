#pragma once

#include "includes.hpp"
#include "Board.hpp"

#define INPUT_SIZE 768
#define ACC_SIZE 16
#define OUTPUT_SIZE 1
#define QA 255
#define QB 64
#define SCALE 400

namespace NNUE {
    extern int16_t acc_weights[INPUT_SIZE][ACC_SIZE];
    extern int16_t out_weights[ACC_SIZE * 2][OUTPUT_SIZE];

    extern int16_t acc_bias[ACC_SIZE];
    extern int16_t out_bias[OUTPUT_SIZE];

    void init();

    int32_t evalBoard(Board& board);
}