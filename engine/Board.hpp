#pragma once
#include "includes.hpp"
#include "helper.hpp"
#include "Move.hpp"

class Board {
public:
	uint64_t pieceBoards[12], colorBoards[2];
	bool whiteQueenCastle, whiteKingCastle, blackQueenCastle, blackKingCastle;

	/**
	 * @brief who's turn it is to move, white=false; black=true
	 */
	bool turn;

	Board();

	/**
	 * @brief sets the copy of the board to the current position
	 *
	 * @param board pointer to the board to copy
	 */
	void setCopyBoard(Board* board);

	/**
	 * @brief sets bitboards to standard starting pos
	 *
	 * @param moveGen helper class used to generate pseudo legal moves
	 */
	void setStartingPos();

	/**
	 * @brief sets bitboards to pos given by FEN string
	 *
	 * @param fen string representing the position in FEN format
	 */
	void setFenPos(string pos, string turn, string castling, string enPassant);

	/**
	 * @brief move piece in standard piece bitboards
	 *
	 * @param move move to make
	 */
	void movePiece(uint32_t move);

	/**
	 * @brief checks if the square is attacked by the opponent
	 *
	 * @param square square to check
	 * @param color color of the player to check for attacks
	 * @return true if the square is attacked, false otherwise
	 */
	bool squareIsAttacked(bool color, int square);

	/**
	 * @brief checks if the king of the mentioned player is attacked
	 *
	 * @return true if the king is attacked, false otherwise
	 */
	bool kingIsAttacked(bool color);

	/**
	 * @brief checks if the move is a capturing move
	 *
	 * @param move move to check
	 * @return true if the move is a capturing move, false otherwise
	 */
	bool moveIsCapture(uint32_t move) {
		return ((1ULL << Move::to(move)) & colorBoards[!Move::color(move)]);
	}

	/**
	 * @brief prints the board to stdout
	 */
	void print();
};