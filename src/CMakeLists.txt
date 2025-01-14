add_executable(compiler
    Compiler.cpp
    CodeGen.cpp
    Lexer.cpp
    Parser.cpp
    Sema.cpp
    optimizer.cpp
    utils.cpp
)

target_link_libraries(compiler PRIVATE ${llvm_libs})
