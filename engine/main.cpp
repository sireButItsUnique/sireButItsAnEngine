#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"
#include "Search.hpp"
#include "Hash.hpp"

string cmd;
Board board;
int main(int argc, char *argv[]) {
    MoveGen::init(); // Initialize ray attacks and lookup tables
    Zobrist::init(); // Initialize Zobrist hashing tables

    // Run benchmark
    if (argc == 2 && std::string(argv[1]) == "bench") {
        board.setStartingPos();
        vector<vector<uint32_t>> moveHistory(64, vector<uint32_t>(64, 0));
        Search::initSearch(INFINITE_SCORE);
        Search::MAX_DEPTH = 5;
        auto start = chrono::high_resolution_clock::now();
        
        Search::bestMoves(board, 5, -INFINITE_SCORE, INFINITE_SCORE, moveHistory);

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
        
        else if (cmd == "position") {
            getline(cin, cmd);
            vector<string> tokens;
            SPLIT_STRING(cmd, tokens);
            
            // set initial position
            if (tokens[1] == "startpos") board.setStartingPos();
            else if (tokens[1] == "fen") board.setFenPos(tokens[2], tokens[3], tokens[4], tokens[5]);

            // add moves to position 
            if (tokens[1] == "startpos" && tokens.size() > 2) {

                for (int i = 3; i < tokens.size(); ++i) {
                    uint32_t move = 0;
                    Move::setColor(move, board.turn);

                    if (tokens[i] == "e1g1" && board.castlingRights[WHITE_KINGSIDE]) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e1c1" && board.castlingRights[WHITE_QUEENSIDE]) {
                        Move::setCastle(move, true);
                    } else if (tokens[i] == "e8g8" && board.castlingRights[BLACK_KINGSIDE]) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e8c8" && board.castlingRights[BLACK_QUEENSIDE]) {
                        Move::setCastle(move, true);
                    } else {
                        int from = TO_SQUARE(tokens[i][0], tokens[i][1]);
                        int to = TO_SQUARE(tokens[i][2], tokens[i][3]);
                        Move::setPosition(move, from, to);
                    }

                    if (tokens[i].size() == 5) {
                        switch (tokens[i][4]) {
                            case 'n': Move::setPromotion(move, KNIGHT); break;
                            case 'b': Move::setPromotion(move, BISHOP); break;
                            case 'r': Move::setPromotion(move, ROOK); break;
                            case 'q': Move::setPromotion(move, QUEEN); break;
                        }
                    }

                    board.movePiece(move);
                }
            }
            if (tokens[1] == "fen" && tokens.size() > 8) {
                for (int i = 9; i < tokens.size(); ++i) {
                    uint32_t move = 0;
                    Move::setColor(move, board.turn);

                    if (tokens[i] == "e1g1" && board.castlingRights[WHITE_KINGSIDE]) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e1c1" && board.castlingRights[WHITE_QUEENSIDE]) {
                        Move::setCastle(move, true);
                    } else if (tokens[i] == "e8g8" && board.castlingRights[BLACK_KINGSIDE]) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e8c8" && board.castlingRights[BLACK_QUEENSIDE]) {
                        Move::setCastle(move, true);
                    } else {
                        int from = TO_SQUARE(tokens[i][0], tokens[i][1]);
                        int to = TO_SQUARE(tokens[i][2], tokens[i][3]);
                        Move::setPosition(move, from, to);
                    }

                    if (tokens[i].size() == 5) {
                        switch (tokens[i][4]) {
                            case 'n': Move::setPromotion(move, KNIGHT); break;
                            case 'b': Move::setPromotion(move, BISHOP); break;
                            case 'r': Move::setPromotion(move, ROOK); break;
                            case 'q': Move::setPromotion(move, QUEEN); break;
                        }
                    }

                    board.movePiece(move);
                }
            }
        } 
        
        else if (cmd == "isready") {
            cout << "readyok" << endl;
        } 
        
        else if (cmd == "go") {

            // sorting out the command line arguments
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
            int64_t timeCap = max(int64_t(20), timeLeft / 20);

            // initiating search
            vector<vector<uint32_t>> moveHistory(64, vector<uint32_t>(64, 0));
            int32_t eval = 0;
            uint32_t move = 0;
            auto start = chrono::high_resolution_clock::now();
            
            // static depth search if depth is given
            if (depth != -1) {
                Search::initSearch(INFINITE_SCORE);
                Search::MAX_DEPTH = depth;
                eval = Search::bestMoves(board, depth, -INFINITE_SCORE, INFINITE_SCORE, moveHistory);
                move = moveHistory[depth][0];
            } 
            
            // iterative deepening
            else {
                Search::initSearch(timeCap);
                depth = 0;
                for ( ; depth < 16; ++depth) {
                    Search::MAX_DEPTH = depth;
                    uint32_t tmp = Search::bestMoves(board, depth, -INFINITE_SCORE, INFINITE_SCORE, moveHistory);
                    
                    if (Search::ABORT_SEARCH) break;
                    move = moveHistory[depth][0];
                    eval = tmp;
                    double time_taken = 1e-9 * chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - start).count();
                    cout << "info depth " << depth;
                    cout << " score cp " << eval;
                    cout << " nodes " << Search::NODE_COUNT;
                    cout << " time " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count();
                    cout << " nps " << int64_t(Search::NODE_COUNT / time_taken);
                    cout << " pv ";
                    for (int i = 0; i < 64; ++i) {
                        if (!moveHistory[depth][i]) break;
                        cout << Move::toAlgebra(moveHistory[depth][i]) << " ";
                    }
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
            vector<uint32_t> moves;
            MoveGen::genMoves(board, moves, board.turn);
            cout << "Moves: ";
            for (uint32_t move : moves) {
                cout << Move::toAlgebra(move) << " ";
            }
            cout << endl;
        }
    }
}