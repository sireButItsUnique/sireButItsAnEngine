#include "Search.hpp"
using namespace Search;

namespace Search {
    int64_t NODE_COUNT;
    TimePoint START_TIME;
    int64_t TIME_LIMIT;
    bool ABORT_SEARCH; // Flag to abort search if needed
}

void Search::initSearch(int64_t timeLimit) {
    START_TIME = chrono::high_resolution_clock::now();
    TIME_LIMIT = timeLimit;
    ABORT_SEARCH = false;
    NODE_COUNT = 0;
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

    return score;
}

int32_t Search::finishCaptures(Board& board, int32_t alpha, int32_t beta, int depth) {
    
    // Time management
    if (Search::ABORT_SEARCH) return 0;
    int64_t timeUsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - Search::START_TIME).count();
    if (timeUsed > Search::TIME_LIMIT) {
        Search::ABORT_SEARCH = true;
        return 0;
    }
    Search::NODE_COUNT++; 
    
    // Generate all possible moves for the current player
    vector<uint32_t> moves;
    MoveGen::genMoves(board, moves, board.turn);

    // Initialize evaluation score
    int32_t eval = -INFINITE_SCORE; // Initialize to a very low value
    int32_t staticEval = evalBoard(board);
    if (staticEval >= beta) return beta;
    if (staticEval > alpha) alpha = staticEval;
    eval = staticEval; // Start with static evaluation since we are not forced to play a capture
    
    // Iterate through all capturing moves
    for (uint32_t move : moves) {
        if (Move::isCastle(move)) continue; // Skip castling moves for captures
        if (!((1ULL << Move::to(move)) & board.colorBoards[!board.turn])) continue; // Skip moves that don't capture
        
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move

        if (newBoard.pieceBoards[KING + newBoard.turn] == 0) return MATE_SCORE; // Check for checkmate

        // Evaluate the new position
        int32_t score = -Search::finishCaptures(newBoard, -beta, -alpha, depth + 1); // Negate for minimax

        // Prune if move is too good -> opp has a better move last ply
        if (score >= beta) return beta;
        eval = max(eval, score);
        alpha = max(alpha, eval);
    }

    // Return the evaluated score
    if (abs(eval) > MATE_SCORE - 100) return eval - 1;
    return eval;
}

int32_t Search::bestMoves(Board& board, int depth, int32_t alpha, int32_t beta, vector<uint32_t>& PV) {
    
    // Time management
    if (Search::ABORT_SEARCH) return 0;
    int64_t timeUsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - Search::START_TIME).count();
    if (timeUsed > Search::TIME_LIMIT) {
        Search::ABORT_SEARCH = true;
        return 0;
    }
    Search::NODE_COUNT++; 
    
    // Initialize evaluation score
    vector<uint32_t> moves;
    MoveGen::genMoves(board, moves, board.turn);
    int32_t eval = -INFINITE_SCORE; // Initialize to a very low value

    // Iterate through all possible moves
    for (uint32_t move : moves) {
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move
        if (newBoard.pieceBoards[KING + newBoard.turn] == 0) return MATE_SCORE; // Check for checkmate

        // Evaluate the new position
        int32_t score; // Negative because score is from opponent's perspective
        if (depth > 0) score = -Search::bestMoves(newBoard, depth - 1, -beta, -alpha, PV); // Negate for minimax
        else score = -Search::finishCaptures(newBoard, -beta, -alpha, 1); // Leaf node evaluation

        // Prune if move is too good -> opp has a better move last ply
        if (score >= beta) return beta;
			
        if (score > eval) {
            eval = score;
            if (score > alpha) {
                alpha = score;
                PV[depth] = move; // Store the best move for this depth
            }
        }
    }

    // Return the evaluated score
    if (moves.empty()) return evalBoard(board);
    if (abs(eval) > MATE_SCORE - 100) return eval - 1;
    return eval;
}