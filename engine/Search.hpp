#pragma once
#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"

namespace Search {
    extern int64_t NODE_COUNT;
    extern TimePoint START_TIME;
    extern int64_t TIME_LIMIT;
    extern bool ABORT_SEARCH; // Flag to abort search if needed

    /**
     * @brief Initializes the search parameters.
     *
     * @param timeLimit The time limit for the search in milliseconds.
     */
    void initSearch(int64_t timeLimit);
    
    /**
     * @brief Evaluates the board position after a "steady" state is reached
     *
     * @param board The current board state.
     * @param alpha The highest score that the cur player is guranteed
     * @param beta The highest score that the opp player is guranteed. If the score is greater than beta, the search can be pruned since from opp's POV it will be worse than beta.
     * @return An integer score representing the evaluation of the board.
     */
    int32_t finishCaptures(Board& board, int32_t alpha, int32_t beta, int depth);

    /**
     * @brief Finds the best moves for the current player.
     *
     * @param board The current board state.
     * @param depth The search depth.
     * @param alpha The highest score that the cur player is guranteed
     * @param beta The highest score that the opp player is guranteed. If the score is greater than beta, the search can be pruned since from opp's POV it will be worse than beta.
     * @param PV Stores the best moves
     * @return An integer score representing the evaluation of the best move.
     */
    int32_t bestMoves(Board& board, int depth, int32_t alpha, int32_t beta, vector<vector<uint32_t>>& PV);
}