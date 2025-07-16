#include "Search.hpp"
using namespace Search;

uint32_t Search::bestMove(Board& board, int depth) {
    vector<uint32_t> moves;
    MoveGen::genMoves(board, moves, board.turn);

    // For now, just return the first move as the best move
    if (!moves.empty()) {
        return moves[0];
    }
    
    return 0; // No valid moves
}