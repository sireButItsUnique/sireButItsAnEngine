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
            splitString(cmd, tokens);
            cout << tokens.size() << " tokens received." << endl;
        }
    }
}