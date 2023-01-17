# Graphing Calculator
A simple c++ graphing calculator.

### Version 2 (new.cpp)

Not quite as featureful as the old one, but rewritten with better logic.
I hope to bring the rest of the features (and maybe more) over eventually.

When you run, the program will prompt you for an equation. It will accept any implicit equation (eg `0=x-y`) using these symbols:
- Groups: `(` ... `)`
- Variables: `x`, `y`, any lowercase letter a-z
- Numbers: `123`, `123.456`, `0.123`, `.123`, `123.0`
- Operators: `+`, `-` (subtract and negate), `*`, `/`, `^` (exponent)
- Multiplication also happens when two values are next to each other, eg `xy`, `3x` or even `3 4`
- Functions: `SIN(x)`, `PI()` - Constants and functions. See FUNCTIONS.md
- Spaces are ignored
  
The calculator follows order of operations when solving equations, and
does so in the following order from left to right:
- Negation
- Groups
- Exponentation
- Multiplication
- Division
- Subtraction
- Addition

---

### Building with g++
1. make sure you have g++ installed, then run
2. `g++ new.cpp -o new -std=c++20` (g++ v12)
3. If that doesn't work try `g++ new.cpp -o new -std=c++2a -DSNUMBERS` (for older g++ versions)
4. Run `new.exe`

### Building in VSCode
My .vscode folder is included in the repository.
If you want to use it:
1. Make sure you have g++ and gdb installed.
2. Edit `launch.json` and `tasks.json` and change `command` and `miDebuggerPath` to point to your g++/gdb executables
3. Open the `new.cpp` file in the editor
4. Click `Run -> Start Debugging` or press `F5`

**An overview of the program logic is provided in new.txt**
---
---

### Version 1 (v1.cpp)
When run, type 'h' or 'help' for a list of options.

When entering equations, refer to the following modes and syntax structure

**Modes**
- Equation: `0 = (your equation eg. x + 2y)`, uses `x` and `y` variables
- Function: `f(x) = (your equation eg. 3x)`, solves for the `y` variable

**Syntax & operations support:**

Operations are evaluated in the order they appear in this list, then in left-to-right order.

* Groups `( )` eg. `(1/2 + (x/4))`
* Exponents `^` eg. `x^2`
* Multiplication `*` and Division `/` eg. `x * 3` or `x / 3`
  * Multiplication can omit the `*`, so `3x`, `xy`, `(1/2)(xy)` and even `3 4` are valid multiplication
* Addition `+` and subtraction `-` eg. `y - x + 3`
* **[BUG]** Trailing `+` and `-` do not work (eg `-x + 3`). Use `(0 - x)` instead.

---

### Bugs
- When entering an equation *starting* with addition or subtraction, (eg `+2` or `-x`), a segmentation fault occurs, seemlingly on line `209-210` (the very beginning of `solve->GET`)

  I do not know what is causing this and am not fixing it as I've now started on v2.

---

### Building/Running

#### Building with g++
1. make sure you have g++ installed, then run

2. `g++ v1.cpp -o v1`

3. Run `v1.exe`

#### Building in VSCode
See instructions for v2
