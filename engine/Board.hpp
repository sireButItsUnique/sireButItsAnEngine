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
	void setFenPos(string fen);

	/**
	 * @brief move piece in standard piece bitboards
	 *
	 * @param move move to make
	 */
	void movePiece(uint32_t move);

	/**
	 * @brief prints the board to stdout
	 */
	void print();
};