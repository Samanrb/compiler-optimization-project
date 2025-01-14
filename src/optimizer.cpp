#include "optimizer.h"
#include "utils.h"
#include <iostream>

// Constructor: Initialize optimizer with input buffer
// - Splits the input code into lines and prepares for optimization
// - Stores each line until semicolon in Lines vector
// - Initializes tracking vectors for dead code elimination and optimized lines
Optimizer::Optimizer(const llvm::StringRef &Buffer) {
    BufferPtr = Buffer.begin();
    const char *end = BufferPtr + 1;
    while (*end) {
        end++;
    }
    llvm::StringRef Context(BufferPtr, end - BufferPtr);
    code = (std::string)Context;

    const char *pointer = BufferPtr;
    const char *line_start = BufferPtr;
    while (*pointer) {
        while (!utils::isSemiColon(*pointer)) {
            ++pointer;
        }
        llvm::StringRef Context(line_start, pointer - line_start);
        Lines.push_back(Context);
        deadLines.push_back(true);
        new_lines.push_back("");
        line_start = ++pointer;
    }
}

// Parser helper functions
// top(): Peek at current character without advancing position
// Useful for lookahead during parsing
char Optimizer::top(const char *&expr) {
    return *expr;
}

// get(): Read and advance to next character
// Advances the pointer position after reading
char Optimizer::get(const char *&expr) {
    return *expr++;
}

// Parse numeric values from the expression
// - Handles multi-digit numbers
// - Skips trailing whitespace
// Returns: The parsed integer value
int Optimizer::number(const char *&expr) {
    int result = get(expr) - '0';
    while (top(expr) >= '0' && top(expr) <= '9') {
        result = 10 * result + get(expr) - '0';
    }
    while (top(expr) == ' ')
        get(expr);
    return result;
}

// Handle variable references and constant propagation
// - Processes variable names (letters and digits)
// - Handles boolean literals (true/false)
// - Performs constant propagation for variables
// Parameters:
//   expr: Current position in expression
//   i: Current line number being processed
// Returns: The evaluated value of the variable
int Optimizer::variable(const char *&expr, int i) {
    const char *temp = expr;
    while (utils::isLetter(top(expr)) || utils::isDigit(top(expr))) {
        get(expr);
    }
    llvm::StringRef name(temp, expr - temp);
    while (top(expr) == ' ')
        get(expr);
    if (name == "true")
        return 1;
    else if (name == "false")
        return 0;
    return evaluateConstant(i, name);
}

// Parse factors in the expression grammar
// Handles:
// - Numbers
// - Parenthesized expressions
// - Unary negation
// - Variables
// Returns: The evaluated value of the factor
int Optimizer::factor(const char *&expr, int i) {
    while (top(expr) == ' ')
        get(expr);
    if (top(expr) >= '0' && top(expr) <= '9')
        return number(expr);
    else if (top(expr) == '(') {
        get(expr);
        while (top(expr) == ' ')
            get(expr);
        int result = expression(expr, i);
        while (top(expr) == ' ')
            get(expr);
        get(expr);
        while (top(expr) == ' ')
            get(expr);
        return result;
    }
    else if (top(expr) == '-') {
        get(expr);
        return -factor(expr, i);
    }
    else if (utils::isLetter(top(expr))) {
        return variable(expr, i);
    }
    return 0;
}

// Parse and evaluate multiplication/division terms
// - Processes sequences of * and / operations
// - Maintains operator precedence
// Returns: The computed value of the term
int Optimizer::term(const char *&expr, int i) {
    while (top(expr) == ' ')
        get(expr);
    int result = factor(expr, i);
    while (top(expr) == ' ')
        get(expr);
    while (top(expr) == '*' || top(expr) == '/') {
        if (get(expr) == '*')
            result *= factor(expr, i);
        else
            result /= factor(expr, i);
    }
    return result;
}

// Parse and evaluate addition/subtraction expressions
// - Processes sequences of + and - operations
// - Maintains operator precedence
// Returns: The computed value of the arithmetic expression
int Optimizer::condition(const char *&expr, int i) {
    while (top(expr) == ' ')
        get(expr);
    int result = term(expr, i);
    while (top(expr) == ' ')
        get(expr);
    while (top(expr) == '+' || top(expr) == '-') {
        if (get(expr) == '+')
            result += term(expr, i);
        else
            result -= term(expr, i);
    }
    return result;
}

// Parse and evaluate comparison expressions
// Handles all comparison operators:
// - < (less than)
// - > (greater than)
// - <= (less than or equal)
// - >= (greater than or equal)
// - == (equality)
// - != (inequality)
// Returns: 1 if condition is true, 0 if false
int Optimizer::expression(const char *&expr, int i) {
    while (top(expr) == ' ')
        get(expr);
    int result = condition(expr, i);
    while (top(expr) == ' ')
        get(expr);
    while (top(expr) == '<' || top(expr) == '>' || top(expr) == '=' || top(expr) == '!') {
        if (top(expr) == '<') {
            get(expr);
            if (top(expr) == '=') {
                get(expr);
                result = result <= condition(expr, i);
            }
            else
                result = result < condition(expr, i);
        }
        else if (top(expr) == '>') {
            get(expr);
            if (top(expr) == '=') {
                get(expr);
                result = result >= condition(expr, i);
            }
            else
                result = result > condition(expr, i);
        }
        else if (top(expr) == '=') {
            get(expr);
            get(expr);
            result = result == condition(expr, i);
        }
        else if (get(expr) == '!') {
            get(expr);
            result = result != condition(expr, i);
        }
    }
    return result;
}

// Constant Propagation Algorithm
// - Traces variable definitions backwards from current line
// - Evaluates constant expressions
// - Updates dead code tracking
// - Generates optimized line replacements
// Parameters:
//   j: Current line number
//   variab: Variable name to evaluate
// Returns: The computed constant value for the variable
int Optimizer::evaluateConstant(int j, llvm::StringRef variab) {
    int i = j;
    bool flag = true;
    while (i > 0 && flag) {
        i--;
        llvm::StringRef corrent_line = Lines[i];
        const char *pointer = corrent_line.begin();

        while (*pointer) {
            while (*pointer && utils::isWhitespace(*pointer)) {
                ++pointer;
            }

            if (utils::isLetter(*pointer)) {
                const char *end = pointer + 1;
                while (utils::isLetter(*end) || utils::isDigit(*end))
                    ++end;
                llvm::StringRef Context(pointer, end - pointer);

                if (Context == variab) {
                    flag = false;
                    break;
                }
                pointer = end;
            }
            else if (utils::isEqual(*pointer)) {
                break;
            }
            ++pointer;
        }
    }
    deadLines[i] = false;
    llvm::StringRef corrent_line = Lines[i];
    const char *pointer = corrent_line.begin();
    const char *start_exp = corrent_line.begin();
    while (!utils::isEqual(*start_exp)) {
        ++start_exp;
    }
    start_exp++;
    llvm::StringRef new_line(pointer, start_exp - pointer);
    start_exp++;
    int value = expression(start_exp, i);
    new_lines[i] = new_line.str() + " " + std::to_string(value) + ";";
    return value;
}

// Main optimization function
// Performs multiple optimization passes:
// 1. Constant propagation starting from output variable
// 2. Dead code elimination for unused statements
// 3. Variable declaration management:
//    - Tracks initialized variables
//    - Adds missing declarations
//    - Maintains proper scoping
// Returns: The optimized code as a single string
std::string Optimizer::optimize() {
    int i = Lines.size();
    evaluateConstant(i, "output");
    code = "";
    int len = Lines.size();

    std::vector<std::string> initialized_variables;

    i = 0;
    while (i < len) {
        if (!deadLines[i]) {
            if ((int)new_lines[i][0] == 10) {
                new_lines[i] = new_lines[i].substr(1);
            }
            std::string temp_str = new_lines[i];
            std::vector<std::string> temp = utils::split(temp_str, " ");

            int j = 0;
            while (temp[j] != "int" && temp[j] != "=" && temp[j] != "bool") {
                j++;
            }

            if (temp[j] == "int" || temp[j] == "bool") {
                j++;
                while (temp[j] == " ") {
                    j++;
                }
                initialized_variables.push_back(temp[j + 1]);
            }
            else {
                int flag = 0;
                for (int k = 0; k < initialized_variables.size(); k++) {
                    std::string temp_str = new_lines[i];
                    std::vector<std::string> temp = utils::split(temp_str, " ");
                    int j = 0;
                    while (temp[j] != "=") {
                        if (initialized_variables[k] == temp[j]) flag = 1;
                        j++;
                    }
                }
                if (flag == 0) {
                    new_lines[i] = "int " + new_lines[i];
                    std::string temp_str = new_lines[i];
                    std::vector<std::string> temp = utils::split(temp_str, " ");
                    j = 0;
                    while (temp[j] == "int" || temp[j] == " ") {
                        j++;
                    }
                    initialized_variables.push_back(temp[j]);
                }
            }
            code.append(new_lines[i]);
            if (i != new_lines.size() - 1) {
                code.append("\n");
            }
        }
        i++;
    }
    return code;
} 