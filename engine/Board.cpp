#include "Board.hpp"

Board::Board() {
    this->setStartingPos();
}

void Board::setCopyBoard(Board* board) {
    for (int i = 0; i < 12; ++i) {
        pieceBoards[i] = board->pieceBoards[i];
    }
    colorBoards[WHITE] = board->colorBoards[WHITE];
    colorBoards[BLACK] = board->colorBoards[BLACK];
    whiteQueenCastle = board->whiteQueenCastle;
    whiteKingCastle = board->whiteKingCastle;
    blackQueenCastle = board->blackQueenCastle;
    blackKingCastle = board->blackKingCastle;
    turn = board->turn;
}

void Board::setStartingPos() {

    // Initialize pieceBoards to the standard starting position
    pieceBoards[PAWN + WHITE]   = 0x000000000000FF00; 
    pieceBoards[PAWN + BLACK]   = 0x00FF000000000000; 
    pieceBoards[KNIGHT + WHITE] = 0x0000000000000042;
    pieceBoards[KNIGHT + BLACK] = 0x4200000000000000;
    pieceBoards[BISHOP + WHITE] = 0x0000000000000024;
    pieceBoards[BISHOP + BLACK] = 0x2400000000000000;
    pieceBoards[ROOK + WHITE]   = 0x0000000000000081;
    pieceBoards[ROOK + BLACK]   = 0x8100000000000000;
    pieceBoards[QUEEN + WHITE]  = 0x0000000000000008;
    pieceBoards[QUEEN + BLACK]  = 0x0800000000000000;
    pieceBoards[KING + WHITE]   = 0x0000000000000010;
    pieceBoards[KING + BLACK]   = 0x1000000000000000;

    // Set color boards
    colorBoards[WHITE] = 0x000000000000FFFF;
    colorBoards[BLACK] = 0xFFFF000000000000; // Black pieces

    whiteQueenCastle = true;
    whiteKingCastle = true;
    blackQueenCastle = true;
    blackKingCastle = true;

    turn = WHITE; // White starts
}

void Board::setFenPos(string fen) {
    // TODO: Implement FEN parsing logic
}

void Board::movePiece(uint32_t move) {
    this->turn = !this->turn; // Switch turn

    bool color = Move::color(move);
    uint8_t from = Move::from(move);
    uint8_t to = Move::to(move);

    // Handle castling
    if (Move::isCastle(move)) {
        if (Move::castleSide(move) == QUEENSIDE) {
            pieceBoards[ROOK + color] ^= (0x9ULL << (color * 56)); // Move rook from a-file
            pieceBoards[KING + color] ^= (0x14ULL << (color * 56)); // Move king
            colorBoards[color] ^= (0x9ULL << (color * 56));
            colorBoards[color] ^= (0x14ULL << (color * 56));
        } else { // Kingside
            pieceBoards[ROOK + color] ^= (0xa0ULL << (color * 56)); // Move rook from h-file
            pieceBoards[KING + color] ^= (0x50ULL << (color * 56)); // Move king
            colorBoards[color] ^= (0xa0ULL << (color * 56));
            colorBoards[color] ^= (0x50ULL << (color * 56));
        }
        if (color) {
            blackQueenCastle = false;
            blackKingCastle = false;
        } else {
            whiteQueenCastle = false;
            whiteKingCastle = false;
        }
        return;
    }

    // Castling Shenanigans
    if (from == 0 || to == 0) whiteQueenCastle = false;
    if (from == 7 || to == 7) whiteKingCastle = false;
    if (from == 56 || to == 56) blackQueenCastle = false;
    if (from == 63 || to == 63) blackKingCastle = false;
    if (from == 4) {
        whiteKingCastle = false;
        whiteQueenCastle = false;
    }
    if (from == 60) {
        blackKingCastle = false;
        blackQueenCastle = false;
    }

    // Normal move
    for (int i = 0; i < 12; i += 2) {
        if (pieceBoards[i + color] & (1ULL << from)) {
            pieceBoards[i + color] &= ~(1ULL << from); // Remove piece from old square
            pieceBoards[i + color] |= (1ULL << to); // Place piece on new square
            break;
        }
    }
    for (int i = 0; i < 12; i += 2) {
        pieceBoards[i + !color] &= ~(1ULL << to); // Remove enemy piece on new square
    }
    colorBoards[color] &= ~(1ULL << from); // Remove piece from old square
    colorBoards[color] |= (1ULL << to); // Place piece on new square
    colorBoards[!color] &= ~(1ULL << to); // Remove enemy piece on new square
    
    // Handle promotion
    if (Move::isPromotion(move)) {
        pieceBoards[Move::promotionPiece(move)] |= (1ULL << to); // Add promoted piece
        pieceBoards[PAWN + color] &= ~(1ULL << to); // Remove pawn
    }
}

void Board::print() {
    cout << "+---+---+---+---+---+---+---+---+" << endl;
    for (int row = 7; row >= 0; row--) {
        cout << "|";
        for (int col = 0; col < 8; col++) {
            int square = TO_SQUARE(col + 'a', row + '1');
            char piece = ' ';
            for (int i = 0; i < 12; i++) {
                if (pieceBoards[i] & (1ULL << square)) {
                    piece = "PpNnBbRrQqKk"[i];
                    break;
                }
            }
            cout << " " << piece << " |";
        }
        cout << " " << row + 1 << endl;
        cout << "+---+---+---+---+---+---+---+---+" << endl;
    }
    cout << "  a   b   c   d   e   f   g   h" << endl;
    cout << "Turn: " << (turn ? "Black" : "White") << endl;
}