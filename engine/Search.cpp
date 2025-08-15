#include "Search.hpp"
using namespace Search;

namespace Search {
    int64_t NODE_COUNT;
    TimePoint START_TIME;
    int64_t TIME_LIMIT;
    int MAX_DEPTH;
    bool ABORT_SEARCH; // Flag to abort search if needed

    // Standard move ordering stuff
    vector<vector<uint32_t>> killer; // Killer moves for each depth
    vector<int32_t> history; // History table for quiet move ordering
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
    START_TIME = chrono::high_resolution_clock::now();
    TIME_LIMIT = timeLimit;
    ABORT_SEARCH = false;
    NODE_COUNT = 0;

    killer = vector<vector<uint32_t>>(36, vector<uint32_t>(2, 0)); // Initialize killer moves for each depth
    history = vector<int32_t>(16384, 0); // Initialize history table for quiet move ordering
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

void Search::updateHistory(uint32_t move, int32_t bonus) {
    const int MAX_HISTORY = MATE_SCORE;
	int clamped_bonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY); // Ensure the bonus is within bounds
	history[Move::id(move)] += clamped_bonus - history[Move::id(move)] * abs(clamped_bonus) / MAX_HISTORY; // Update the history value
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
    vector<uint32_t> captures;
    moves.reserve(240);
    captures.reserve(240);
    MoveGen::genMoves(board, moves, board.turn);

    // Collect capturing moves and sort them by qsearch history
    for (uint32_t move : moves) {
        if (board.moveIsCapture(move)) {
            captures.push_back(move);
        }
    }
    sort(captures.begin(), captures.end(), [&](uint32_t a, uint32_t b) {
        return Search::MVV_LVA[board.mailbox[Move::to(a)] >> 1][board.mailbox[Move::from(a)] >> 1] >
               Search::MVV_LVA[board.mailbox[Move::to(b)] >> 1][board.mailbox[Move::from(b)] >> 1];
    });
    
    // Iterate through all capturing moves
    for (uint32_t move : captures) {
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
    if ((NODE_COUNT & 1023) == 0) {
        int64_t timeUsed = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - Search::START_TIME).count();
        if (timeUsed > Search::TIME_LIMIT) {
            Search::ABORT_SEARCH = true;
            return 0;
        }
    }
    Search::NODE_COUNT++;
    
    // Check for transposition table entry (not allowed in root search node)
    uint32_t hashMove = 0;
    TTEntry *entry = TT::get(board.key);
    if (depth != MAX_DEPTH) {
        if (entry && entry->depth >= depth) {
            // Entry exists and satisfies depth requirement
            if (entry->flag == TT_EXACT) return entry->eval;
            else if (entry->flag == TT_LOWER) {
                if (entry->eval >= beta) return entry->eval; // Will never be played, we can prune the search
            } else if (entry->flag == TT_UPPER) {
                if (entry->eval <= alpha) return entry->eval; // Worse for sure, we can prune the search
            }

            hashMove = entry->move; // Get the best move from the transposition table
        }
    }

    // Generate moves and order them
    vector<uint32_t> moves;
    vector<pair<int32_t, uint32_t>> scored;
    moves.reserve(240);
    scored.reserve(240);
    MoveGen::genMoves(board, moves, board.turn);
    
    int realDepth = MAX_DEPTH - depth; // depth = how many left, realDepth = how many already done (same realDepth = similar board state)
    bool beatAlpha = false; // Flag to check if we beat alpha in this node
    for (uint32_t move : moves) {
        int32_t score;

        // Transposition table move ordering
        if (move == hashMove) {
            score = INFINITE_SCORE; // Highest score for the hash move
        }

        // Capturing Moves Ordering
        else if (board.moveIsCapture(move)) {
            score = 20000 + Search::MVV_LVA[board.mailbox[Move::to(move)] >> 1][board.mailbox[Move::from(move)] >> 1];
        } 
        
        // Quiet Moves Ordering
        else {
            score = -10000;
            if (move == killer[realDepth][0]) score += 1500; // Killer moves
            if (move == killer[realDepth][1]) score += 1000;
            score += history[Move::id(move)]; // Historical value
        }
         
        scored.push_back({score, move});
    }
    sort(scored.rbegin(), scored.rend());

    // Initialize evaluation score to a very low value
    int32_t eval = -INFINITE_SCORE;
    uint64_t bestMove = 0; // Best move for this depth (keep for TT ordering)

    // Iterate through all possible moves
    int illegals = 0;
    for (int idx = 0; idx < scored.size(); ++idx) {
        uint32_t move = scored[idx].second;
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move
        if (newBoard.kingIsAttacked(board.turn)) {
            illegals++;
            continue; // Skip moves that leave the king in check
        }

        // Evaluate the new position
        int32_t score; // Negative because score is from opponent's perspective
        if (newBoard.threeFold()) score = 0; // If the position is drawn
        else if (depth > 0) score = -Search::bestMoves(newBoard, depth - 1, -beta, -alpha, PV); // Negate for minimax
        else score = -Search::finishCaptures(newBoard, -beta, -alpha, 0); // Leaf node evaluation

        // Time management here so we don't write bs into transposition table (thanks sebastian lague)
        if (Search::ABORT_SEARCH) return 0;

        // Prune if move is too good -> opp has a better move last ply
        if (score >= beta) {

            // Store killer moves
            if (!board.moveIsCapture(move) && move != killer[realDepth][0] && move != killer[realDepth][1]) {
                killer[realDepth][1] = killer[realDepth][0];
                killer[realDepth][0] = move; // Store the killer move
            }

            // Update history
            if (!board.moveIsCapture(move)) {
                int32_t bonus = depth * depth; // Usually quadratic is a good choice, because it rewards deeper searches more
                Search::updateHistory(move, bonus); // Update the history
                for (int i = 0; i < idx; ++i) {
                    if (!board.moveIsCapture(scored[i].second)) Search::updateHistory(scored[i].second, -bonus); // Penalize bad quiet moves
                }
            }

            // Update transposition table
            TT::set(board.key, score, depth, move, TT_LOWER); // Store the transposition table entry

            // Exit early since we found a move that is too good
            return score;
        }

        // Update evaluation score & update bounds
        if (score > eval) {
            eval = score;
            bestMove = move;
            if (score > alpha) {
                beatAlpha = true;
                alpha = score;
                PV[depth][0] = move; // Store the best move for this depth
            }
        }
    }

    // Adjust for mate scores & update transposition table
    if (illegals == moves.size()) {
        eval = -MATE_SCORE;
    }
    else if (abs(eval) > MATE_SCORE - 100) {
        if (eval > 0) eval--;
        else eval++;
    }

    // Update transposition table.
    if (!beatAlpha) TT::set(board.key, eval, depth, bestMove, TT_UPPER);
    else TT::set(board.key, eval, depth, bestMove, TT_EXACT);

    return eval;
}