#pragma once
#include "includes.hpp"
#include "helper.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "MoveGen.hpp"
#include "Search.hpp"

string cmd;
Board board;
int main() {
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

            if (tokens[1] == "depth") {
                // search to a specific depth
            }

        }

        else if (cmd == "d") {
            board.print();
        }
    }
}