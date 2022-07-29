# Graphing Calculator
A simple c++ graphing calculator.

When run, the program should give instructions for use.

**Modes**
- Equation: `0 = (your equation eg. x + 2y)`, uses `x` and `y` variables
- Function: `f(x) = (your equation eg. 3x)`, `y` variable is treated as zero if used

**Syntax & operations support:**

Operations are evaluated in the order they appear in this list, then in left-to-right order.

* Groups `( )` eg. `(1/2 + (x/4))`
* Exponents `^` eg. `x^2`
* Multiplication `*` and Division `/` eg. `x * 3` or `x / 3`
  * Multiplication can omit the `*`, so `3x`, `xy`, `(1/2)(xy)` and even `3 4` are valid multiplication
* Addition `+` and subtraction `-` eg. `y - x + 3`
* **[BUG]** Trailing `+` and `-` do not work (eg `-x + 3`). Use `(0 - x)` instead.

---

## Bugs
Bugs nearest the top of the list are ones that will likely get fixed first

- When entering an equation *starting* with addition or subtraction, (eg `+2` or `-x`), a segmentation fault occurs, seemlingly on line `209-210` (the very beginning of `solve->GET`)

  I do not know what is causing this.

---

## Building/Running

### Building with g++
1. make sure you have g++ installed, then run

2. `g++ main.cpp -o main`

3. Run `main.exe`

### Building in VSCode
A simple .vscode folder is included for building/debugging.
If you want to use it:
1. Make sure you have g++ and gdb installed.

2. Edit `launch.json` and `tasks.json` and change `command` and `miDebuggerPath` to point to your g++/gdb executables

3. Click `Run -> Start Debugging` or press `F5`
