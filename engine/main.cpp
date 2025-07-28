#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"
#include "Search.hpp"

string cmd;
Board board;
int main() {
    MoveGen::init(); // Initialize ray attacks and lookup tables
    while (cin >> cmd) {
        if (cmd == "quit") return 0;
        
        else if (cmd == "uci") {
            cout << "id name sireButItsAnEngine" << endl;
            cout << "id author Robert Lu" << endl;
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
                    int from = TO_SQUARE(tokens[i][0], tokens[i][1]);
                    int to = TO_SQUARE(tokens[i][2], tokens[i][3]);
                    Move::setColor(move, board.turn);
                    Move::setPosition(move, from, to);

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
            getline(cin, cmd);
            vector<string> tokens;
            SPLIT_STRING(cmd, tokens);

            int depth = 0;
            int wtime = INT32_MAX, btime = INT32_MAX;

            for (int i = 1; i < tokens.size(); ++i) {
                if (tokens[i] == "depth") {
                    depth = stoi(tokens[i + 1]);
                } else if (tokens[i] == "wtime") {
                    wtime = stoi(tokens[i + 1]);
                } else if (tokens[i] == "btime") {
                    btime = stoi(tokens[i + 1]);
                }
            }

            vector<vector<uint32_t>> moveHistory(64, vector<uint32_t>(64, 0));
            Search::count = 0;
            auto start = chrono::high_resolution_clock::now();
            
            int32_t eval = Search::bestMoves(board, depth, -50000, 50000, moveHistory);

            auto end = chrono::high_resolution_clock::now();
            double time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            time_taken *= 1e-9;
            uint32_t move = moveHistory[depth][0];
            for (int i = 0; i <= depth; ++i) {
                cout << "info Depth " << i << ": ";
                for (int j = 0; j < 64; ++j) {
                    if (moveHistory[i][j] != 0) {
                        cout << Move::toAlgebra(moveHistory[i][j]) << " ";
                    }
                }
                cout << endl;
            }

            cout << "info Evaluated " << Search::count << " positions in " << fixed << time_taken << setprecision(4) << " secs (" << (Search::count / time_taken) << " pos/s)" << endl;
            cout << "info Eval: " << eval << endl;
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