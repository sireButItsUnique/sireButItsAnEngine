#include "Search.hpp"
using namespace Search;

namespace Search {
    int64_t count = 0; // Global move count for debugging
}

// @brief temporary function to evaluate the board until nnue
int32_t evalBoard(Board& board) {
    int32_t score = 0;

    // Evaluate material balance
    score += __popcnt64(board.pieceBoards[PAWN + board.turn]) * 100; // Pawns
    score -= __popcnt64(board.pieceBoards[PAWN + !board.turn]) * 100;
    score += __popcnt64(board.pieceBoards[KNIGHT + board.turn]) * 300; // Knights
    score -= __popcnt64(board.pieceBoards[KNIGHT + !board.turn]) * 300;
    score += __popcnt64(board.pieceBoards[BISHOP + board.turn]) * 350; // Bishops
    score -= __popcnt64(board.pieceBoards[BISHOP + !board.turn]) * 350;
    score += __popcnt64(board.pieceBoards[ROOK + board.turn]) * 500; // Rooks
    score -= __popcnt64(board.pieceBoards[ROOK + !board.turn]) * 500;
    score += __popcnt64(board.pieceBoards[QUEEN + board.turn]) * 900; // Queens
    score -= __popcnt64(board.pieceBoards[QUEEN + !board.turn]) * 900;
    score += __popcnt64(board.pieceBoards[KING + board.turn]) * 20000; // Kings
    score -= __popcnt64(board.pieceBoards[KING + !board.turn]) * 20000;

    return score;
}

int32_t Search::finishCaptures(Board& board, int32_t alpha, int32_t beta) {
    Search::count++; // Increment move count for debugging
    
    vector<uint32_t> moves;
    MoveGen::genMoves(board, moves, board.turn);

    int32_t eval = -50000; // Initialize to a very low value
    int32_t staticEval = evalBoard(board);
    if (staticEval >= beta) return beta;
    if (staticEval > alpha) alpha = staticEval;
    eval = staticEval; // Start with static evaluation since we are not forced to play a capture

    for (uint32_t move : moves) {
        if (!((1ULL << Move::to(move)) & board.colorBoards[!board.turn])) continue; // Skip moves that don't capture
        
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move

        // Evaluate the new position
        int32_t score = -Search::finishCaptures(newBoard, -beta, -alpha); // Negate for minimax

        // Prune if move is too good -> opp has a better move last ply
        if (score >= beta) return beta;
        eval = max(eval, score);
        alpha = max(alpha, eval);
    }

    return eval;
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
        int32_t score; // Negative because score is from opponent's perspective
        if (depth > 0) score = -Search::bestMoves(newBoard, depth - 1, -beta, -alpha, PV); // Negate for minimax
        else score = -Search::finishCaptures(newBoard, -beta, -alpha); // Leaf node evaluation

        // Prune if move is too good -> opp has a better move last ply
        if (score >= beta) return beta;
			
        if (score > eval) {
            eval = score;
            if (score > alpha) {
                alpha = score;
                PV[depth][0] = move; // Store the best move for this depth
                if (depth > 0) {
                    for (int i = 0; i + 1 < 60; ++i) PV[depth][i + 1] = PV[depth - 1][i];
                }
            }
        }
    }

    if (moves.empty()) return evalBoard(board);
    return eval;
}