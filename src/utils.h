#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace utils {
    bool isWhitespace(char c);
    bool isDigit(char c);
    bool isLetter(char c);
    bool isSemiColon(char c);
    bool isEqual(char c);
    std::vector<std::string> split(std::string &s, const std::string &delimiter);
}

#endif // UTILS_H 