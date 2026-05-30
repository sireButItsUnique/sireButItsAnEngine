#include "NNUE.hpp"
#include "incbin.h"

extern "C" {
	INCBIN(network_weights, "engine/network_data.bin");
}

namespace NNUE {
    int16_t acc_weights[INPUT_SIZE][ACC_SIZE];
    int16_t out_weights[OUTPUT_SIZE][2][ACC_SIZE * 2];

    int16_t acc_bias[ACC_SIZE];
    int16_t out_bias[OUTPUT_SIZE];
}

void NNUE::init() {
    char *ptr = (char *)gnetwork_weightsData;
	memcpy(acc_weights, ptr, sizeof(acc_weights));
	ptr += sizeof(acc_weights);
	memcpy(acc_bias, ptr, sizeof(acc_bias));
	ptr += sizeof(acc_bias);
	int16_t tmp[OUTPUT_SIZE][ACC_SIZE * 2];
	memcpy(tmp, ptr, sizeof(tmp));
	ptr += sizeof(tmp);
	memcpy(&out_bias, ptr, sizeof(out_bias));

    for (int i = ACC_SIZE; i < ACC_SIZE * 2; i++) {
        out_weights[0][WHITE][i] = tmp[0][i];
        out_weights[0][BLACK][i] = tmp[0][i - ACC_SIZE];
    }
    for (int i = 0; i < ACC_SIZE; i++) {
        out_weights[0][WHITE][i] = tmp[0][i];
        out_weights[0][BLACK][i] = tmp[0][i + ACC_SIZE];
    }
}

void NNUE::initAccBias(int32_t (&acc)[2 * ACC_SIZE]) {
    memset(acc, 0, sizeof(acc)); // Initialize acc to zero
    for (int i = 0; i < ACC_SIZE; i++) {
        acc[i] += acc_bias[i];
        acc[i + ACC_SIZE] += acc_bias[i]; 
    }
}

int32_t getWeightIdx(int piece, int square, bool perspective) {
    int idx = piece / 2; // Get piece
    if ((piece % 2) != perspective) idx += 6; // Add side offset for opponent's pieces
    idx *= 64; // Multiply by 64 for correct slots
    if (perspective) idx += (square ^ 56); // Add square offset (flip board if black)
    else idx += square;
    return idx;
}

int32_t sumVec32(__m256i vec) {
    __m128i sum = _mm_add_epi32(_mm256_castsi256_si128(vec), _mm256_extracti128_si256(vec, 1));
    sum = _mm_add_epi32(sum, _mm_shuffle_epi32(sum, _MM_PERM_BADC));
    sum = _mm_add_epi32(sum, _mm_shuffle_epi32(sum, _MM_PERM_CDAB));

    return _mm_cvtsi128_si32(sum);
}

int32_t NNUE::evalBoardFast(Board& board, int32_t (&acc)[2 * ACC_SIZE], Board& accBoard) {
    
    // Update acc layer based on changes from last move
    int16_t* accMailbox = accBoard.mailbox;
    for (int square = 0; square < 64; square++) {
        if (board.mailbox[square] != accMailbox[square]) {

            // remove old piece
            for (int j = 0; j < ACC_SIZE; j++) {
                if (accMailbox[square] < EMPTY) {
                    acc[j] -= acc_weights[getWeightIdx(accMailbox[square], square, WHITE)][j];
                    acc[j + ACC_SIZE] -= acc_weights[getWeightIdx(accMailbox[square], square, BLACK)][j];
                }
            }

            // add new piece
            for (int j = 0; j < ACC_SIZE; j++) {
                if (board.mailbox[square] < EMPTY) {
                    acc[j] += acc_weights[getWeightIdx(board.mailbox[square], square, WHITE)][j];
                    acc[j + ACC_SIZE] += acc_weights[getWeightIdx(board.mailbox[square], square, BLACK)][j];
                }
            }
        }
    }

    // Compute output layer
    __m256i clampMin = _mm256_setzero_si256();
    __m256i clampMax = _mm256_set1_epi32(QA);
    __m256i sum = _mm256_setzero_si256();

    for (int i = 0; i < ACC_SIZE * 2; i += 16) {
        // Load 16x int32 in two halves bc its somehow overflowing if 16 bit is used
        __m256i acc1 = _mm256_loadu_si256((__m256i*)&acc[i]);
        __m256i acc2 = _mm256_loadu_si256((__m256i*)&acc[i + 8]);

        acc1 = _mm256_max_epi32(acc1, clampMin);
        acc1 = _mm256_min_epi32(acc1, clampMax);
        acc2 = _mm256_max_epi32(acc2, clampMin);
        acc2 = _mm256_min_epi32(acc2, clampMax);

        __m256i input = _mm256_packs_epi32(acc1, acc2);
        input = _mm256_permute4x64_epi64(input, _MM_SHUFFLE(3, 1, 2, 0));

        __m256i weight = _mm256_loadu_si256((__m256i*)&out_weights[0][board.turn][i]);
        
        __m256i x = _mm256_mullo_epi16(input, weight);
        x = _mm256_madd_epi16(x, input);
        sum = _mm256_add_epi32(sum, x);
    }

    int output = sumVec32(sum);
    
    output /= QA;
    output += out_bias[0];
    output *= SCALE;
    output /= QA * QB;

    // Update accBoard to current board
    accBoard = board;
    return output;
}

int32_t NNUE::evalBoard(Board& board) {
    
    // Prepare input layers
    int16_t acc[2 * ACC_SIZE] = {0};
    for (int square = 0; square < 64; square++) {
        if (board.mailbox[square] != EMPTY) {

            // calc acc layer immediately to save time and memory
            for (int j = 0; j < ACC_SIZE; j++) {
                acc[j] += acc_weights[getWeightIdx(board.mailbox[square], square, WHITE)][j];
                acc[j + ACC_SIZE] += acc_weights[getWeightIdx(board.mailbox[square], square, BLACK)][j];
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
    int16_t clampMin = 0, clampMax = QA;
    for (int i = 0; i < ACC_SIZE * 2; i++) {
        int32_t input = clamp(acc[i], clampMin, clampMax);
        output += (input * input) * out_weights[0][board.turn][i];
    }
    output /= QA;
    output += out_bias[0];
    output *= SCALE;
    output /= QA * QB;

    return output;
}