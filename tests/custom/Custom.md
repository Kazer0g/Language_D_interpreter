# Custom Test Programs

Sixteen small, self-contained programs — the kind of thing you would write *in* the
language rather than *about* it. Where the block programs
([Tests.md](../Tests.md)) verify one rule at a time, these exercise the language the way
a real program does: many features at once, over many iterations, with an answer at the
end that is either right or wrong.

Each file is a complete program with no extension, runnable as-is. Expected output is in
`// expect` comments beside the code.

Suite index: [../Tests.md](../Tests.md) · Spec: [../../Language D.md](../../Language%20D.md)

---

## The programs

| #   | Program                                                | Computes                                                                    | Exercises                                            |
| --- | ------------------------------------------------------ | --------------------------------------------------------------------------- | ---------------------------------------------------- |
| 01  | [01-fibonacci](01-fibonacci)                           | the first 15 Fibonacci numbers, their sum                                   | `while`, `for` range, array growth                   |
| 02  | [02-factorial](02-factorial)                           | 0! through 12!                                                              | nested range loops, accumulation, large integers     |
| 03  | [03-gcd-euclid](03-gcd-euclid)                         | GCD by subtraction and by remainder, LCM, GCD of an array                   | `while`, `if/else`, integer division                 |
| 04  | [04-primes](04-primes)                                 | primes below 50, by trial division and by sieve                             | nested loops, `exit`, array as a flag table          |
| 05  | [05-bubble-sort](05-bubble-sort)                       | bubble sort, with and without early termination                             | nested loops, element swap, boolean flag             |
| 06  | [06-insertion-sort](06-insertion-sort)                 | insertion sort and selection sort                                           | `while` with a flag, in-place shifting               |
| 07  | [07-binary-search](07-binary-search)                   | binary search for present and absent values, linear search                  | `exit` from `while`, integer midpoint                |
| 08  | [08-array-statistics](08-array-statistics)             | count, sum, min, max, range, both averages                                  | integer vs real division, `is` on results            |
| 09  | [09-reverse-and-palindrome](09-reverse-and-palindrome) | reverse into a new array, reverse in place, palindrome test                 | index arithmetic, early `exit`                       |
| 10  | [10-matrix-operations](10-matrix-operations)           | transpose, 2×2 multiply, identity, trace                                    | arrays of arrays, `a[i][j]`, triple nesting          |
| 11  | [11-higher-order-functions](11-higher-order-functions) | map, filter, reduce, function tables                                        | functions as values and arguments, tuples as results |
| 12  | [12-tuple-records](12-tuple-records)                   | aggregates and selections over records                                      | array of tuples, named access, tuple extension       |
| 13  | [13-fizzbuzz](13-fizzbuzz)                             | FizzBuzz to 20, category counts to 100                                      | divisibility without modulo, `if/else` chains, `xor` |
| 14  | [14-collatz-and-powers](14-collatz-and-powers)         | Collatz sequences and step counts, fast exponentiation, integer square root | parity without modulo, halving, running maximum      |
| 15  | [15-string-building](15-string-building)               | repeat, join, framed banner                                                 | `String + String` in a loop, strings in composites   |
| 16  | [16-recursion-and-closures](16-recursion-and-closures) | recursive gcd, power, sum and depth; counters; function factories           | recursion, closures by reference, captured locals ([Q19](../../Language%20D.md#11-spec-ambiguities-and-decisions)) |

Each program's `expect` comments use the decided output format
([Q15](../../Language%20D.md#11-spec-ambiguities-and-decisions)): one space between the
values of a `print`, reals always with a `.`, arrays as `[1, 2]`.

---

## Idioms these programs need

Three things a normal language provides are missing from Language D. Every program here
works around them the same way, so the workaround is worth knowing once.

### There is no modulo operator

`d` divides `n` exactly when

```d
d * (n / d) = n
```

because `Integer / Integer` rounds down ([§8.4](../../Language%20D.md#84-division)). Used
in [04-primes](04-primes), [13-fizzbuzz](13-fizzbuzz),
[14-collatz-and-powers](14-collatz-and-powers). The remainder itself is
`n - (n / d) * d`, used in [03-gcd-euclid](03-gcd-euclid).

### There is no way to ask an array for its size

Count it with an aggregate loop first ([T-ARR-903](../12-type-arrays.md#9xx--decided)):

```d
var n := 0
for v in a loop
    n := n + 1
end
```

Every array program opens with this. A function that filters an array has to return the
new count alongside the new array — [11-higher-order-functions](11-higher-order-functions)
returns a tuple `{items, count}` for exactly that reason. Since a missing key reads as
`none` ([Q8](../../Language%20D.md#11-spec-ambiguities-and-decisions)), a dense array
can also be walked with `while not (a[i] is none)`, which
[16-recursion-and-closures](16-recursion-and-closures) does.

### `and` does not short-circuit

Decided ([Q29](../../Language%20D.md#11-spec-ambiguities-and-decisions)): both operands
are always evaluated. The usual insertion sort inner loop

```d
while j >= 1 and a[j] > key loop        // NOT used here
```

reads `a[0]` when `j` reaches 0; that yields `none`
([Q8](../../Language%20D.md#11-spec-ambiguities-and-decisions)), and `none > key` is a
runtime error. [06-insertion-sort](06-insertion-sort) uses an explicit flag instead, and
says so. The general shape is: test the guard first, then the element, in two steps.

### `not x` and `-x` — allowed, and the parenthesised form still used

The grammar as written could not derive `not flag` or `-x`; the corrected grammar can
([Q4](../../Language%20D.md#11-spec-ambiguities-and-decisions)). The programs written
before that decision use `not (flag)` and `0 - x`, which remain valid and are left as
they are; [16-recursion-and-closures](16-recursion-and-closures), written after it,
uses the bare forms.

---

## Decisions these programs rely on

If a program misbehaves, check these first — each is a decision in
[Language D.md §11](../../Language%20D.md#11-spec-ambiguities-and-decisions), and an
interpreter that implements it differently will fail here before it fails anywhere else.

| Behaviour | Where it shows | Decision |
|---|---|---|
| `//` line comments | every file | [Q16](../../Language%20D.md#11-spec-ambiguities-and-decisions) |
| a function body sees, and may assign, variables of the enclosing scope | [16-recursion-and-closures](16-recursion-and-closures) (counters, factories); the older programs pass everything as parameters and work either way | [Q19](../../Language%20D.md#11-spec-ambiguities-and-decisions) |
| recursion | [01-fibonacci](01-fibonacci), [02-factorial](02-factorial), [16-recursion-and-closures](16-recursion-and-closures) | [Q19](../../Language%20D.md#11-spec-ambiguities-and-decisions) |
| a descending range like `1..0` runs zero times | [02-factorial](02-factorial), [09-reverse-and-palindrome](09-reverse-and-palindrome), [15-string-building](15-string-building) | [Q37](../../Language%20D.md#11-spec-ambiguities-and-decisions) |
| `Integer / Integer` floors | not relied on — every division here has positive operands | [Q7](../../Language%20D.md#11-spec-ambiguities-and-decisions) |
| arrays alias on assignment | avoided by the older programs (arrays are mutated in place, never copied and then mutated); used deliberately in [16-recursion-and-closures](16-recursion-and-closures) | [Q26](../../Language%20D.md#11-spec-ambiguities-and-decisions) |
| a missing array key reads as `none` | [16-recursion-and-closures](16-recursion-and-closures) walks arrays with `is none` | [Q8](../../Language%20D.md#11-spec-ambiguities-and-decisions) |
| `print` renders reals as `4.2`, separates values with one space | every `expect` comment involving a real or several values | [Q15](../../Language%20D.md#11-spec-ambiguities-and-decisions) |
| `"a" = "b"` is a valid comparison | [12-tuple-records](12-tuple-records), [15-string-building](15-string-building) note where a string comparison would have helped; they were written before the decision and still select by number | [Q5](../../Language%20D.md#11-spec-ambiguities-and-decisions) |

---

## Resolved question: do we need to read input?

**Every program here has its data hard-coded**, because the language has no way to read
any. The statement list is `Declaration | Assignment | If | IfShort | Loop | Exit |
Return | Print` — `print` is the only I/O construct, and there is no `read`, no `input`,
no command-line arguments, and no file access anywhere in the grammar or the type system.

**Decision ([Q21](../../Language%20D.md#11-spec-ambiguities-and-decisions)): option A,
no input.** Programs are closed; a test is a single source file; the suite needs no
fixture files. The options that were weighed are kept below for the record.

**The question was:** should Language D be able to read input, and if so, in what form?

| Option | Shape | Cost |
|---|---|---|
| **A. No input** *(chosen)* | programs are closed; all data is literal | nothing to build; every test stays a single file |
| **B. A `read` statement** | `read IDENT { , IDENT }`, mirroring `print` | a new statement, a new keyword, and a rule for what type the value gets in a dynamically typed language |
| **C. A builtin function** | `var x := readInt()`, `readLine()` | needs a notion of builtins, which the spec does not have — functions are user-written literals only |
| **D. Command-line arguments** | a predefined array variable, e.g. `args` | needs a predefined name in the global scope, also absent from the spec |

The awkward part of B and C is typing: the language is dynamic, so `read x` has to decide
at runtime whether `42` is an integer, a real or a string. That either needs one
statement per type (`readInt`, `readReal`, `readString`) or a documented inference rule.

That typing problem is the main reason A was chosen: it adds nothing the description
does not already have, and every test stays one file.
