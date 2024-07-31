#pragma once
#include "Board.hpp"
#include "Move.hpp"
#include "Eval.hpp"
#include "includes.hpp"

class Search {
public:
    Search();

    /**
     * @param board the board
     */
    Move getBestMove(Board* board, int ply, double& eval, Eval* evaluator);
};