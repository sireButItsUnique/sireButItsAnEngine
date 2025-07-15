#pragma once
#include "includes.hpp"
#include "helper.hpp"

string cmd;
int main() {
    while (cin >> cmd) {
        if (cmd == "quit") return 0;
        else if (cmd == "uci") {
            cout << "id name sireButItsAnEngine" << endl;
            cout << "id author Robert Lu" << endl;
            cout << "uciok" << endl;
        } else if (cmd == "position") {
            getline(cin, cmd);
            vector<string> tokens;
            SPLIT_STRING(cmd, tokens);
            
            if (tokens[1] == "startpos") {
                // set position to start position
            } else if (tokens[1] == "fen") {
                // set position to fen position
            }

            if ((tokens[1] == "startpos" && tokens.size() > 2) || (tokens[1] == "fen" && tokens.size() > 8)) {
                // add moves to position 
            }
        } else if (cmd == "isready") {
            cout << "readyok" << endl;
        } else if (cmd == "go") {
            getline(cin, cmd);
            vector<string> tokens;
            SPLIT_STRING(cmd, tokens);

            if (tokens[1] == "depth") {
                // search to a specific depth
            }

        }
    }
}