#include "Search.hpp"
#include "NNUE.hpp"
using namespace Search;

namespace Search {
    int64_t NODE_COUNT;
    TimePoint START_TIME;
    int64_t TIME_LIMIT;
    int MAX_DEPTH;
    bool ABORT_SEARCH; // Flag to abort search if needed
    fast::lvector<uint64_t> threeFoldReps;

    // Standard move ordering stuff
    uint32_t killer[64][2]; // Killer moves for each depth
    int32_t history[16384]; // History table for quiet move ordering
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

void Search::initSearch(int64_t timeLimit, fast::lvector<uint64_t> threeFoldReps) {
    START_TIME = chrono::high_resolution_clock::now();
    TIME_LIMIT = timeLimit;
    ABORT_SEARCH = false;
    NODE_COUNT = 0;
    Search::threeFoldReps = threeFoldReps;

    memset(killer, 0, sizeof(killer)); // Initialize killer moves to zero
    memset(history, 0, sizeof(history)); // Initialize history table to zero
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
    int32_t staticEval = NNUE::evalBoard(board);
    if (staticEval >= beta) return beta;
    if (staticEval > alpha) alpha = staticEval;
    eval = staticEval; // Start with static evaluation since we are not forced to play a capture

    // Generate all possible moves for the current player
    fast::vector<uint32_t> moves;
    fast::vector<uint32_t> captures;
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

int32_t Search::bestMoves(Board& board, int depth, int32_t alpha, int32_t beta, vector<vector<uint32_t>>& PV, bool isPvNode) {

    // Leaf node, q search
    if (depth <= 0) return Search::finishCaptures(board, alpha, beta, 0);
    
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

    // Check for draw
    if (depth != MAX_DEPTH && board.threeFold(threeFoldReps)) {
        return 0; // Draw by threefold repetition
    }
    
    // Check for transposition table entry (not allowed in root search node)
    uint32_t hashMove = 0;
    TTEntry *entry = TT::get(board.key);
    if (!isPvNode) {
        if (entry) {

            // Only use the entry if it is good enough for the current search depth
            if (entry->depth >= depth) {
                if (entry->flag == TT_EXACT) return entry->eval;
                else if (entry->flag == TT_LOWER) {
                    if (entry->eval >= beta) return entry->eval; // Will never be played, we can prune the search
                } else if (entry->flag == TT_UPPER) {
                    if (entry->eval <= alpha) return entry->eval; // Worse for sure, we can prune the search
                }

                hashMove = entry->move; // Get the best move from the transposition table
            }
        }
    }

    // Get metadata for the current node
    //int32_t staticEval = NNUE::evalBoard(board);
    bool inCheck = board.kingIsAttacked(board.turn);
    bool pawnEndgame = false;
    for (int i = KNIGHT + WHITE; i <= QUEEN + BLACK; i++) {
        pawnEndgame |= board.pieceBoards[i];
    }
    pawnEndgame = !pawnEndgame;
    bool nearMate = false;
    if (abs(alpha) > MATE_SCORE - 100 || abs(beta) > MATE_SCORE - 100) nearMate = true;

    // Reverse frutility pruning
    // if (!inCheck && !isPvNode && !nearMate) {
    //     if (staticEval >= beta + (150 * depth)) {
    //         return staticEval - (150 * depth); // Prune the branch
    //     }
    // }

    // // Null move pruning
    // if (!inCheck && !pawnEndgame && staticEval >= beta) {
    //     board.turn = !board.turn; // Switch turn for null move
    //     int32_t nullMoveScore = -Search::bestMoves(board, max(0, depth - 4), -beta, -alpha, PV, false); // Null move search
    //     board.turn = !board.turn; // Switch back turn
    //     if (nullMoveScore >= beta) return nullMoveScore; // Prune the branch if null move score is too high
    // }

    // Generate moves and order them
    fast::vector<uint32_t> moves;
    fast::vector<pair<int32_t, uint32_t>> scored;
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
    sort(scored.begin(), scored.end(), [](const pair<int32_t, uint32_t>& a, const pair<int32_t, uint32_t>& b) {
        return a.first > b.first; // Sort in descending order
    });

    // Initialize evaluation score to a very low value
    int32_t eval = -INFINITE_SCORE;
    uint64_t bestMove = 0; // Best move for this depth (keep for TT ordering)

    // Iterate through all possible moves
    int illegals = 0;
    for (int idx = 0; idx < scored.size(); ++idx) {
        
        // Move making shenanigans
        uint32_t move = scored[idx].second;
        Board newBoard = board; // Create a copy of the board
        newBoard.movePiece(move); // Make the move
        if (newBoard.kingIsAttacked(board.turn)) {
            illegals++;
            continue; // Skip moves that leave the king in check
        }
        threeFoldReps.push_back(newBoard.key); // Add the new position to the threefold repetitions

        // PV node checking
        bool childIsPv = false; // Assume first move is always a PV only if parent is PV 
        if (isPvNode && idx == 0) childIsPv = true; // (true with perfect move ordering)

        // Evaluate the new position
        int32_t score; // Negative because score is from opponent's perspective
        score = -Search::bestMoves(newBoard, depth - 1, -beta, -alpha, PV, childIsPv); // Negate for minimax

        // Time management here so we don't write bs into transposition table (thanks sebastian lague)
        if (Search::ABORT_SEARCH) return 0;

        // Beta cutoff: move is too good, opponent has a better move last ply
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
            threeFoldReps.pop_back(); // Remove the cur position
            return score;
        }

        // Update evaluation score
        if (score > eval) {
            eval = score;
            bestMove = move;

            // Alpha cutoff: move is better than the current alpha, is a PV node
            if (score > alpha) {
                beatAlpha = true;
                alpha = score;
                PV[depth][0] = move; // Store the best move for this depth
            }
        }
        
        // Pop the last position from the threefold repetitions
        threeFoldReps.pop_back();
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