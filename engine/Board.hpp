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
	 * @brief sets bitboards to standard starting pos
	 *
	 * @param moveGen helper class used to generate pseudo legal moves
	 */
	void setStartingPos();

	/**
	 * @brief move piece given wtv lichess gives -> have to make modifications
	 *
	 * @param move move in string
	 */
	void movePiece(string);
	/**
	 * @brief move piece in standard piece bitboards
	 *
	 * @param move pointer to move object
	 */
	void movePiece(Move *);

	/**
	 * @brief move piece in standard piece bitboards
	 *
	 * @param from int representing the square piece is on
	 * @param to int representing the square piece will move to
	 */
	void movePiece(int, int);

	/**
	 * @brief
	 *
	 * @param color bool representing the color to generate attack board for, white = false; black = true
	 * @param moveGen helper class used to generate pseudo legal moves (and in this case attacks)
	 */
	void genAttackBoard(bool);

	/**
	 * @brief generates all possible moves for the current board
	 *
	 * @param debug whether or not to print time msgs
	 */
	void genMoves(bool = false);
};