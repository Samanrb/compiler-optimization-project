#include <string>
#include <iostream>
using namespace std;

namespace charinfo
{

    LLVM_READNONE inline bool isWhitespace(char c)
    {
        return c == ' ' || c == '\t' || c == '\f' ||
               c == '\v' || c == '\r' || c == '\n';
    }

    LLVM_READNONE inline bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    LLVM_READNONE inline bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z');
    }

    LLVM_READNONE inline bool isSemiColon(char c)
    {
        return c == ';';
    }

    LLVM_READNONE inline bool isEqual(char c)
    {
        return c == '=';
    }

}

std::vector<std::string> split(std::string& s, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);
    return tokens;
}


class Optimizer
{
    std::vector<llvm::StringRef> Lines;
    std::vector<std::string> new_lines;
    std::vector<bool> dead_lines;
    std::string code = "";
    const char *BufferPtr;

public:
    Optimizer(const llvm::StringRef &Buffer)
    { // constructor scans the whole context

        BufferPtr = Buffer.begin();
        const char *end = BufferPtr + 1;
        while (*end)
        { // since end of context is 0 -> !0 = true -> end of context
            end++;
        }
        llvm::StringRef Context(BufferPtr, end - BufferPtr);
        code = (std::string)Context;

        const char *pointer = BufferPtr;
        const char *line_start = BufferPtr;
        while (*pointer)
        {
            while (!charinfo::isSemiColon(*pointer))
            {
                ++pointer; //  ¯\_(ツ)_/¯
            }
            // pointer++;
            llvm::StringRef Context(line_start, pointer - line_start);
            Lines.push_back(Context);
            dead_lines.push_back(true);
            new_lines.push_back("");
            llvm::errs() << "read line: " << Context << "\n";
            line_start = ++pointer;
        }
    }

public:
    void optimize()
    {
        int i = Lines.size();
        llvm::errs() << "ready for const_pul\n";
        llvm::errs() << "output has value: " << const_pul(i, "output") << "\n";
        code = "";
        int len = Lines.size();

        //convert the initialized_variables to a vector of strings
        std::vector<std::string> initialized_variables;


        i = 0;
        while (i < len)
        {
            if (!dead_lines[i])
            {   
                if((int) new_lines[i][0] == 10){
                    new_lines[i] = new_lines[i].substr(1);
                }
                std::string temp3 = new_lines[i];
                std::vector<std::string> temp1 = split(temp3, " ");


                llvm::errs() << "temp1: " << temp1[0] << "\n";


                int j = 0;
                while (temp1[j] != "int" && temp1[j] != "=")
                {
                    j++;
                }
                

                if (temp1[j] == "int")
                {
                    j++;
                    while (temp1[j] == " ")
                    {
                        j++;
                    }
                    initialized_variables.push_back(temp1[j+1]);
                    llvm::errs() << "initialized_variables: " << initialized_variables[1] << "\n";
                    
                }else{
                    int flag = 0;
                    for(int j = 0; j < initialized_variables.size(); j++){
                        std::vector<std::string> temp = split(new_lines[i], " ");
                        if(std::find(temp.begin(), temp.end(), initialized_variables[i]) != temp.end()){
                            flag = 1;
                            break;
                        }
                    }
                    if(flag == 0){
                        new_lines[i] = "int " + new_lines[i];
                    }
                }
                code.append(new_lines[i]);
                if(i != new_lines.size() - 1){
                    code.append("\n");
                }
            }
            i++;
        }
        llvm::errs() << "\n🚀🚀🚀\n";
        llvm::errs() << code;
        llvm::errs() << "\n🚀🚀🚀\n";
    }

    int const_pul(int j, llvm::StringRef variab)
    {
        int i = j;
        bool flag = true;
        while (i > 0 && flag)
        {
            i--;
            llvm::StringRef corrent_line = Lines[i];
            const char *pointer = corrent_line.begin(); // ¯\_(ツ)_/¯

            while (*pointer)
            {
                while (*pointer && charinfo::isWhitespace(*pointer))
                {
                    ++pointer;
                }

                if (charinfo::isLetter(*pointer))
                {

                    const char *end = pointer + 1;

                    while (charinfo::isLetter(*end) || charinfo::isDigit(*end))
                        ++end;
                    llvm::StringRef Context(pointer, end - pointer);

                    if (Context == variab)
                    {
                        flag = false;
                        break;
                    }

                    pointer = end;
                }
                else if (charinfo::isEqual(*pointer))
                {
                    break;
                }

                ++pointer;
            }
        }
        dead_lines[i] = false;
        llvm::StringRef corrent_line = Lines[i];
        const char *pointer = corrent_line.begin();
        const char *start_exp = corrent_line.begin();
        while (!charinfo::isEqual(*start_exp))
        {
            ++start_exp;
        }
        start_exp++;
        llvm::StringRef new_line(pointer, start_exp - pointer);
        start_exp++;
        int value = expression(start_exp, i);
        new_lines[i] = new_line.str() + " " + std::to_string(value) + ";";
        return value;
    }

    char peek(const char *&expr)
    {
        return *expr;
    }

    char get(const char *&expr)
    {
        return *expr++;
    }
        int number(const char *&expr)
    {
        int result = get(expr) - '0';
        while (peek(expr) >= '0' && peek(expr) <= '9')
        {
            result = 10 * result + get(expr) - '0';
        }
        while (peek(expr) == ' ')
            get(expr);
        return result;
    }

    int variable(const char *&expr, int i)
    {
        const char *temp = expr;
        while (charinfo::isLetter(peek(expr)) || charinfo::isDigit(peek(expr)))
        {
            get(expr);
        }
        llvm::StringRef name(temp, expr - temp);
        while (peek(expr) == ' ')
            get(expr);
        return const_pul(i, name);
    }

    int factor(const char *&expr, int i)
    {
        while (peek(expr) == ' ')
            get(expr);
        if (peek(expr) >= '0' && peek(expr) <= '9')
            return number(expr);
        else if (peek(expr) == '(')
        {
            get(expr); // '('
            while (peek(expr) == ' ')
                get(expr);
            int result = expression(expr, i);
            while (peek(expr) == ' ')
                get(expr);
            get(expr); // ')'
            while (peek(expr) == ' ')
                get(expr);
            return result;
        }
        else if (peek(expr) == '-')
        {
            get(expr);
            return -factor(expr, i);
        }
        else if (charinfo::isLetter(peek(expr)))
        {
            return variable(expr, i);
        }
        return 0; // error
    }

    int term(const char *&expr, int i)
    {
        while (peek(expr) == ' ')
            get(expr);
        int result = factor(expr, i);
        while (peek(expr) == ' ')
            get(expr);
        while (peek(expr) == '*' || peek(expr) == '/')
        {
            if (get(expr) == '*')
                result *= factor(expr, i);

            else
                result /= factor(expr, i);
        }
        return result;
    }

    int expression(const char *&expr, int i)
    {
        while (peek(expr) == ' ')
            get(expr);
        int result = term(expr, i);
        while (peek(expr) == ' ')
            get(expr);
        while (peek(expr) == '+' || peek(expr) == '-')
        {
            if (get(expr) == '+')
                result += term(expr, i);
            else
                result -= term(expr, i);
        }
        return result;
    }
};