#include "Eval.hpp"

Eval::Eval() {

}

double Eval::getBoardEval(Board* board) {
	double eval = 0;
	
	// king dead
	if (!board->pieceBoards[kingBlack]) {
		return KING_DEAD;
	}
	if (!board->pieceBoards[kingWhite]) {
		return -1 * KING_DEAD;
	}

	// simple sum piece val
	eval += 1 * __popcnt64(board->pieceBoards[pawnWhite]);
	eval -= 1 * __popcnt64(board->pieceBoards[pawnBlack]);
	eval += 3 * __popcnt64(board->pieceBoards[knightWhite]);
	eval -= 3 * __popcnt64(board->pieceBoards[knightBlack]);
	eval += 3 * __popcnt64(board->pieceBoards[bishopWhite]);
	eval -= 3 * __popcnt64(board->pieceBoards[bishopBlack]);
	eval += 5 * __popcnt64(board->pieceBoards[rookWhite]);
	eval -= 5 * __popcnt64(board->pieceBoards[rookBlack]);
	eval += 9 * __popcnt64(board->pieceBoards[queenWhite]);
	eval -= 9 * __popcnt64(board->pieceBoards[queenBlack]);

	return eval;
}