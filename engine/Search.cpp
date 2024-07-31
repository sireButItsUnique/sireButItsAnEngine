#include "Search.hpp"

double rec(Board* board, int ply, Eval* evaluator) {

    // base case
    if (!ply) {
        return evaluator->getBoardEval(board);
    }
    double eval;
    board->genMoves();
    if (!board->moves.size()) {
        return 0;
    }
    
    
    // keep recing: should make a move even if it leads to mate
    if (board->turn == white) {
        eval = KING_DEAD * -2;
    } else {
        eval = KING_DEAD * 2;
    }

    // search all moves
    for (Move& move: board->moves) {
        Board* branch = new Board(*board);
        branch->movePiece(&move);
        double moveEval = rec(branch, ply - 1, evaluator);

        if (board->turn == white && moveEval > eval) {
            eval = moveEval;
        } 
        if (board->turn == black && moveEval < eval) {
            eval = moveEval;
        }
    }

    return eval;
}

Search::Search() {

}

Move Search::getBestMove(Board* board, int ply, double& eval, Eval* evaluator) {
    Move res = Move(16, 16);
    board->genMoves();

    // should make a move even if it leads to mate
    if (board->turn == white) {
        eval = KING_DEAD * -2;
    } else {
        eval = KING_DEAD * 2;
    }

    // search all moves
    for (Move& move: board->moves) {
        Board* branch = new Board(*board);
        branch->movePiece(&move);
        double moveEval = rec(branch, ply - 1, evaluator);

        if (board->turn == white && moveEval > eval) {
            res = move;
            eval = moveEval;
        } 
        if (board->turn == black && moveEval < eval) {
            res = move;
            eval = moveEval;
        }
    }

    return res;
}