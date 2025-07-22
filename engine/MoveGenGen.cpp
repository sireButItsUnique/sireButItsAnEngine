#include "MoveGen.hpp"
using namespace MoveGen;

void MoveGen::genMoves(Board& board, vector<uint32_t>& moves, bool color) {

    // Generate moves for each piece type
    genPawnMoves(board, moves, color);
    genKnightMoves(board, moves, color);
    genBishopMoves(board, moves, color);
    genRookMoves(board, moves, color);
    genQueenMoves(board, moves, color);
    genKingMoves(board, moves, color);

    // Generate castling moves if applicable
    genCastlingMoves(board, moves, color);
}

void MoveGen::genKnightMoves(Board& board, vector<uint32_t>& moves, bool color) {

    uint64_t knightBoard = board.pieceBoards[KNIGHT + color];
	
    while (knightBoard) {
		// get lookup
		uint8_t from = _tzcnt_u64(knightBoard);
		uint64_t knight = knightLookup[from];
		knight &= ~board.colorBoards[color];

		// iterate over end positions
		while (knight) {
			uint8_t to = _tzcnt_u64(knight);
            uint32_t move = 0;
            Move::setColor(move, color);
            Move::setPosition(move, from, to);
			moves.push_back(move);
			knight &= ~(1ULL << to);
		}
		knightBoard &= ~(1ULL << from);
	}
}

void MoveGen::genKingMoves(Board& board, vector<uint32_t>& moves, bool color) {

    uint64_t king = board.pieceBoards[KING + color];

	// get lookup
	uint8_t from = _tzcnt_u64(king);
	king = kingLookup[from];
	king &= ~board.colorBoards[color];

	// iterate over end positions
	while (king) {
		uint8_t to = _tzcnt_u64(king);
		uint32_t move = 0;
        Move::setColor(move, color);
        Move::setPosition(move, from, to);
        moves.push_back(move);
		king &= ~(1ULL << to);
	}
}

void MoveGen::genBishopMoves(Board& board, vector<uint32_t>& moves, bool color) {

    uint64_t bishopBoard = board.pieceBoards[BISHOP + color];
    uint64_t friendlyPieces = board.colorBoards[color];
    uint64_t enemyPieces = board.colorBoards[!color];

	while (bishopBoard) {
		// gen rays
		uint8_t from = _tzcnt_u64(bishopBoard);
		uint64_t bishop = bishopRays[from];

		// gen relevant bits + lookup
		uint64_t blockers = _pext_u64(friendlyPieces | enemyPieces, bishop);
		bishop = bishopLookup[bishopLookupOffsets[from] + blockers];
		bishop &= ~friendlyPieces;

		// iterate over end positions
		while (bishop) {
			uint8_t to = _tzcnt_u64(bishop);
			uint32_t move = 0;
            Move::setColor(move, color);
            Move::setPosition(move, from, to);
            moves.push_back(move);
			bishop &= ~(1ULL << to);
		}
		bishopBoard &= ~(1ULL << from);
	}
}

void MoveGen::genRookMoves(Board& board, vector<uint32_t>& moves, bool color) {

    uint64_t rookBoard = board.pieceBoards[ROOK + color];
    uint64_t friendlyPieces = board.colorBoards[color];
    uint64_t enemyPieces = board.colorBoards[!color];

	while (rookBoard) {
		// gen rays
		uint8_t from = _tzcnt_u64(rookBoard);
		uint64_t rook = rookRays[from];

		// gen relevant bits + lookup
		uint64_t blockers = _pext_u64(friendlyPieces | enemyPieces, rook);
		rook = rookLookup[rookLookupOffsets[from] + blockers];
		rook &= ~friendlyPieces;

		// iterate over end positions
		while (rook) {
			uint8_t to = _tzcnt_u64(rook);
			uint32_t move = 0;
            Move::setColor(move, color);
            Move::setPosition(move, from, to);
            moves.push_back(move);
			rook &= ~(1ULL << to);
		}
		rookBoard &= ~(1ULL << from);
	}
}

void MoveGen::genQueenMoves(Board& board, vector<uint32_t>& moves, bool color) {

    uint64_t queenBoard = board.pieceBoards[QUEEN + color];
    uint64_t friendlyPieces = board.colorBoards[color];
    uint64_t enemyPieces = board.colorBoards[!color];

	while (queenBoard) {
		// gen rays
		uint8_t from = _tzcnt_u64(queenBoard);
		uint64_t rookBlockers = rookRays[from];
		uint64_t bishopBlockers = bishopRays[from];

		// gen relevant bits + lookup
		rookBlockers = _pext_u64(friendlyPieces | enemyPieces, rookBlockers);
		bishopBlockers = _pext_u64(friendlyPieces | enemyPieces, bishopBlockers);
		uint64_t queen = rookLookup[rookLookupOffsets[from] + rookBlockers];
		queen |= bishopLookup[bishopLookupOffsets[from] + bishopBlockers];
		queen &= ~friendlyPieces;

		// iterate over end positions
		while (queen) {
			uint8_t to = _tzcnt_u64(queen);
			uint32_t move = 0;
            Move::setColor(move, color);
            Move::setPosition(move, from, to);
            moves.push_back(move);
			queen &= ~(1ULL << to);
		}
		queenBoard &= ~(1ULL << from);
	}
}

void MoveGen::genPawnMoves(Board& board, vector<uint32_t>& moves, bool color) { // fix endian

    uint64_t pawnBoard = board.pieceBoards[PAWN + color];
    uint64_t friendlyPieces = board.colorBoards[color];
    uint64_t enemyPieces = board.colorBoards[!color];

	while (pawnBoard) {
		// mask pawn
		uint8_t from = _tzcnt_u64(pawnBoard);
		uint64_t pawn = (1ULL << from);

		// pushes
		if (color) {
			pawn >>= 8;
			pawn &= ~(friendlyPieces | enemyPieces);
			if (pawn & 0xff0000000000) {
				pawn |= (pawn >> 8);
				pawn &= ~(friendlyPieces | enemyPieces);
			}
		} else {
			pawn <<= 8;
			pawn &= ~(friendlyPieces | enemyPieces);
			if (pawn & 0xff0000) {
				pawn |= (pawn << 8);
				pawn &= ~(friendlyPieces | enemyPieces);
			}
		}

		// captures
		if (color) {
			pawn |= (((1ULL << from) >> 9) & 0x7f7f7f7f7f7f7f7f & enemyPieces);
			pawn |= (((1ULL << from) >> 7) & 0xfefefefefefefefe & enemyPieces);
		} else {
			pawn |= (((1ULL << from) << 9) & 0xfefefefefefefefe & enemyPieces);
			pawn |= (((1ULL << from) << 7) & 0x7f7f7f7f7f7f7f7f & enemyPieces);
		}

		// iterate over end positions
		while (pawn) {
			uint8_t to = _tzcnt_u64(pawn);
			if (to >= 56 || to <= 7) {
                uint32_t move = 0;
                Move::setColor(move, color);
                Move::setPosition(move, from, to);
                Move::setPromotion(move, QUEEN);
                moves.push_back(move);
				Move::setPromotion(move, KNIGHT);
                moves.push_back(move);
                Move::setPromotion(move, ROOK);
                moves.push_back(move);
                Move::setPromotion(move, BISHOP);
                moves.push_back(move);
			} else {
				uint32_t move = 0;
                Move::setColor(move, color);
                Move::setPosition(move, from, to);
                moves.push_back(move);
			}

			pawn &= ~(1ULL << to);
		}
		pawnBoard &= ~(1ULL << from);
	}
}

void MoveGen::genCastlingMoves(Board& board, vector<uint32_t>& moves, bool color) {

    bool canCastleQueen = (color ? board.blackQueenCastle : board.whiteQueenCastle);
    bool canCastleKing = (color ? board.blackKingCastle : board.whiteKingCastle);
    uint64_t friendlyPieces = board.colorBoards[color];
    uint64_t enemyPieces = board.colorBoards[!color];

	// queenside
	if (canCastleQueen) {
		if (color) { // black
			if (!((friendlyPieces | enemyPieces) & 0xe00000000000000ULL)) {
				// no pieces blocking
				uint32_t move = 0;
    			Move::setColor(move, color);
                Move::setCastle(move, QUEENSIDE);
                moves.push_back(move);
			}
		} else { // white
			if (!((friendlyPieces | enemyPieces) & 0xeULL)) {
				// no pieces blocking
				uint32_t move = 0;
    			Move::setColor(move, color);
                Move::setCastle(move, QUEENSIDE);
                moves.push_back(move);
			}
		}
	}

	// kingside
	if (canCastleKing) {
		if (color) { // black
			if (!((friendlyPieces | enemyPieces) & 0x6000000000000000ULL)) {
				// no pieces blocking
				uint32_t move = 0;
    			Move::setColor(move, color);
                Move::setCastle(move, KINGSIDE);
                moves.push_back(move);
			}
		} else { // white
			if (!((friendlyPieces | enemyPieces) & 0x60ULL)) {
				// no pieces blocking
				uint32_t move = 0;
    			Move::setColor(move, color);
                Move::setCastle(move, KINGSIDE);
                moves.push_back(move);
			}
		}
	}
}