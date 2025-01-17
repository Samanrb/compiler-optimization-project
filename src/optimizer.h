#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <string>
#include <vector>
#include "llvm/ADT/StringRef.h"

namespace utils {
    LLVM_READNONE inline bool isWhitespace(char c);
    LLVM_READNONE inline bool isDigit(char c);
    LLVM_READNONE inline bool isLetter(char c);
    LLVM_READNONE inline bool isSemiColon(char c);
    LLVM_READNONE inline bool isEqual(char c);
    std::vector<std::string> split(std::string &s, const std::string &delimiter);
}


class Optimizer {
private:
    std::vector<llvm::StringRef> Lines;
    std::vector<std::string> new_lines;
    std::vector<bool> deadLines;
    std::string code;
    const char *BufferPtr;

    char top(const char *&expr);
    char get(const char *&expr);
    int number(const char *&expr);
    int variable(const char *&expr, int i);
    int factor(const char *&expr, int i);
    int term(const char *&expr, int i);
    int condition(const char *&expr, int i);
    int expression(const char *&expr, int i);
    int evaluateConstant(int j, llvm::StringRef variab);

public:
    Optimizer(const llvm::StringRef &Buffer);
    std::string optimize();
};

#endif // OPTIMIZER_H 