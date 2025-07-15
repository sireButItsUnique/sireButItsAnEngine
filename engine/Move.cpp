#include "Move.hpp"
using namespace Move;

inline void Move::setColor(uint32_t& data, bool color) {
	if (color) data |= 0x80000000; // set the highest bit for black
	else data &= ~0x80000000; // clear the highest bit for white
}

inline void Move::setPosition(uint32_t& data, uint8_t from, uint8_t to) {
	data |= (from << 20); // set the from bits
	data |= (to << 14); // set the to bits
}

inline void Move::setPromotion(uint32_t& data, uint8_t piece) {
	data |= 0x40000000; // set the promotion bit
	data |= (((piece >> 1) - 1) << 28); // set the piece code for promotion
}

inline void Move::setCastle(uint32_t& data, bool side) {
	data |= 0x8000000; // set the castle bit
	if (side) data |= 0x4000000; // set the queenside flag
	else data &= ~0x4000000; // clear the queenside flag for kingside
}

inline bool Move::color(uint32_t& data) {
	return (data & 0x80000000);
}

inline uint8_t Move::from(uint32_t& data) {
	return (data & 0x3F00000) >> 20;
}

inline uint8_t Move::to(uint32_t& data) {
	return (data & 0xFC000) >> 14;
}

inline bool Move::isPromotion(uint32_t& data) {
	return (data & 0x40000000);
}

inline uint8_t Move::promotionPiece(uint32_t& data) {
	return ((((data & 0x30000000) >> 28) + 1) << 1 + color(data));
}

inline bool Move::isCastle(uint32_t& data) {
	return (data & 0x8000000);
}

inline bool Move::castleSide(uint32_t& data) {
	return (data & 0x4000000);
}

inline string Move::toAlgebra(uint32_t& data) {
	if (isCastle(data)) {
		if (color(data) == BLACK) return (castleSide(data) ? "e8c8" : "e8g8");
        else return (castleSide(data) ? "e1c1" : "e1g1");
	}

	if (isPromotion(data)) return (TO_ALGEBRA(from(data)) + TO_ALGEBRA(to(data))); // add promotion piece logic
	return (TO_ALGEBRA(from(data)) + TO_ALGEBRA(to(data)));
}