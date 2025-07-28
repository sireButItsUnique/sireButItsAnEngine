#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"
#include "Search.hpp"

string cmd;
Board board;
int main(int argc, char *argv[]) {
    MoveGen::init(); // Initialize ray attacks and lookup tables

    // Run benchmark
    if (argc == 2 && std::string(argv[1]) == "bench") {
        board.setStartingPos();
        vector<vector<uint32_t>> moveHistory(64, vector<uint32_t>(64, 0));
        Search::count = 0;
        auto start = chrono::high_resolution_clock::now();
        
        Search::bestMoves(board, 5, -INFINITE_SCORE, INFINITE_SCORE, moveHistory);

        // outputing the results
        auto end = chrono::high_resolution_clock::now();
        double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
        time_taken *= 1e-9;
        cout << Search::count << " nodes " << fixed << setprecision(2) << (Search::count / time_taken) << " nps" << std::endl;
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
            else if (tokens[1] == "fen") board.setFenPos(tokens[2]);

            // add moves to position 
            if (tokens[1] == "startpos" && tokens.size() > 2) {

                for (int i = 3; i < tokens.size(); ++i) {
                    uint32_t move = 0;
                    Move::setColor(move, board.turn);

                    if (tokens[i] == "e1g1" && board.whiteKingCastle) {
                        Move::setCastle(move, true);
                    } else if (tokens[i] == "e1c1" && board.whiteQueenCastle) {
                        Move::setCastle(move, false);
                    } else if (tokens[i] == "e8g8" && board.blackKingCastle) {
                        Move::setCastle(move, true);
                    } else if (tokens[i] == "e8c8" && board.blackQueenCastle) {
                        Move::setCastle(move, false);
                    } else {
                        int from = TO_SQUARE(tokens[i][0], tokens[i][1]);
                        int to = TO_SQUARE(tokens[i][2], tokens[i][3]);
                        Move::setPosition(move, from, to);
                    }

                    board.movePiece(move);
                }
            }
            if (tokens[1] == "fen" && tokens.size() > 8) {

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

            int64_t timeLimit = (board.turn == WHITE) ? wtime : btime;
            if (depth == -1) {
                if (timeLimit < 60 * 1000) depth = 2;
                else depth = 3;
            }

            // initiating search
            vector<vector<uint32_t>> moveHistory(64, vector<uint32_t>(64, 0));
            Search::count = 0;
            auto start = chrono::high_resolution_clock::now();
            
            int32_t eval = Search::bestMoves(board, depth, -INFINITE_SCORE, INFINITE_SCORE, moveHistory);

            // outputing the results
            auto end = chrono::high_resolution_clock::now();
            double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            time_taken *= 1e-9;
            uint32_t move = moveHistory[depth][0];
            for (int i = 0; i <= depth; ++i) {
                cout << "info string Depth " << i << ": ";
                for (int j = 0; j < 64; ++j) {
                    if (moveHistory[i][j] != 0) {
                        cout << Move::toAlgebra(moveHistory[i][j]) << " ";
                    }
                }
                cout << endl;
            }

            cout << "info string Evaluated " << Search::count << " positions in " << fixed << time_taken << setprecision(4) << " secs (" << (Search::count / time_taken) << " pos/s)" << endl;
            cout << "info string Eval: " << eval << endl;
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