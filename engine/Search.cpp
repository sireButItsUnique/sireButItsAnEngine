#include "Search.hpp"
using namespace Search;

namespace Search {
    vector<vector<int32_t>> history;
    vector<vector<int32_t>> qhistory;
    int64_t NODE_COUNT;
    TimePoint START_TIME;
    int64_t TIME_LIMIT;
    int MAX_DEPTH;
    bool ABORT_SEARCH; // Flag to abort search if needed

    // Standard move ordering stuff
    vector<vector<uint32_t>> killer; // Killer moves for each depth
    constexpr int32_t MVV_LVA[7][7] = {
        // PNBRQKX
        {15, 14, 13, 12, 11, 10, 0}, // Taking a pawn
        {25, 24, 23, 22, 21, 20, 0}, // Taking a knight
        {35, 34, 33, 32, 31, 30, 0}, // Taking a bishop
        {45, 44, 43, 42, 41, 40, 0}, // Taking a rook
        {55, 54, 53, 52, 51, 50, 0}, // Taking a queen
        {0, 0, 0, 0, 0, 0, 0}, // Taking a king (should never happen)
        {0, 0, 0, 0, 0, 0, 0} // No Piece
    };
}

void Search::initSearch(int64_t timeLimit) {
    history = vector<vector<int32_t>>(36, vector<int32_t>(4096 + 5, -90000000)); // Initialize history for move ordering
    qhistory = vector<vector<int32_t>>(36, vector<int32_t>(4096 + 5, -90000000)); // Initialize qsearch history for move ordering
    START_TIME = chrono::high_resolution_clock::now();
    TIME_LIMIT = timeLimit;
    ABORT_SEARCH = false;
    NODE_COUNT = 0;

    killer = vector<vector<uint32_t>>(36, vector<uint32_t>(2, 0)); // Initialize killer moves for each depth
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
    if ((NODE_COUNT & 1023) == 0) {
        int64_t timeUsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - Search::START_TIME).count();
        if (timeUsed > Search::TIME_LIMIT) {
            Search::ABORT_SEARCH = true;
            return 0;
        }
    }
    Search::NODE_COUNT++; 

    // Initialize evaluation score
    int32_t eval = -INFINITE_SCORE; // Initialize to a very low value
    int32_t staticEval = evalBoard(board);
    if (staticEval >= beta) return beta;
    if (staticEval > alpha) alpha = staticEval;
    eval = staticEval; // Start with static evaluation since we are not forced to play a capture

    // Generate all possible moves for the current player
    vector<uint32_t> moves;
    vector<pair<int32_t, uint32_t>> captures;
    moves.reserve(240);
    captures.reserve(240);
    MoveGen::genMoves(board, moves, board.turn);

    // Collect capturing moves and sort them by qsearch history
    for (uint32_t move : moves) {
        if (!Move::isCastle(move) && board.moveIsCapture(move)) {
            int from = 6;
            int to = 6;
            for (int i = 0; i < 12; ++i) {
                if (board.pieceBoards[i] & (1ULL << Move::from(move))) from = (i >> 1);
                if (board.pieceBoards[i] & (1ULL << Move::to(move))) to = (i >> 1);
            }
            int32_t score = Search::MVV_LVA[from][to];
            captures.push_back({score, move});
        }
    }
    stable_sort(captures.rbegin(), captures.rend());
    
    // Iterate through all capturing moves
    for (pair<int32_t, uint32_t>& c: captures) {
        uint32_t move = c.second;
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move

        if (newBoard.pieceBoards[KING + newBoard.turn] == 0) return MATE_SCORE; // Check for checkmate
        if (newBoard.kingIsAttacked(board.turn)) continue; // Skip moves that leave the king in check

        // Evaluate the new position
        int32_t score = -Search::finishCaptures(newBoard, -beta, -alpha, depth + 1); // Negate for minimax
        Search::qhistory[depth][(move & 0x3ffc000) >> 14] = score; // Update qsearch history for move ordering

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
    if ((NODE_COUNT & 1023) == 0) {
        int64_t timeUsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - Search::START_TIME).count();
        if (timeUsed > Search::TIME_LIMIT) {
            Search::ABORT_SEARCH = true;
            return 0;
        }
    }
    Search::NODE_COUNT++; 
    
    // Generate moves and order them
    vector<uint32_t> moves;
    vector<pair<int32_t, uint32_t>> scored;
    moves.reserve(240);
    scored.reserve(240);
    MoveGen::genMoves(board, moves, board.turn);
    
    int realDepth = MAX_DEPTH - depth; // Adjust depth for history table
    for (uint32_t move : moves) {
        int32_t score;

        if (board.moveIsCapture(move)) {
            int from = 6;
            int to = 6;
            for (int i = 0; i < 12; ++i) {
                if (board.pieceBoards[i] & (1ULL << Move::from(move))) from = (i >> 1);
                if (board.pieceBoards[i] & (1ULL << Move::to(move))) to = (i >> 1);
            }
            score = 20000 + Search::MVV_LVA[from][to];
        } else {
            score = -10000;
            score += 1500 * (move == killer[0][realDepth]);
            score += 1000 * (move == killer[1][realDepth]);
        }
         
        scored.push_back({score, move});
    }
    stable_sort(scored.rbegin(), scored.rend());

    // Initialize evaluation score to a very low value
    int32_t eval = -INFINITE_SCORE;

    // Iterate through all possible moves
    int illegals = 0;
    for (pair<int32_t, uint32_t>& c: scored) {
        uint32_t move = c.second;
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move
        if (newBoard.kingIsAttacked(board.turn)) {
            illegals++;
            continue; // Skip moves that leave the king in check
        }

        // Evaluate the new position
        int32_t score; // Negative because score is from opponent's perspective
        if (depth > 0) score = -Search::bestMoves(newBoard, depth - 1, -beta, -alpha, PV); // Negate for minimax
        else score = -Search::finishCaptures(newBoard, -beta, -alpha, 0); // Leaf node evaluation
        Search::history[realDepth][(move & 0x3ffc000) >> 14] = score; // Update history table for move ordering

        // Prune if move is too good -> opp has a better move last ply
        if (score >= beta) {

            // Store killer moves
            if (!board.moveIsCapture(move) && move != killer[0][realDepth] && move != killer[1][realDepth]) {
                killer[1][realDepth] = killer[0][realDepth];
                killer[0][realDepth] = move; // Store the killer move
            }
            return score;
        }

        // Update evaluation score & update bounds
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