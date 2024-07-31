#pragma once
#include "Board.hpp"
#include "Move.hpp"
#include "includes.hpp"

class Eval {
public:
	Eval();

	/**
	 * @brief Get the CURRENT eval, not considering future moves
	 *
	 * @param board
	 * @return eval, negative = black advantage, positive = white advantage
	 */
	double getBoardEval(Board *);
};