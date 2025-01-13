#include "Lexer.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include "AST.h"
#include "CodeGen.h"
#include "Parser.h"
#include "Sema.h"
#include "optimizer.h"


using namespace std;

// Define a command-line option for specifying the input expression.
static llvm::cl::opt<std::string>
    Input(llvm::cl::Positional,
          llvm::cl::desc("<input expression>"),
          llvm::cl::init(""));

static llvm::cl::opt<std::string> FileName("f",
	llvm::cl::desc("<Specify the file name>"),
	llvm::cl::value_desc("filename"),
	llvm::cl::init(""));


// The main function of the program.
int main(int argc, const char **argv)
{
    // Initialize the LLVM framework.
    llvm::InitLLVM X(argc, argv);

    // Parse command-line options.
    llvm::cl::ParseCommandLineOptions(argc, argv, "Simple Compiler\n");



	string contentString;
	llvm::StringRef contentRef;

	if (!FileName.empty()) // if filename is specified
	{
		std::string fileName = FileName;

		// Use llvm::MemoryBuffer::getFile with the fileName
		llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrErr =
			llvm::MemoryBuffer::getFile(fileName);

		if (auto error = fileOrErr.getError()) {
			llvm::errs() << "Error opening file: " << error.message() << "\n";
			return 1;
		}
		// Use the file content from the MemoryBuffer
		contentString = (*fileOrErr)->getBuffer().str();
	}
	else // if input is given directly
	{
		contentString = Input;

	}

	contentRef = contentString;

	Token nextToken;

	Optimizer optimizer(contentRef);

    std::string formattedCode = optimizer.optimize();
	// std::string code = remove_code.pointer_to_string();
	std::cout << "\n---------------\n🚀Optimized code: \n" << formattedCode << "\n---------------\n" << std::endl;


    // Create a lexer object and initialize it with the input expression.
    Lexer Lex(formattedCode);

    // Create a parser object and initialize it with the lexer.
    Parser Parser(Lex);

    // Parse the input expression and generate an abstract syntax tree (AST).
    Program *Tree = Parser.parse();

    // Check if parsing was successful or if there were any syntax errors.
    if (!Tree || Parser.hasError())
    {
        llvm::errs() << "Syntax errors occurred\n";
        return 1;
    }

    // Perform semantic analysis on the AST.
    Sema Semantic;
    if (Semantic.semantic(Tree))
    {
        llvm::errs() << "Semantic errors occurred\n";
        return 1;
    }

    // Generate code for the AST using a code generator.
    CodeGen CodeGenerator;
    CodeGenerator.compile(Tree);

    // The program executed successfully.
    return 0;
}
