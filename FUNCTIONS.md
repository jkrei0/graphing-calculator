# Functions
Functions are unary operators indicated by one or more uppercase letters A-Z. They operate on their right-hand value.

### Using functions
Functions can be called with or without a value. If you don't include a value it is assumed to be 0.

To call a function without a value, pass it an empty group `()`.

*To see a list of functions, skip to the Function List section. However, you should probably read this first to understand their semantics.*

**Examples**

- `SINx` = sin of x
- `SIN(8)` = sin of 8
- `SIN(x)` = sin of x
- `SINx+y` = (sin of x) + y
- `SIN(x+y)` = sin of (x+y)
- `SIN(PI())` = sin of PI
- `SINPI(x)` = 0, SINPI is not a function
- `PI()` = Constant PI (3.14...)
- `PI(x)` = Constant PI (`PI` ignores its argument)
- `PI` = Error, function has no right side.

## Function List
|Name   |Short  |Description
|-|-|-|
|TRIG
|`SIN`  |sine   |Returns the sine of a value
|`COS`  |cosine |Returns the cosine of a value
|`TAN`  |tangent|Returns the tangent of a value
|`ASIN` |arcsin |Returns the inverse sine of a value
|`ACOS` |arccos |Returns the inverse cosine of a value
|`ATAN` |arctan |Returns the inverse tangent of a value
|ROOTS
|`SQRT` |square root|Returns the square root of a value
|`CBRT` |cube root  |Returns the cube root of a value
|       |nth root   |Does not exist, but you can use `x^(1/y)` to take the `y` root of `x`
|LOGARITHMS
|`LOG`  |log base 10          |Returns the log base 10 of a value
|`LN`   |natural log (base e) |Returns the natural log of a value
|`LB`   |binary log (base 2)  |Returns the binary log of a value
|MODULO
|`EVEN` |modulo\* 2 | 0 if even, 1 or -1 if odd.
| | | *Use `%` operator for x mod y: `x % y`* 
|SIGNS
|`ABS`  |absolute value |Returns the absolute value of a number
|`SIGN` |sign |Returns 1 if `value > 0`, 0 if `value = 0`, and -1 if `value < 0`
|CONSTANTS
|`PI()` |pi     |(no argument) The constant `pi`
|`E()`  |e      |(no argument) The constant `e`

Undefined functions return 0