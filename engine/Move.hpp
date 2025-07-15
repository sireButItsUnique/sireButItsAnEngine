#pragma once
#include "includes.hpp"
#include "helper.hpp"

/**
 * @brief [0] = color
 *        [1, 3] = promotion {is, piece, piece}
 *        [4, 5] = castling {is, side}
 *        [6, 11] = from
 *        [12, 17] = to
 *
 */
namespace Move {
	
	/**
	 * @brief sets the color of the piece that is moving
	 *
	 * @param color true = black; false = white
	 */
	inline void setColor(uint32_t& data, bool color);

	/**
	 * @brief sets the position of the piece in the bitboard
	 *
	 * @param from square piece is on
	 * @param to square piece will move to
	 */
	inline void setPosition(uint32_t& data, uint8_t from, uint8_t to);

	/**
	 * @brief sets the promotion flags
	 *
	 * @param piece piece code the pawn is promoting to
	 */
	inline void setPromotion(uint32_t& data, uint8_t piece);

	/**
	 * @brief sets the castling flags
	 *
	 * @param side true = queenside; false = kingside
	 */
	inline void setCastle(uint32_t& data, bool side);

	/**
	 * @brief returns the color of the piece that is moving
	 *
	 * @return true = black; false = white
	 */
	inline bool color(uint32_t& data);

	/**
	 * @brief returns the square the piece came from
	 *
	 * @return uint8_t
	 */
	inline uint8_t from(uint32_t& data);

	/**
	 * @brief returns the square the piece moved to
	 *
	 * @return uint8_t
	 */
	inline uint8_t to(uint32_t& data);

	/**
	 * @brief returns whether or not the move has a promotion flag
	 *
	 * @return true
	 * @return false
	 */
	inline bool isPromotion(uint32_t& data);

	/**
	 * @brief returns which piece the pawn promoted to during the move
	 */
	inline uint8_t promotionPiece(uint32_t& data);

	/**
	 * @brief returns whether or not the move has a castle flag
	 *
	 * @return true
	 * @return false
	 */
	inline bool isCastle(uint32_t& data);

	/**
	 * @brief returns which side the guy is castling
	 *
	 * @return true = queenside
	 * @return false = kingside
	 */
	inline bool castleSide(uint32_t& data);

	/**
	 * @brief returns algebraic move
	 */
	inline string toAlgebra(uint32_t& data);
};