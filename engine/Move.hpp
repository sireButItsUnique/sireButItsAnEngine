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
	inline void setColor(uint32_t& data, bool color) {
		if (color) data |= 0x80000000; // set the highest bit for black
		else data &= ~0x80000000; // clear the highest bit for white
	}

	/**
	 * @brief sets the position of the piece in the bitboard
	 *
	 * @param from square piece is on
	 * @param to square piece will move to
	 */
	inline void setPosition(uint32_t& data, uint8_t from, uint8_t to) {
		data |= (from << 20); // set the from bits
		data |= (to << 14); // set the to bits
	}

	/**
	 * @brief sets the promotion flags
	 *
	 * @param piece piece code the pawn is promoting to
	 */
	inline void setPromotion(uint32_t& data, uint8_t piece) {
		data |= 0x40000000; // set the promotion bit
		data &= ~0x30000000; // clear the previous promotion bits
		data |= (((piece >> 1) - 1) << 28); // set the piece code for promotion
	}

	/**
	 * @brief sets the castling flags
	 *
	 * @param side true = queenside; false = kingside
	 */
	inline void setCastle(uint32_t& data, bool side) {
		data |= 0x8000000; // set the castle bit
		if (side) data |= 0x4000000; // set the queenside flag
		else data &= ~0x4000000; // clear the queenside flag for kingside
	}

	/**
	 * @brief returns the color of the piece that is moving
	 *
	 * @return true = black; false = white
	 */
	inline bool color(uint32_t& data) {
		return (data & 0x80000000);
	}

	/**
	 * @brief returns the square the piece came from
	 *
	 * @return uint8_t
	 */
	inline uint8_t from(uint32_t& data) {
		return (data & 0x3F00000) >> 20;
	}

	/**
	 * @brief returns the square the piece moved to
	 *
	 * @return uint8_t
	 */
	inline uint8_t to(uint32_t& data) {
		return (data & 0xFC000) >> 14;
	}

	/**
	 * @brief returns whether or not the move has a promotion flag
	 *
	 * @return true
	 * @return false
	 */
	inline bool isPromotion(uint32_t& data) {
		return (data & 0x40000000);
	}

	/**
	 * @brief returns which piece the pawn promoted to during the move
	 */
	inline uint8_t promotionPiece(uint32_t& data) {
		return (((((data & 0x30000000) >> 28) + 1) << 1) + color(data));
	}

	/**
	 * @brief returns whether or not the move has a castle flag
	 *
	 * @return true
	 * @return false
	 */
	inline bool isCastle(uint32_t& data) {
		return (data & 0x8000000);
	}

	/**
	 * @brief returns which side the guy is castling
	 *
	 * @return true = queenside
	 * @return false = kingside
	 */
	inline bool castleSide(uint32_t& data) {
		return (data & 0x4000000);
	}

	/**
	 * @brief returns the 14 bit functionally unique id of the move
	 * @attention 16384 possible ids, so only 14 bits are used
	 */
	inline uint16_t id(uint32_t& data) {
		uint16_t id = (data & 0x3ffc000) >> 14; // 12 bits for from & to
		id |= (Move::color(data) << 13); // 1 bit for color
		id |= (Move::castleSide(data) << 12); // 1 bit for castle side
		return id;
	}

	/**
	 * @brief returns algebraic move
	 */
	inline string toAlgebra(uint32_t& data) {
		if (isCastle(data)) {
			if (color(data) == BLACK) return (castleSide(data) ? "e8c8" : "e8g8");
			else return (castleSide(data) ? "e1c1" : "e1g1");
		}

		if (isPromotion(data)) {
			string pieces = "pnbrq";
			return (TO_ALGEBRA(from(data)) + TO_ALGEBRA(to(data)) + pieces[promotionPiece(data) / 2]);
		}
		return (TO_ALGEBRA(from(data)) + TO_ALGEBRA(to(data)));
	}
};