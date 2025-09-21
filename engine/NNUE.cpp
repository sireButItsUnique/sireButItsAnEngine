#include "NNUE.hpp"
#include "incbin.h"

extern "C" {
	INCBIN(network_weights, "engine/network_data.bin");
}

namespace NNUE {
    int16_t acc_weights[INPUT_SIZE][ACC_SIZE];
    int16_t out_weights[ACC_SIZE * 2][OUTPUT_SIZE];

    int16_t acc_bias[ACC_SIZE];
    int16_t out_bias[OUTPUT_SIZE];
}

void NNUE::init() {
    char *ptr = (char *)gnetwork_weightsData;
	memcpy(acc_weights, ptr, sizeof(acc_weights));
	ptr += sizeof(acc_weights);
	memcpy(acc_bias, ptr, sizeof(acc_bias));
	ptr += sizeof(acc_bias);
	memcpy(out_weights, ptr, sizeof(out_weights));
	ptr += sizeof(out_weights);
	memcpy(&out_bias, ptr, sizeof(out_bias));
}

void NNUE::initAccBias(int32_t acc[2 * ACC_SIZE]) {
    memset(acc, 0, sizeof(acc)); // Initialize acc to zero
    for (int i = 0; i < ACC_SIZE; i++) {
        acc[i] += acc_bias[i];
        acc[i + ACC_SIZE] += acc_bias[i]; 
    }
}

int32_t NNUE::evalBoardFast(Board& board, int32_t acc[2 * ACC_SIZE], int16_t accMailbox[64]) {
    
    // Update acc layer based on changes from last move
    for (int square = 0; square < 64; square++) {
        if (board.mailbox[square] != accMailbox[square]) {
            
            // Get piece
            int widxNew = board.mailbox[square] / 2; 
            int bidxNew = board.mailbox[square] / 2; 
            int widxOld = accMailbox[square] / 2; 
            int bidxOld = accMailbox[square] / 2; 

            // Add side offset for opp's pieces
            if ((board.mailbox[square] % 2) == BLACK) widxNew += 6; 
            if ((board.mailbox[square] % 2) == WHITE) bidxNew += 6;
            if ((accMailbox[square] % 2) == BLACK) widxOld += 6; 
            if ((accMailbox[square] % 2) == WHITE) bidxOld += 6;

            // Multiply by 64 for correct slots
            widxNew *= 64;
            bidxNew *= 64;
            widxOld *= 64;
            bidxOld *= 64;

            // add square offset (flip board if black)
            widxNew += square;
            bidxNew += (square ^ 56);
            widxOld += square;
            bidxOld += (square ^ 56);
            
            // calc acc layer immediately to save time and memory -> remove old piece, add new piece
            if (board.turn == WHITE) {
                for (int j = 0; j < ACC_SIZE; j++) {
                    if (accMailbox[square] != EMPTY) {
                        acc[j] -= acc_weights[widxOld][j];
                        acc[j + ACC_SIZE] -= acc_weights[bidxOld][j];
                    }

                    if (board.mailbox[square] != EMPTY) {
                        acc[j] += acc_weights[widxNew][j];
                        acc[j + ACC_SIZE] += acc_weights[bidxNew][j];   
                    }
                }
            } else {
                for (int j = 0; j < ACC_SIZE; j++) {
                    if (accMailbox[square] != EMPTY) {
                        acc[j] -= acc_weights[bidxOld][j];
                        acc[j + ACC_SIZE] -= acc_weights[widxOld][j];
                    }
                    
                    if (board.mailbox[square] != EMPTY) {
                        acc[j] += acc_weights[bidxNew][j];
                        acc[j + ACC_SIZE] += acc_weights[widxNew][j];
                    }
                }
            }
        }
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

    // Update accMailbox to current board
    memcpy(accMailbox, board.mailbox, sizeof(accMailbox));

    return output;
}

int32_t NNUE::evalBoard(Board& board) {
    
    // Prepare input layers
    int32_t acc[2 * ACC_SIZE] = {0};
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
            widx += square;
            bidx += (square ^ 56);
            
            // calc acc layer immediately to save time and memory
            if (board.turn == WHITE) {
                for (int j = 0; j < ACC_SIZE; j++) {
                    acc[j] += acc_weights[widx][j];
                    acc[j + ACC_SIZE] += acc_weights[bidx][j];
                }
            } else {
                for (int j = 0; j < ACC_SIZE; j++) {
                    acc[j] += acc_weights[bidx][j];
                    acc[j + ACC_SIZE] += acc_weights[widx][j];
                }
            }
        }
    }

    // Finish computing hidden layer
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