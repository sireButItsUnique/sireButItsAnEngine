#include "helper.hpp"
void SPLIT_STRING(string s, vector<string>& tokens) {
    size_t pos = 0, found;
    while ((found = s.find_first_of(' ', pos)) != string::npos) {
        tokens.push_back(s.substr(pos, found - pos));
        pos = found + 1;
    }
    if (pos < s.size()) tokens.push_back(s.substr(pos));
}

int TO_SQUARE(int col, int row) {
   return 8 * (row - '1') + 7 - (col - 'a');
}

string TO_ALGEBRA(int square) {
    return string(1, 'a' + (7 - (square % 8))) + string(1, '1' + (square / 8));
}