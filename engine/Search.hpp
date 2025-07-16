#pragma once
#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"

namespace Search {
    extern int64_t count;
    
    int32_t finishCaptures(Board& board, int32_t alpha, int32_t beta);

    int32_t bestMoves(Board& board, int depth, int32_t alpha, int32_t beta, vector<vector<uint32_t>>& PV);
}