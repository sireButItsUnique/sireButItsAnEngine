#include "Search.hpp"
using namespace Search;

namespace Search {
    vector<vector<int32_t>> history;
    vector<vector<uint32_t>> killer;
    int64_t NODE_COUNT;
    TimePoint START_TIME;
    int64_t TIME_LIMIT;
    int MAX_DEPTH;
    bool ABORT_SEARCH; // Flag to abort search if needed
}

void Search::initSearch(int64_t timeLimit) {
    history = vector<vector<int32_t>>(64, vector<int32_t>(4096 + 5, -90000000)); // Initialize history for move ordering
    killer = vector<vector<uint32_t>>(64, vector<uint32_t>(2, 0)); // Initialize killer moves
    START_TIME = chrono::high_resolution_clock::now();
    TIME_LIMIT = timeLimit;
    ABORT_SEARCH = false;
    NODE_COUNT = 0;
}

// @brief temporary function to evaluate the board until nnue
int32_t evalBoard(Board& board) {
    int32_t score = 0;

    // Evaluate material balance
    score += _popcnt64(board.pieceBoards[PAWN + board.turn]) * 100; // Pawns
    score -= _popcnt64(board.pieceBoards[PAWN + !board.turn]) * 100;
    score += _popcnt64(board.pieceBoards[KNIGHT + board.turn]) * 300; // Knights
    score -= _popcnt64(board.pieceBoards[KNIGHT + !board.turn]) * 300;
    score += _popcnt64(board.pieceBoards[BISHOP + board.turn]) * 350; // Bishops
    score -= _popcnt64(board.pieceBoards[BISHOP + !board.turn]) * 350;
    score += _popcnt64(board.pieceBoards[ROOK + board.turn]) * 500; // Rooks
    score -= _popcnt64(board.pieceBoards[ROOK + !board.turn]) * 500;
    score += _popcnt64(board.pieceBoards[QUEEN + board.turn]) * 900; // Queens
    score -= _popcnt64(board.pieceBoards[QUEEN + !board.turn]) * 900;

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
        if (!board.moveIsCapture(move)) continue; // Skip moves that don't capture
        
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move

        if (newBoard.pieceBoards[KING + newBoard.turn] == 0) return MATE_SCORE; // Check for checkmate
        if (newBoard.kingIsAttacked(board.turn)) continue; // Skip moves that leave the king in check

        // Evaluate the new position
        int32_t score = -Search::finishCaptures(newBoard, -beta, -alpha, depth + 1); // Negate for minimax

        // Prune if move is too good -> opp has a better move last ply
        if (score >= beta) return score;
        eval = max(eval, score);
        alpha = max(alpha, eval);
    }

    // Return the evaluated score
    if (abs(eval) > MATE_SCORE - 100) {
        if (eval > 0) return eval - 1;
        else return eval + 1;
    }
    return eval;
}

int32_t Search::bestMoves(Board& board, int depth, int32_t alpha, int32_t beta, vector<vector<uint32_t>>& PV) {
    
    // Time management
    if (Search::ABORT_SEARCH) return 0;
    int64_t timeUsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - Search::START_TIME).count();
    if (timeUsed > Search::TIME_LIMIT) {
        Search::ABORT_SEARCH = true;
        return 0;
    }
    Search::NODE_COUNT++; 
    
    // Generate moves and order them
    vector<uint32_t> moves;
    MoveGen::genMoves(board, moves, board.turn);
    int realDepth = MAX_DEPTH - depth; // Adjust depth for history table
    
    vector<pair<int32_t, uint32_t>> moveScores(moves.size());
    for (int i = 0; i < moves.size(); ++i) {
        int32_t score = history[realDepth][(moves[i] & 0x3ffc000) >> 14]; // Extract historical eval of the move for history table
        moveScores[i] = {score, moves[i]};
    }
    stable_sort(moveScores.rbegin(), moveScores.rend());

    // Initialize evaluation score to a very low value
    int32_t eval = -INFINITE_SCORE;

    // Iterate through all possible moves
    int illegals = 0;
    for (pair<int32_t, uint32_t>& m : moveScores) {
        
        // Copy-make move
        uint32_t move = m.second;
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move
        
        // Check legality of the move
        if (newBoard.kingIsAttacked(board.turn)) {
            illegals++;
            continue; // Skip moves that leave the king in check
        }

        // Evaluate the new position
        int32_t score; // Negative because score is from opponent's perspective
        if (depth > 0) score = -Search::bestMoves(newBoard, depth - 1, -beta, -alpha, PV); // Negate for minimax
        // else score = -Search::finishCaptures(newBoard, -beta, -alpha, 1); // Leaf node evaluation
        else score = -evalBoard(newBoard); // Leaf node evaluation
        Search::history[realDepth][(move & 0x3ffc000) >> 14] = score; // Update history table for move ordering

        // Prune if move is too good -> opp has a better move last ply
        if (score >= beta) {
            if (!board.moveIsCapture(move)) {
                killer[realDepth][1] = killer[realDepth][0];
                killer[realDepth][0] = move; // Update killer moves
            }
            return score;
        }
			
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

    // Return the evaluated score
    if (abs(eval) > MATE_SCORE - 100) {
        if (eval > 0) return eval - 1;
        else return eval + 1;
    }
    if (illegals == moves.size()) return -MATE_SCORE;
    return eval;
}

/*
white 
black 
white
*/