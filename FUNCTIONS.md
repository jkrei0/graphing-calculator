# Functions
Functions are unary operators indicated by one or more uppercase letters A-Z. They operate on their right-hand value.

### Using functions
Functions can be called with or without a value. If you don't include a value it is assumed to be 0.

To call a function without a value, pass it an empty group `()`.

*To see a list of functions, skip to the Function List section. However, you should probably read this first to understand their semantics.*

**Examples**

- `SINx` = sin of x
- `SIN(8)` = sin of pi
- `SIN(x)` = sin of x
- `SINx+y` = (sin of x) + y
- `SIN(x+y)` = sin of (x+y)
- `PI()` = Constant PI (3.14...)
- `PI(x)` = PI times x
- `SIN(PI())` = sin of PI
- `SINPI(x)` = 0, SINPI is not a function
- `PI` = Error, function has no right side.

### N-Functions
Some functions, eg `LOGN` (log base n) require two arguments. Since functions only have one right-side operand, another (setter) function can be used to set the N value. For `LOGN`, the setter function is `SETN`. Setter functions return 0.

The best way to use setter functions is `FUNC(SETTER(x) + y)`. For `LOGN`, you would use `LOGN(SETN(5) + x)` To take the log base 5 of x.

**Setter values stay until you restart the program**, and will **not** reset to the defaults when you make/plot a new graph.

**Caution**

It might be tempting to use `SETN(5)LOGN(x)`, however **this can lead to problems** if you have another setter on the inside.
- For example, `SETN(5)LOGN(5 + SETN(3)LOGN(x))` sets N to 5, but then must parse `(5 + SETN(3)LOGN(x))` before it can do the first LOGN. SETN is set to 3, and then the orignial LOGN does log base 3.

`LOGN(SETN(x) + y)` guarantees that `SETN(x)` will happen immediately before `LOGN` for two reasons: One, `+` is the lowest priority operation and will be executed last and Two, the left side of the operation is parsed last (for this reason).

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
|`LOG`  |log base 10|Returns the log base 10 of a value
|`LOGE` |natural log|Returns the natural log of a value
|`LOGN` |log base n |Returns the log base n of a value
|`SETN` | |Sets the base used in `LOGN`. The default is `2`
|MODULO
|`EVEN` |modulo\* 2  |See below (when `m` is 2)
|`MOD`  |modulo\*    |The floating-point remainder of division `x/m`
|`SETM` | |Sets the divisor for modulo (the `m` value)
| | | \**Modulo in this case means `x - n*y`, where `n` is `x/y` with its fractional part truncated. (from [std::fmod - cppreference](https://en.cppreference.com/w/cpp/numeric/math/fmod))*
|SIGNS
|`ABS`  |absolute value |Returns the absolute value of a number
|`SIGN` |sign |Returns 1 if `value > 0`, 0 if `value = 0`, and -1 if `value < 0`
|CONSTANTS
|`PI()` |pi     |(no argument) The constant `pi`
|`PI`   |`PI()*x`|(given value) The constant `pi` times a value
|`E()`  |e      |(no argument) The constant `e`
|`E`    |`E()*x`|(given value) The constant `e` times a value

Undefined functions return 0