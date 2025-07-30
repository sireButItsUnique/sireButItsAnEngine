#pragma once
#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"

namespace Search {
    /**
     * @brief Stores the historical eval of moves for move ordering.
     * This is a 2D vector, history[depth][move] = eval
     * @attention ONLY enter the from and to portion of the move, not the full move
     */
    extern vector<vector<int32_t>> history;

    /**
     * @brief The number of nodes evaluated during the search.
     */
    extern int64_t NODE_COUNT;
    
    /**
     * @brief The starting time of the search.
     */
    extern TimePoint START_TIME;
    
    /**
     * @brief The time limit for the search in milliseconds.
     */
    extern int64_t TIME_LIMIT;
    
    /**
     * @brief Flag to abort the search if needed.
     */
    extern bool ABORT_SEARCH;

    /**
     * @brief The maximum search depth.
     */
    extern int MAX_DEPTH;

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