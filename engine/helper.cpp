#include "helper.hpp"
void splitString(string s, vector<string>& tokens) {
    size_t pos = 0, found;
    while ((found = s.find_first_of(' ', pos)) != string::npos) {
        tokens.push_back(s.substr(pos, found - pos));
        pos = found + 1;
    }
    if (pos < s.size()) tokens.push_back(s.substr(pos));
}