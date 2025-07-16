#pragma once
#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"

namespace Search {
    uint32_t bestMove(Board& board, int depth);
}