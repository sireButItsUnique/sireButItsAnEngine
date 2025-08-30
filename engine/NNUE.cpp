#include "NNUE.hpp"

namespace NNUE {
    int16_t acc_weights[INPUT_SIZE][ACC_SIZE];
    int16_t out_weights[ACC_SIZE * 2][OUTPUT_SIZE];

    int16_t acc_bias[ACC_SIZE];
    int16_t out_bias[OUTPUT_SIZE];
}

void NNUE::init() {
    ifstream file("engine/network_data.bin", ios::binary);

    file.read((char*)(acc_weights), sizeof(acc_weights));
    file.read((char*)(acc_bias), sizeof(acc_bias));
    file.read((char*)(out_weights), sizeof(out_weights));
    file.read((char*)(out_bias), sizeof(out_bias));
}

int32_t NNUE::evalBoard(Board& board) {
    
    // Prepare input layers
    int16_t winput[INPUT_SIZE] = {0};
    int16_t binput[INPUT_SIZE] = {0};
    for (int square = 0; square < 64; square++) {
        if (board.mailbox[square] != EMPTY) {
            // Get piece
            int widx = board.mailbox[square] / 2; 
            int bidx = board.mailbox[square] / 2; 

            // Add side offset for opp's pieces
            if ((board.mailbox[square] % 2) == BLACK) widx += 6; 
            if ((board.mailbox[square] % 2) == WHITE) bidx += 6;

            // Multiply by 64 for correct slots
            widx *= 64;
            bidx *= 64;

            // add square offset (flip board if black)
            winput[widx + square] = 1;
            binput[bidx + (square ^ 56)] = 1;
        }
    }

    // Compute hidden layer
    int32_t acc[2 * ACC_SIZE] = {0};
    for (int i = 0; i < INPUT_SIZE; i++) {
        if (winput[i]) for (int j = 0; j < ACC_SIZE; j++) acc[j] += acc_weights[i][j];
        if (binput[i]) for (int j = 0; j < ACC_SIZE; j++) acc[j + ACC_SIZE] += acc_weights[i][j];
    }
    for (int i = 0; i < ACC_SIZE; i++) {
        acc[i] += acc_bias[i];
        acc[i + ACC_SIZE] += acc_bias[i]; 
    }

    // Compute output layer
    int32_t output = 0;
    for (int i = 0; i < ACC_SIZE * 2; i++) {
        int16_t input = clamp(acc[i], 0, QA);
        output += (input * input) * out_weights[i][0];
    }
    output /= QA;
    output += out_bias[0];
    output *= SCALE;
    output /= QA * QB;

    return output;
}