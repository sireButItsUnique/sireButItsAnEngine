#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"
#include "Search.hpp"
#include "Hash.hpp"
#include "NNUE.hpp"

string cmd;
Board board;
fast::lvector<uint64_t> threeFoldReps;

int main(int argc, char *argv[]) {
    MoveGen::init(); // Initialize ray attacks and lookup tables
    Zobrist::init(); // Initialize Zobrist hashing tables
    NNUE::init(); // Initialize NNUE network, load weights
    Search::init(); // Initialize search's lookup tables
    TT::init(1ULL << 22); // Initialize transposition table to 4m entries by default

    // Run benchmark
    if (argc == 2 && std::string(argv[1]) == "bench") {
        board.setStartingPos();
        threeFoldReps.clear();
        threeFoldReps.push_back(board.key);
        vector<vector<uint32_t>> moveHistory(64, vector<uint32_t>(64, 0));
        Search::initSearch(INFINITE_SCORE, threeFoldReps);
        auto start = chrono::high_resolution_clock::now();

        for (int depth = 1; depth <= 10; ++depth) {
            Search::MAX_DEPTH = depth;
            Search::bestMoves(board, depth, 0, -INFINITE_SCORE, INFINITE_SCORE, moveHistory, true);
        }

        // outputing the results
        auto end = chrono::high_resolution_clock::now();
        double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
        time_taken *= 1e-9;
        cout << Search::NODE_COUNT << " nodes " << fixed << setprecision(2) << int64_t(Search::NODE_COUNT / time_taken) << " nps" << std::endl;
        return 0;
    }

    // UCI loop
    while (cin >> cmd) {
        if (cmd == "quit") return 0;
        
        else if (cmd == "uci") {
            cout << "id name sireButItsAnEngine" << endl;
            cout << "id author sireButItsUnique" << endl;
            cout << "option name Hash type spin default 1 min 1 max 1" << endl;
            cout << "option name Threads type spin default 1 min 1 max 1" << endl;
            cout << "uciok" << endl;
        } 

        else if (cmd == "ucinewgame") {
            board.setStartingPos();
            threeFoldReps.clear();
            threeFoldReps.push_back(board.key);
            TT::init(TT::TT_SIZE); // clears tt but keeps size
        }

        else if (cmd == "setoption") {
            getline(cin, cmd);
            vector<string> tokens;
            SPLIT_STRING(cmd, tokens);
            if (tokens[2] == "Hash") {
                uint64_t size = stoi(tokens[4]); // size in MB
                uint64_t maxEntries = (size << 20) / sizeof(TTEntry);

                // max amount of entry that fits in given size
                int n = 1;
                while ((1ULL << n) <= maxEntries) n++;
                n--; 

                TT::init(1ULL << n);
            } else if (tokens[2] == "Threads") {
                // ignore for now since we are only using 1 thread
            }
        }
        
        else if (cmd == "position") {
            getline(cin, cmd);
            vector<string> tokens;
            SPLIT_STRING(cmd, tokens);
            
            // set initial position
            if (tokens[1] == "startpos") board.setStartingPos();
            else if (tokens[1] == "fen") board.setFenPos(tokens[2], tokens[3], tokens[4], tokens[5]);
            threeFoldReps.clear();
            threeFoldReps.push_back(board.key);

            // add moves to position 
            if (tokens[1] == "startpos" && tokens.size() > 2) {

                for (int i = 3; i < tokens.size(); ++i) {
                    uint32_t move = 0;
                    int from = TO_SQUARE(tokens[i][0], tokens[i][1]);
                    int to = TO_SQUARE(tokens[i][2], tokens[i][3]);
                    Move::setColor(move, board.turn);

                    // Castling
                    if (tokens[i] == "e1g1" && board.castlingRights[WHITE_KINGSIDE]) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e1c1" && board.castlingRights[WHITE_QUEENSIDE]) {
                        Move::setCastle(move, true);
                    } else if (tokens[i] == "e8g8" && board.castlingRights[BLACK_KINGSIDE]) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e8c8" && board.castlingRights[BLACK_QUEENSIDE]) {
                        Move::setCastle(move, true);
                    } 
                    
                    // Normal move
                    else {
                        Move::setPosition(move, from, to);
                    }

                    // Promotion
                    if (tokens[i].size() == 5) {
                        switch (tokens[i][4]) {
                            case 'n': Move::setPromotion(move, KNIGHT); break;
                            case 'b': Move::setPromotion(move, BISHOP); break;
                            case 'r': Move::setPromotion(move, ROOK); break;
                            case 'q': Move::setPromotion(move, QUEEN); break;
                        }
                    }

                    // En passant
                    if (
                        board.pieceBoards[PAWN + board.turn] & (1ULL << from) // is pawn move
                        && tokens[i][0] != tokens[i][2] // is capture move
                        && !(board.colorBoards[!board.turn] & (1ULL << to)) // target square is empty
                    ) {
                        Move::setEnpassant(move);
                    }

                    board.movePiece(move);
                    threeFoldReps.push_back(board.key);
                }
            }
            if (tokens[1] == "fen" && tokens.size() > 8) {
                for (int i = 9; i < tokens.size(); ++i) {
                    uint32_t move = 0;
                    int from = TO_SQUARE(tokens[i][0], tokens[i][1]);
                    int to = TO_SQUARE(tokens[i][2], tokens[i][3]);
                    Move::setColor(move, board.turn);

                    // Castling
                    if (tokens[i] == "e1g1" && board.castlingRights[WHITE_KINGSIDE]) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e1c1" && board.castlingRights[WHITE_QUEENSIDE]) {
                        Move::setCastle(move, true);
                    } else if (tokens[i] == "e8g8" && board.castlingRights[BLACK_KINGSIDE]) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e8c8" && board.castlingRights[BLACK_QUEENSIDE]) {
                        Move::setCastle(move, true);
                    } 
                    
                    // Normal move
                    else {
                        Move::setPosition(move, from, to);
                    }

                    // Promotion
                    if (tokens[i].size() == 5) {
                        switch (tokens[i][4]) {
                            case 'n': Move::setPromotion(move, KNIGHT); break;
                            case 'b': Move::setPromotion(move, BISHOP); break;
                            case 'r': Move::setPromotion(move, ROOK); break;
                            case 'q': Move::setPromotion(move, QUEEN); break;
                        }
                    }

                    // En passant
                    if (
                        board.pieceBoards[PAWN + board.turn] & (1ULL << from) // is pawn move
                        && tokens[i][0] != tokens[i][2] // is capture move
                        && !(board.colorBoards[!board.turn] & (1ULL << to)) // target square is empty
                    ) {
                        Move::setEnpassant(move);
                    }

                    board.movePiece(move);
                    threeFoldReps.push_back(board.key);
                }
            }
        } 
        
        else if (cmd == "isready") {
            cout << "readyok" << endl;
        } 
        
        else if (cmd == "go") {

            // Sorting out the command line arguments
            getline(cin, cmd);
            vector<string> tokens;
            SPLIT_STRING(cmd, tokens);

            int depth = -1;
            int64_t wtime = 60 * 1000, btime = 60 * 1000; 

            for (int i = 1; i < tokens.size(); ++i) {
                if (tokens[i] == "depth") {
                    depth = stoi(tokens[i + 1]);
                } else if (tokens[i] == "wtime") {
                    wtime = stoi(tokens[i + 1]);
                } else if (tokens[i] == "btime") {
                    btime = stoi(tokens[i + 1]);
                }
            }

            int64_t timeLeft = (board.turn == WHITE) ? wtime : btime;
            int64_t timeCap = timeLeft / 20;
            timeCap = min(timeCap, int64_t(5000)); // Cap at 5 seconds

            // initiating search
            vector<vector<uint32_t>> moveHistory(64, vector<uint32_t>(64, 0));
            int32_t eval = 0;
            uint32_t move = 0;
            auto start = chrono::high_resolution_clock::now();
            
            // static depth search if depth is given
            if (depth != -1) {
                Search::initSearch(INFINITE_SCORE, threeFoldReps);
                Search::MAX_DEPTH = depth;
                eval = Search::bestMoves(board, depth, 0, -INFINITE_SCORE, INFINITE_SCORE, moveHistory, true);
                move = moveHistory[depth][0];
            } 
            
            // iterative deepening
            else {
                Search::initSearch(timeCap, threeFoldReps);
                depth = 1;
                for ( ; depth < 16; ++depth) {

                    // set search depth
                    Search::MAX_DEPTH = depth;
                    
                    // set up aspiration window
                    int32_t alpha = -INFINITE_SCORE;
                    int32_t beta = INFINITE_SCORE;  
                    int32_t windowSize = 50; // aspiration window size
                    if (depth >= 3) { // only use aspiration windows from depth 3 onwards
                        alpha = eval - windowSize;
                        beta = eval + windowSize;
                    }

                    // aspiration window search
                    int32_t tmpEval;
                    while (true) {
                        alpha = max(alpha, -INFINITE_SCORE);
                        beta = min(beta, INFINITE_SCORE);

                        // call actual search
                        tmpEval = Search::bestMoves(board, depth, 0, alpha, beta, moveHistory, true);
                        if (Search::ABORT_SEARCH) break;

                        // adjust window if out of bounds
                        if (tmpEval <= alpha) {
                            windowSize *= 2;
                            alpha = tmpEval - windowSize;
                        } else if (tmpEval >= beta) {
                            windowSize *= 2;
                            beta = tmpEval + windowSize;
                        } else {
                            break; // within bounds, move on to next depth
                        }
                    }
                    if (Search::ABORT_SEARCH) break;
                    
                    
                    // organize info post search
                    move = moveHistory[depth][0];
                    eval = tmpEval;
                    double time_taken = 1e-9 * chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - start).count();
                    cout << "info depth " << depth;
                    cout << " score cp " << eval;
                    cout << " nodes " << Search::NODE_COUNT;
                    cout << " time " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count();
                    cout << " nps " << int64_t(Search::NODE_COUNT / time_taken);
                    cout << " pv " << Move::toAlgebra(moveHistory[depth][0]); // pv broken for now bc tt exits
                    // for (int i = 0; i < 64; ++i) {
                    //     if (!moveHistory[depth][i]) break;
                    //     cout << Move::toAlgebra(moveHistory[depth][i]) << " ";
                    // }
                    cout << endl;
                }
                depth--;
            }
            

            // outputing the results
            auto end = chrono::high_resolution_clock::now();
            int64_t time_taken = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            cout << "bestmove " << Move::toAlgebra(move) << endl;
        }

        else if (cmd == "d") {
            board.print();
            fast::vector<uint32_t> moves;
            MoveGen::genMoves(board, moves, board.turn);
            cout << "Moves: ";
            for (uint32_t move : moves) {
                cout << Move::toAlgebra(move) << " ";
            }
            cout << endl << "Static: " << NNUE::evalBoard(board) << endl;
        }
    }
}