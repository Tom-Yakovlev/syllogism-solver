cmake_minimum_required(VERSION 3.10)
project(SyllogismSolver)

set(CMAKE_CXX_STANDARD 17)

# Include header directory
include_directories(include)

# Main executable
add_executable(SyllogismSolver
    src/main.cpp
    src/ProofSolver.cpp
    src/Rules.cpp
)

# Test executable
add_executable(ProofSolverTests
    tests/ProofSolverTests.cpp
    src/ProofSolver.cpp
    src/Rules.cpp
)

# Enable testing and register test
enable_testing()
add_test(NAME ProofSolverTests COMMAND ProofSolverTests)

