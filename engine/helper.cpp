#include "helper.hpp"
void SPLIT_STRING(string s, vector<string>& tokens) {
    size_t pos = 0, found;
    while ((found = s.find_first_of(' ', pos)) != string::npos) {
        tokens.push_back(s.substr(pos, found - pos));
        pos = found + 1;
    }
    if (pos < s.size()) tokens.push_back(s.substr(pos));
}

uint64_t RAND_64() {
    uint64_t res = rand();
    res <<= 32;
    res |= rand();
    return res;
}

int TO_SQUARE(int col, int row) {
    return 8 * (row - '1') + (col - 'a');
}

string TO_ALGEBRA(int square) {
    return string(1, 'a' + (square % 8)) + string(1, '1' + (square / 8));
}

int FENIDX_TO_SQUARE(int idx) {
    int row = 7 - (idx / 8);
    int col = idx % 8;
    return TO_SQUARE(col + 'a', row + '1');
}