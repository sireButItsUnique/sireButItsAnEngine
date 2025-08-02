#pragma once
#include "includes.hpp"
#include "helper.hpp"
#include "Move.hpp"
#include "Hash.hpp"

class Board {
public:
	uint64_t pieceBoards[12], colorBoards[2];
	int16_t mailbox[64]; // mailbox for piece positions
	bool castlingRights[4]; // castling rights: 0=white king, 1=white queen, 2=black king, 3=black queen

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
	inline bool moveIsCapture(uint32_t move) {
		if (Move::isCastle(move)) return false; // Castling is not a capture
		return ((1ULL << Move::to(move)) & colorBoards[!Move::color(move)]);
	}

	/**
	 * @brief gets the Zobrist key for the current position
	 *
	 * @return Zobrist key
	 */
	uint64_t getZobristKey();

	/**
	 * @brief prints the board to stdout
	 */
	void print();
};