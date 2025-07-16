#include "Search.hpp"
using namespace Search;

namespace Search {
    int64_t count = 0; // Global move count for debugging
}

// @brief temporary function to evaluate the board until nnue
int32_t evalBoard(Board& board) {
    int32_t score = 0;

    // Evaluate material balance
    score += __builtin_popcountll(board.pieceBoards[PAWN + board.turn]) * 100; // Pawns
    score -= __builtin_popcountll(board.pieceBoards[PAWN + !board.turn]) * 100;
    score += __builtin_popcountll(board.pieceBoards[KNIGHT + board.turn]) * 300; // Knights
    score -= __builtin_popcountll(board.pieceBoards[KNIGHT + !board.turn]) * 300;
    score += __builtin_popcountll(board.pieceBoards[BISHOP + board.turn]) * 350; // Bishops
    score -= __builtin_popcountll(board.pieceBoards[BISHOP + !board.turn]) * 350;
    score += __builtin_popcountll(board.pieceBoards[ROOK + board.turn]) * 500; // Rooks
    score -= __builtin_popcountll(board.pieceBoards[ROOK + !board.turn]) * 500;
    score += __builtin_popcountll(board.pieceBoards[QUEEN + board.turn]) * 900; // Queens
    score -= __builtin_popcountll(board.pieceBoards[QUEEN + !board.turn]) * 900;
    score += __builtin_popcountll(board.pieceBoards[KING + board.turn]) * 20000; // Kings
    score -= __builtin_popcountll(board.pieceBoards[KING + !board.turn]) * 20000;

    return score;
}

int32_t Search::bestMoves(Board& board, int depth, int32_t alpha, int32_t beta, vector<vector<uint32_t>>& PV) {
    Search::count++; // Increment move count for debugging
    
    vector<uint32_t> moves;
    MoveGen::genMoves(board, moves, board.turn);

    int32_t eval = -50000; // Initialize to a very low value

    for (uint32_t move : moves) {
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move

        // Evaluate the new position
        int32_t score = evalBoard(newBoard);
        if (depth > 0) score = -Search::bestMoves(newBoard, depth - 1, -beta, -alpha, PV); // Negate for minimax

        // Prune if move is too good -> opp has a better move last ply
        if (eval >= beta) return eval;
			
        if (score > eval) {
            eval = score;
            if (score > alpha) {
                alpha = score;
                PV[depth][0] = move; // Store the best move for this depth
                if (depth > 0) {
                    for (int i = 0; i + 1 < 64; ++i) PV[depth][i + 1] = PV[depth - 1][i];
                }
            }
        }
    }

    return eval;
}