cmake_minimum_required(VERSION 3.10)
project(BasicInterpreter)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS " -O2 -pipe -static-libstdc++ -static-libgcc")
include_directories(include)
set(SOURCES
    src/Basic.cpp
    src/Expression.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/Program.cpp
    src/Recorder.cpp
    src/Statement.cpp
    src/Token.cpp
    src/VarState.cpp
    src/utils/Error.cpp
)
add_executable(code )
