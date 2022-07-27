# Graphing Calculator
A simple c++ graphing calculator.

To use, enter an equation when prompted, eg 0 = `x - y` for a simple diagonal line

**Syntax & operations support:**

Please note operations are evaluated in the order they appear

* Groups `( )` eg. `(1/2 + (x/4))`
* Exponents `^` eg. `x^2`
* Multiplication `*` and Division `/` eg. `x * 3` or `x / 3`
  * Multiplication can omit the `*`, so `3x`, `xy`, `(1/2)(xy)` and even `3 4` are valid multiplication
* Addition `+` and subtraction `-` eg. `y - x + 3`

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
