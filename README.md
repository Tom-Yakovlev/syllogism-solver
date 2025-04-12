# Logic Proof Engine

A modular C++ engine for constructing step-by-step logical proofs using propositional and predicate inference rules, inspired by systems like Carnap.io.

## ✨ Features
- Supports classic inference rules like:
  - Modus Ponens (MP)
  - Modus Tollens (MT)
  - Double Negation Elimination (DNE)
  - Double Negation Introduction (DNI)
- Step-by-step natural deduction output
- Carnap-style proof formatting
- Cleanly modularized for extensibility
- Input-based proof loop with terminal UI

## 🚀 Getting Started
### 🔧 Build Instructions
1. Make sure you have a C++17 compiler and CMake 3.10+
2. Clone this repository:
git clone https://github.com/YOUR_USERNAME/logic-proof-engine.git
cd logic-proof-engine

Build and run:
mkdir build && cd build
cmake ..
make
./SyllogismSolver

📋 Usage
Enter premises separated by commas, ending with ;
Enter the conclusion, ending with ;
The engine will display a step-by-step proof if derivable
