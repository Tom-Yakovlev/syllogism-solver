# 🧠 Logic Proof Engine: Syllogism Solver

A modular C++ engine for constructing **step-by-step logical proofs** using classical, derived, and De Morgan inference rules. Inspired by systems like [Carnap.io](https://carnap.io/), this engine produces Carnap-style natural deduction output with a clean and extensible architecture.

---

## ✨ Features

- ✅ Classic inference rules  
  Modus Ponens (MP), Modus Tollens (MT), Double Negation (DNE/DNI), Adjunction, Simplification, etc.

- 📚 Derived rules  
  Includes Hypothetical Syllogism (D-HS), Contrapositive (D-CPO), Conditional Proof (CD), Proof by Cases (D-PBC), and more.

- 🔁 De Morgan's Laws  
  Full bidirectional support for equivalences involving ¬(A ∨ B), ¬(A ∧ B), etc.

- 🔍 Step-by-step Carnap-style proof output  
  Each inference includes justification, line references, and subproof indentation.

- 🧪 Automated proof testing framework  
  Run tests for every rule and verify correct derivations.

- 🧩 Extensible rule system  
  Easily add new rules via self-contained `Rule` objects in `Rules.cpp`.

---

## 🚀 Getting Started

### 🔧 Build Instructions

Ensure you have:

- CMake ≥ 3.10
- C++17-compatible compiler (GCC, Clang, or MSVC)

Then:

```bash
git clone https://github.com/Tom-Yakovlev/syllogism-solver.git
cd syllogism-solver
mkdir build && cd build
cmake ..
make
```

### ▶️ Run the Solver

```bash
./SyllogismSolver
```

### 🧪 Run the Tests

```bash
./ProofSolverTests
```

This will run an automated suite of rule checks and print formatted proof results.

---

## 📋 Usage

The solver prompts for:

1. **Premises** (comma-separated):
   ```
   P->Q, Q->R
   ```

2. **Conclusion**:
   ```
   P->R
   ```

It will derive the conclusion if possible and print the step-by-step natural deduction proof with rule annotations and references.

---

## 🛠 Project Structure

```
.
├── include/              # Header files
├── src/                  # Solver and rule implementation
├── tests/                # Rule-based testing framework
├── CMakeLists.txt        # Build configuration
└── README.md             # You're reading it
```

---

## 📌 License

MIT — use freely for education or research. Contributions welcome!
