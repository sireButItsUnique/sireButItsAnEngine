#include "Board.hpp"

Board::Board() {
    this->setStartingPos();
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

    // Initialize mailbox
    for (int i = 0; i < 64; ++i) {
        mailbox[i] = 12; // Initialize mailbox to empty
        for (int j = 0; j < 12; j++) {
            if (pieceBoards[j] & (1ULL << i)) mailbox[i] = j;
        }
    }

    // Set castling rights
    for (int i = 0; i < 4; ++i) {
        castlingRights[i] = true;
    }

    turn = WHITE; // White starts

    // Gen zobrist key
    this->key = this->getZobristKey();
}

void Board::setFenPos(string pos, string turn, string castling, string enPassant) {
    
    // Set board position
    for (int i = 0; i < 12; ++i) pieceBoards[i] = 0;
    colorBoards[WHITE] = 0;
    colorBoards[BLACK] = 0;
    int square = 0;
    for (char c : pos) {
        if (c >= '1' && c <= '8') {
            square = min(square + (c - '0'), 8 * (square / 8) + 7); // Clamp to row
        } else if (c == '/') {
            square++;
        } else {
            // Set piece on board
            int piece = 0;
            switch (c) {
                case 'P': piece = PAWN + WHITE; break;
                case 'p': piece = PAWN + BLACK; break;
                case 'N': piece = KNIGHT + WHITE; break;
                case 'n': piece = KNIGHT + BLACK; break;
                case 'B': piece = BISHOP + WHITE; break;
                case 'b': piece = BISHOP + BLACK; break;
                case 'R': piece = ROOK + WHITE; break;
                case 'r': piece = ROOK + BLACK; break;
                case 'Q': piece = QUEEN + WHITE; break;
                case 'q': piece = QUEEN + BLACK; break;
                case 'K': piece = KING + WHITE; break;
                case 'k': piece = KING + BLACK; break;
            }
            pieceBoards[piece] |= (1ULL << FENIDX_TO_SQUARE(square));
            square = min(square + 1, 8 * (square / 8) + 7); // Clamp to row
        }
    }
    for (int i = 0; i < 12; ++i) {
        colorBoards[i % 2] |= pieceBoards[i]; // Update color boards
    }

    // Initialize mailbox
    for (int i = 0; i < 64; ++i) {
        mailbox[i] = 12; // Initialize mailbox to empty
        for (int j = 0; j < 12; j++) {
            if (pieceBoards[j] & (1ULL << i)) mailbox[i] = j;
        }
    }

    // Set turn
    if (turn == "w") this->turn = WHITE;
    else this->turn = BLACK;

    // Set castling rights
    for (int i = 0; i < 4; ++i) {
        castlingRights[i] = false;
    }

    for (char c : castling) {
        if (c == 'K') castlingRights[0] = true;
        else if (c == 'Q') castlingRights[1] = true;
        else if (c == 'k') castlingRights[2] = true;
        else if (c == 'q') castlingRights[3] = true;
    }

    // Gen zobrist key
    this->key = this->getZobristKey();

    // TODO: En Passant not implemented yet
}

void Board::movePiece(uint32_t move) {
    this->turn = !this->turn; // Switch turn

    bool color = Move::color(move);
    uint8_t from = Move::from(move);
    uint8_t to = Move::to(move);

    // Update zobrist key
    key ^= Zobrist::TURN[!color];
    key ^= Zobrist::TURN[color];

    // Handle castling
    if (Move::isCastle(move)) {
        if (Move::castleSide(move) == QUEENSIDE) {
            pieceBoards[ROOK + color] ^= (0x9ULL << (color * 56)); // Move rook from a-file
            pieceBoards[KING + color] ^= (0x14ULL << (color * 56)); // Move king
            colorBoards[color] ^= (0x9ULL << (color * 56));
            colorBoards[color] ^= (0x14ULL << (color * 56));

            // Update mailbox & zobrist keys
            if (color) {
                mailbox[56] = 12;
                mailbox[57] = 12;
                mailbox[58] = KING + BLACK;
                mailbox[59] = ROOK + BLACK;
                mailbox[60] = 12;
                key ^= Zobrist::PIECES[ROOK + BLACK][56];
                key ^= Zobrist::PIECES[ROOK + BLACK][59];
                key ^= Zobrist::PIECES[KING + BLACK][60];
                key ^= Zobrist::PIECES[KING + BLACK][58];

                key ^= Zobrist::CASTLING[BLACK_QUEENSIDE + 4];
                key ^= Zobrist::CASTLING[BLACK_QUEENSIDE];
                if (castlingRights[BLACK_KINGSIDE]) {
                    key ^= Zobrist::CASTLING[BLACK_KINGSIDE + 4];
                    key ^= Zobrist::CASTLING[BLACK_KINGSIDE];
                }
            } else {
                mailbox[0] = 12;
                mailbox[1] = 12;
                mailbox[2] = KING + WHITE;
                mailbox[3] = ROOK + WHITE;
                mailbox[4] = 12;
                key ^= Zobrist::PIECES[ROOK + WHITE][0];
                key ^= Zobrist::PIECES[ROOK + WHITE][3];
                key ^= Zobrist::PIECES[KING + WHITE][4];
                key ^= Zobrist::PIECES[KING + WHITE][2];

                key ^= Zobrist::CASTLING[WHITE_QUEENSIDE + 4];
                key ^= Zobrist::CASTLING[WHITE_QUEENSIDE];
                if (castlingRights[WHITE_KINGSIDE]) {
                    key ^= Zobrist::CASTLING[WHITE_KINGSIDE + 4];
                    key ^= Zobrist::CASTLING[WHITE_KINGSIDE];
                }
            }

        } else { // Kingside
            pieceBoards[ROOK + color] ^= (0xa0ULL << (color * 56)); // Move rook from h-file
            pieceBoards[KING + color] ^= (0x50ULL << (color * 56)); // Move king
            colorBoards[color] ^= (0xa0ULL << (color * 56));
            colorBoards[color] ^= (0x50ULL << (color * 56));

            // Update mailbox & zobrist key
            if (color) {
                mailbox[63] = 12;
                mailbox[62] = KING + BLACK;
                mailbox[61] = ROOK + BLACK;
                mailbox[60] = 12;
                key ^= Zobrist::PIECES[ROOK + BLACK][63];
                key ^= Zobrist::PIECES[ROOK + BLACK][61];
                key ^= Zobrist::PIECES[KING + BLACK][60];
                key ^= Zobrist::PIECES[KING + BLACK][62];

                key ^= Zobrist::CASTLING[BLACK_KINGSIDE + 4];
                key ^= Zobrist::CASTLING[BLACK_KINGSIDE];
                if (castlingRights[BLACK_QUEENSIDE]) {
                    key ^= Zobrist::CASTLING[BLACK_QUEENSIDE + 4];
                    key ^= Zobrist::CASTLING[BLACK_QUEENSIDE];
                }
            } else {
                mailbox[7] = 12;
                mailbox[6] = KING + WHITE;
                mailbox[5] = ROOK + WHITE;
                mailbox[4] = 12;
                key ^= Zobrist::PIECES[ROOK + WHITE][7];
                key ^= Zobrist::PIECES[ROOK + WHITE][5];
                key ^= Zobrist::PIECES[KING + WHITE][4];
                key ^= Zobrist::PIECES[KING + WHITE][6];

                key ^= Zobrist::CASTLING[WHITE_KINGSIDE + 4];
                key ^= Zobrist::CASTLING[WHITE_KINGSIDE];
                if (castlingRights[WHITE_QUEENSIDE]) {
                    key ^= Zobrist::CASTLING[WHITE_QUEENSIDE + 4];
                    key ^= Zobrist::CASTLING[WHITE_QUEENSIDE];
                }
            }
        }
        if (color) {
            castlingRights[BLACK_QUEENSIDE] = false;
            castlingRights[BLACK_KINGSIDE] = false;
        } else {
            castlingRights[WHITE_QUEENSIDE] = false;
            castlingRights[WHITE_KINGSIDE] = false;
        }
        return;
    }

    // Castling Shenanigans
    if (from == 0 || to == 0) {
        if (castlingRights[WHITE_QUEENSIDE]) {
            key ^= Zobrist::CASTLING[WHITE_QUEENSIDE + 4];
            key ^= Zobrist::CASTLING[WHITE_QUEENSIDE];
        }
        castlingRights[WHITE_QUEENSIDE] = false;
    }
    if (from == 7 || to == 7) {
        if (castlingRights[WHITE_KINGSIDE]) {
            key ^= Zobrist::CASTLING[WHITE_KINGSIDE + 4];
            key ^= Zobrist::CASTLING[WHITE_KINGSIDE];
        }
        castlingRights[WHITE_KINGSIDE] = false;
    }
    if (from == 56 || to == 56) {
        if (castlingRights[BLACK_QUEENSIDE]) {
            key ^= Zobrist::CASTLING[BLACK_QUEENSIDE + 4];
            key ^= Zobrist::CASTLING[BLACK_QUEENSIDE];
        }
        castlingRights[BLACK_QUEENSIDE] = false;
    }
    if (from == 63 || to == 63) {
        if (castlingRights[BLACK_KINGSIDE]) {
            key ^= Zobrist::CASTLING[BLACK_KINGSIDE + 4];
            key ^= Zobrist::CASTLING[BLACK_KINGSIDE];
        }
        castlingRights[BLACK_KINGSIDE] = false;
    }
    if (from == 4) {
        if (castlingRights[WHITE_QUEENSIDE]) {
            key ^= Zobrist::CASTLING[WHITE_QUEENSIDE + 4];
            key ^= Zobrist::CASTLING[WHITE_QUEENSIDE];
        }
        if (castlingRights[WHITE_KINGSIDE]) {
            key ^= Zobrist::CASTLING[WHITE_KINGSIDE + 4];
            key ^= Zobrist::CASTLING[WHITE_KINGSIDE];
        }
        castlingRights[WHITE_KINGSIDE] = false;
        castlingRights[WHITE_QUEENSIDE] = false;
    }
    if (from == 60) {
        if (castlingRights[BLACK_QUEENSIDE]) {
            key ^= Zobrist::CASTLING[BLACK_QUEENSIDE + 4];
            key ^= Zobrist::CASTLING[BLACK_QUEENSIDE];
        }
        if (castlingRights[BLACK_KINGSIDE]) {
            key ^= Zobrist::CASTLING[BLACK_KINGSIDE + 4];
            key ^= Zobrist::CASTLING[BLACK_KINGSIDE];
        }
        castlingRights[BLACK_KINGSIDE] = false;
        castlingRights[BLACK_QUEENSIDE] = false;
    }

    // Normal move
    for (int i = 0; i < 12; i += 2) {
        if (pieceBoards[i + color] & (1ULL << from)) {
            pieceBoards[i + color] &= ~(1ULL << from); // Remove piece from old square
            pieceBoards[i + color] |= (1ULL << to); // Place piece on new square
            
            // Update mailbox
            mailbox[from] = 12;
            mailbox[to] = i + color;

            // Update zobrist key
            key ^= Zobrist::PIECES[i + color][from];
            key ^= Zobrist::PIECES[i + color][to];
            break;
        }
    }
    for (int i = 0; i < 12; i += 2) {
        if (pieceBoards[i + !color] & (1ULL << to)) {
            pieceBoards[i + !color] &= ~(1ULL << to); // Remove enemy piece on new square
            key ^= Zobrist::PIECES[i + !color][to];
        }
    }
    colorBoards[color] &= ~(1ULL << from); // Remove piece from old square
    colorBoards[color] |= (1ULL << to); // Place piece on new square
    colorBoards[!color] &= ~(1ULL << to); // Remove enemy piece on new square
    
    // Handle promotion
    if (Move::isPromotion(move)) {
        pieceBoards[Move::promotionPiece(move)] |= (1ULL << to); // Add promoted piece
        pieceBoards[PAWN + color] &= ~(1ULL << to); // Remove pawn

        // Update mailbox
        mailbox[to] = Move::promotionPiece(move);

        // Update zobrist key
        key ^= Zobrist::PIECES[PAWN + color][to];
        key ^= Zobrist::PIECES[Move::promotionPiece(move)][to];
    }
}

uint64_t Board::getZobristKey() {
    uint64_t key = 0;
    for (int i = 0; i < 12; ++i) {
        if (pieceBoards[i]) {
            uint64_t board = pieceBoards[i];
            while (board) {
                key ^= Zobrist::PIECES[i][_tzcnt_u64(board)];
                board &= (board - 1); 
            }
        }
    }
    
    key ^= Zobrist::TURN[turn];
    key ^= Zobrist::CASTLING[WHITE_KINGSIDE + (4 * castlingRights[WHITE_KINGSIDE])];
    key ^= Zobrist::CASTLING[WHITE_QUEENSIDE + (4 * castlingRights[WHITE_QUEENSIDE])];
    key ^= Zobrist::CASTLING[BLACK_KINGSIDE + (4 * castlingRights[BLACK_KINGSIDE])];
    key ^= Zobrist::CASTLING[BLACK_QUEENSIDE + (4 * castlingRights[BLACK_QUEENSIDE])];
    return key;
}

uint64_t Board::checkKey() {
    uint64_t computedKey = this->getZobristKey();
    if (computedKey != this->key) {
        cerr << "Computed: " << hex << computedKey << " Stored: " << this->key << dec << endl;
    }
    return computedKey;
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
    cout << "Key: " << hex << getZobristKey() << dec << endl;
}