cmake_minimum_required(VERSION 3.10)
project(SyllogismSolver)

set(CMAKE_CXX_STANDARD 17)

add_executable(SyllogismSolver
    src/main.cpp
    src/ProofSolver.cpp
    src/Rules.cpp
)