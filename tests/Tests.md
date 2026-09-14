# Language D — Test Suite

Index of the test suite. Every test lives in a **block file**; this page is the only
place that lists all blocks, and it does so **twice** so the suite is reachable from
both directions:

- [Index A — by language topic](#index-a--by-language-topic): start from a rule in the
  spec, find the tests that verify it.
- [Index B — by test block](#index-b--by-test-block): start from a test file, find the
  rule it verifies.

Spec: [Language D.md](../Language%20D.md) · Custom programs: [custom/Custom.md](custom/Custom.md)

## Two kinds of file

Each block is a **pair** of files with the same name:

| File | Extension | What it is |
|---|---|---|
| `04-op-addition.md` | `.md` | the description — every test as a row, with its ID, program and expected result |
| `04-op-addition` | *(none)* | the runnable program — every test of that block composed into one Language D source file |

A composed program holds all the block's **runnable** tests, each preceded by its ID and
expected value. Negative tests cannot share a run — a syntax error stops the file from
parsing and a runtime error aborts execution — so they sit **commented out** at the end
of the same file, one per entry, ready to be uncommented one at a time.

Two things the composed programs rely on, both noted in every file's header:

- `//` line comments ([Q16](../Language%20D.md#11-spec-ambiguities-and-decisions));
- a **branch body is a scope** ([§3.2](../Language%20D.md#32-scope-and-visibility)), so
  `if true then ... end` wraps each test that declares variables and keeps one test's
  `var a` from colliding with the next one's.

**Every question the description left open is decided.** The decisions are numbered
Q1–Q41 and collected in
[Language D.md §11](../Language%20D.md#11-spec-ambiguities-and-decisions); each block's
`9xx` section names the questions it raised and the tests that now verify the answer.
There is no test in the suite whose expected value is still `?`.

Alongside the blocks, [custom/](custom/) holds sixteen ordinary programs — sorts,
Fibonacci, primes, matrices, closures — that exercise the language the way real code does.

---

## Conventions

### Test IDs

Each block owns a prefix; each test has a stable ID `T-<PREFIX>-<number>`.
The number encodes the **kind** of the test, so `T-ADD-104` is recognisably a
negative type test for addition without opening the file:

| Range | Kind | Meaning |
|---|---|---|
| `0xx` | **positive** | Supported operand types / valid programs. Must produce the stated value. |
| `1xx` | **negative** | Unsupported operand types / invalid programs. Must be rejected. |
| `2xx` | **semantics** | Behaviour beyond the type matrix: precedence, order, scope, mutation. |
| `3xx` | **structure** | Grammar and syntax shape (separators, optional parts, nesting). |
| `4xx` | **edge** | Boundaries: empty, single element, deep nesting, aliasing. |
| `9xx` | **decided** | A question the description left open, with the decision taken ([§11](../Language%20D.md#11-spec-ambiguities-and-decisions)) and the tests that verify it. A `9xx` row that carries its own program is an ordinary pass/fail test; one that only points at other IDs is a record of the decision. |

IDs are **never reused**. If a test is dropped, its ID retires with it. A test whose
verdict changed when a question was decided keeps its ID and its place — for example
`T-CMP-104` (`"a" = "a"`) still sits in the `1xx` section of block 08, now expecting
`true`, so the change is visible where it happened.

### Type matrix convention

The language has eight operand kinds. Every operator block reuses the same canonical
operand values, so "the same type verification test" reads identically across
addition, subtraction, multiplication, division and comparison:

| Kind | Canonical value | Written in tests as |
|---|---|---|
| integer | `1` | `1`, `2`, `5`, `7` |
| real | `1.5` | `1.5`, `2.0`, `3.5` |
| string | `"s"` | `"a"`, `"ab"` |
| boolean | `true` | `true`, `false` |
| array | `[1]` | `[1]`, `[1,2]` |
| tuple | `{a := 1}` | `{a := 1}` |
| func | `func => 0` | `func => 0` |
| none | *(uninitialised var)* | `var n` then `n` |

Each binary operator block contains the same three sections in the same order:

1. **Result grid** — the full 8×8 kind-by-kind table, `→ type` where allowed and `✗`
   where not.
2. **Supported types** (`0xx`) — one test per allowed cell.
3. **Unsupported types** (`1xx`) — one test per disallowed **row kind**, plus the mixed
   pairs the spec explicitly rules out.

### Expected results

- A positive test states the exact expected console output, in the format fixed by
  [Q15](../Language%20D.md#11-spec-ambiguities-and-decisions): one space between the
  values of one `print`, one line per `print`.
- A negative test states `runtime error` — the language is dynamic, so operand type
  control happens **during execution**, not at compile time
  ([spec §8](../Language%20D.md#8-operator-semantics)). A test that must be rejected
  before execution states `syntax error` instead. Which errors are which is fixed by
  [Q24](../Language%20D.md#11-spec-ambiguities-and-decisions): grammar violations and the
  three placement rules (`exit`, `return`, call as assignment target) are syntax errors;
  everything else is a runtime error.
- A runtime error aborts the program; output produced before it stays and is part of
  the expected result where a test says so.

---

## Index A — by language topic

Follows the section order of [Language D.md](../Language%20D.md).

| Spec section                                                                   | Verified by                                                                  | IDs                             |     |
| ------------------------------------------------------------------------------ | ---------------------------------------------------------------------------- | ------------------------------- | --- |
| [§2 Program Structure](../Language%20D.md#2-program-structure)                 | [01-lexical-and-literals.md](01-lexical-and-literals.md)                     | `T-LEX-3xx`                     |     |
| [§3.1 Variable Declaration](../Language%20D.md#31-variable-declaration)        | [02-declarations-and-scope.md](02-declarations-and-scope.md)                 | `T-DECL-0xx`, `T-DECL-1xx`      |     |
| [§3.2 Scope and Visibility](../Language%20D.md#32-scope-and-visibility)        | [02-declarations-and-scope.md](02-declarations-and-scope.md)                 | `T-DECL-2xx`                    |     |
| [§4.1 Assignment](../Language%20D.md#41-assignment)                            | [03-assignment.md](03-assignment.md)                                         | `T-ASGN-*`                      |     |
| [§4.2 If](../Language%20D.md#42-if)                                            | [15-stmt-conditionals.md](15-stmt-conditionals.md)                           | `T-IF-0xx`, `T-IF-1xx`          |     |
| [§4.3 IfShort](../Language%20D.md#43-ifshort)                                  | [15-stmt-conditionals.md](15-stmt-conditionals.md)                           | `T-IF-2xx`                      |     |
| [§4.4 Body](../Language%20D.md#44-body)                                        | [01-lexical-and-literals.md](01-lexical-and-literals.md)                     | `T-LEX-3xx`                     |     |
| [§4.5 Loop](../Language%20D.md#45-loop)                                        | [16-stmt-loops.md](16-stmt-loops.md)                                         | `T-LOOP-0xx`–`T-LOOP-3xx`       |     |
| [§4.6 Exit](../Language%20D.md#46-exit)                                        | [16-stmt-loops.md](16-stmt-loops.md)                                         | `T-LOOP-4xx`                    |     |
| [§4.7 Return](../Language%20D.md#47-return)                                    | [14-type-functions.md](14-type-functions.md)                                 | `T-FUN-3xx`                     |     |
| [§4.8 Print](../Language%20D.md#48-print)                                      | [17-stmt-print.md](17-stmt-print.md)                                         | `T-PRN-*`                       |     |
| [§5 Expressions](../Language%20D.md#5-expressions)                             | [18-expression-precedence.md](18-expression-precedence.md)                   | `T-PREC-*`                      |     |
| [§5.7 TypeIndicator](../Language%20D.md#57-typeindicator)                      | [11-op-type-check-is.md](11-op-type-check-is.md)                             | `T-IS-0xx`                      |     |
| [§6 References](../Language%20D.md#6-references-accessors)                     | [12](12-type-arrays.md), [13](13-type-tuples.md), [14](14-type-functions.md) | `T-ARR-*`, `T-TUP-*`, `T-FUN-*` |     |
| [§7.1–7.4 Simple types](../Language%20D.md#7-types-and-literals)               | [01-lexical-and-literals.md](01-lexical-and-literals.md)                     | `T-LEX-0xx`                     |     |
| [§7.5 Array](../Language%20D.md#75-array)                                      | [12-type-arrays.md](12-type-arrays.md)                                       | `T-ARR-*`                       |     |
| [§7.6 Tuple](../Language%20D.md#76-tuple)                                      | [13-type-tuples.md](13-type-tuples.md)                                       | `T-TUP-*`                       |     |
| [§7.7 Function](../Language%20D.md#77-function)                                | [14-type-functions.md](14-type-functions.md)                                 | `T-FUN-*`                       |     |
| [§7.8 none](../Language%20D.md#78-none)                                        | [11-op-type-check-is.md](11-op-type-check-is.md)                             | `T-IS-4xx`                      |     |
| [§8.1 Addition](../Language%20D.md#81-addition)                                | [04-op-addition.md](04-op-addition.md)                                       | `T-ADD-*`                       |     |
| [§8.2 Subtraction](../Language%20D.md#82-subtraction)                          | [05-op-subtraction.md](05-op-subtraction.md)                                 | `T-SUB-*`                       |     |
| [§8.3 Multiplication](../Language%20D.md#83-multiplication)                    | [06-op-multiplication.md](06-op-multiplication.md)                           | `T-MUL-*`                       |     |
| [§8.4 Division](../Language%20D.md#84-division)                                | [07-op-division.md](07-op-division.md)                                       | `T-DIV-*`                       |     |
| [§8.5 Comparisons](../Language%20D.md#85-comparisons)                          | [08-op-comparison.md](08-op-comparison.md)                                   | `T-CMP-*`                       |     |
| [§8.6 Unary plus and minus](../Language%20D.md#86-unary-plus-and-minus)        | [09-op-unary-sign.md](09-op-unary-sign.md)                                   | `T-UNS-*`                       |     |
| [§8.7 Logical operations](../Language%20D.md#87-logical-operations)            | [10-op-logical.md](10-op-logical.md)                                         | `T-LOG-0xx`–`T-LOG-4xx`         |     |
| [§8.8 Unary negation](../Language%20D.md#88-unary-negation)                    | [10-op-logical.md](10-op-logical.md)                                         | `T-LOG-5xx`                     |     |
| [§8.9 Type check `is`](../Language%20D.md#89-type-check-is)                    | [11-op-type-check-is.md](11-op-type-check-is.md)                             | `T-IS-*`                        |     |
| [§9 Implicit Type Conversions](../Language%20D.md#9-implicit-type-conversions) | [19-implicit-conversions.md](19-implicit-conversions.md)                     | `T-CONV-*`                      |     |

---

## Index B — by test block

| # | Description | Program | Prefix | Covers | Spec |
|---|---|---|---|---|---|
| 01 | [01-lexical-and-literals.md](01-lexical-and-literals.md) | [`01-lexical-and-literals`](01-lexical-and-literals) | `LEX` | Literal forms of every type, statement separators, comments | [§2](../Language%20D.md#2-program-structure), [§7](../Language%20D.md#7-types-and-literals) |
| 02 | [02-declarations-and-scope.md](02-declarations-and-scope.md) | [`02-declarations-and-scope`](02-declarations-and-scope) | `DECL` | `var`, multiple definitions, `none` default, shadowing, redeclaration | [§3](../Language%20D.md#3-declarations) |
| 03 | [03-assignment.md](03-assignment.md) | [`03-assignment`](03-assignment) | `ASGN` | `Reference := Expression`, type change on reassignment, all reference forms as targets | [§4.1](../Language%20D.md#41-assignment) |
| 04 | [04-op-addition.md](04-op-addition.md) | [`04-op-addition`](04-op-addition) | `ADD` | `+` on supported and unsupported types (incl. string/array/tuple concatenation) | [§8.1](../Language%20D.md#81-addition) |
| 05 | [05-op-subtraction.md](05-op-subtraction.md) | [`05-op-subtraction`](05-op-subtraction) | `SUB` | `-` on supported and unsupported types | [§8.2](../Language%20D.md#82-subtraction) |
| 06 | [06-op-multiplication.md](06-op-multiplication.md) | [`06-op-multiplication`](06-op-multiplication) | `MUL` | `*` on supported and unsupported types | [§8.3](../Language%20D.md#83-multiplication) |
| 07 | [07-op-division.md](07-op-division.md) | [`07-op-division`](07-op-division) | `DIV` | `/` on supported and unsupported types, integer round-down | [§8.4](../Language%20D.md#84-division) |
| 08 | [08-op-comparison.md](08-op-comparison.md) | [`08-op-comparison`](08-op-comparison) | `CMP` | `< > <= >= = /=` on supported and unsupported types | [§8.5](../Language%20D.md#85-comparisons) |
| 09 | [09-op-unary-sign.md](09-op-unary-sign.md) | [`09-op-unary-sign`](09-op-unary-sign) | `UNS` | Unary `+` and `-` | [§8.6](../Language%20D.md#86-unary-plus-and-minus) |
| 10 | [10-op-logical.md](10-op-logical.md) | [`10-op-logical`](10-op-logical) | `LOG` | `and`, `or`, `xor`, `not` | [§8.7](../Language%20D.md#87-logical-operations), [§8.8](../Language%20D.md#88-unary-negation) |
| 11 | [11-op-type-check-is.md](11-op-type-check-is.md) | [`11-op-type-check-is`](11-op-type-check-is) | `IS` | `is` against every `TypeIndicator`, and `none` handling | [§8.9](../Language%20D.md#89-type-check-is), [§7.8](../Language%20D.md#78-none) |
| 12 | [12-type-arrays.md](12-type-arrays.md) | [`12-type-arrays`](12-type-arrays) | `ARR` | Array literals, 1-based access, associative/sparse keys, dynamic growth | [§7.5](../Language%20D.md#75-array) |
| 13 | [13-type-tuples.md](13-type-tuples.md) | [`13-type-tuples`](13-type-tuples) | `TUP` | Tuple literals, named/unnamed elements, dot and positional access, immutability | [§7.6](../Language%20D.md#76-tuple) |
| 14 | [14-type-functions.md](14-type-functions.md) | [`14-type-functions`](14-type-functions) | `FUN` | Function literals, both body forms, calls, functions as values, `return` | [§7.7](../Language%20D.md#77-function), [§4.7](../Language%20D.md#47-return) |
| 15 | [15-stmt-conditionals.md](15-stmt-conditionals.md) | [`15-stmt-conditionals`](15-stmt-conditionals) | `IF` | `if/then/else/end` and `if =>` short form | [§4.2](../Language%20D.md#42-if), [§4.3](../Language%20D.md#43-ifshort) |
| 16 | [16-stmt-loops.md](16-stmt-loops.md) | [`16-stmt-loops`](16-stmt-loops) | `LOOP` | `while`, `for` range, `for in` aggregate, infinite `loop`, `exit` | [§4.5](../Language%20D.md#45-loop), [§4.6](../Language%20D.md#46-exit) |
| 17 | [17-stmt-print.md](17-stmt-print.md) | [`17-stmt-print`](17-stmt-print) | `PRN` | `print` with one and many expressions, value rendering | [§4.8](../Language%20D.md#48-print) |
| 18 | [18-expression-precedence.md](18-expression-precedence.md) | [`18-expression-precedence`](18-expression-precedence) | `PREC` | Precedence and associativity across all five expression levels | [§5](../Language%20D.md#5-expressions) |
| 19 | [19-implicit-conversions.md](19-implicit-conversions.md) | [`19-implicit-conversions`](19-implicit-conversions) | `CONV` | Integer → real widening in arithmetic and comparison | [§9](../Language%20D.md#9-implicit-type-conversions) |

---

## Cross-block map: the shared type matrix

Blocks 04–09 all check operand types. This grid is what they verify between them —
read a row to see how one type behaves across every operator, read a column to see
which types one operator accepts.

| Operand types | `+` | `-` | `*` | `/` | `< > <= >=` | `= /=` | unary `+ -` | `and or xor` | `not` |
|---|---|---|---|---|---|---|---|---|---|
| integer ⊕ integer | Integer | Integer | Integer *(floor)* | Integer | boolean | boolean | Integer | ✗ | ✗ |
| integer ⊕ real | Real | Real | Real | Real | boolean | boolean | — | ✗ | ✗ |
| real ⊕ integer | Real | Real | Real | Real | boolean | boolean | Real | ✗ | ✗ |
| real ⊕ real | Real | Real | Real | Real | boolean | boolean | — | ✗ | ✗ |
| string ⊕ string | String | ✗ | ✗ | ✗ | ✗ | **boolean** ([Q5](../Language%20D.md#11-spec-ambiguities-and-decisions)) | ✗ | ✗ | ✗ |
| boolean ⊕ boolean | ✗ | ✗ | ✗ | ✗ | ✗ | **boolean** ([Q5](../Language%20D.md#11-spec-ambiguities-and-decisions)) | ✗ | Boolean | Boolean |
| array ⊕ array | Array *(renumbered)* | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| tuple ⊕ tuple | Tuple *(no duplicate names)* | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| func ⊕ func | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| none ⊕ none | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| any mixed pair not listed | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | — | ✗ | — |

`✗` = not allowed, runtime error. `—` = not applicable (unary operators take one operand).
The two bold cells are the project's one extension of the description's operator table
([Q5](../Language%20D.md#11-spec-ambiguities-and-decisions)). Division by zero is a
runtime error in every numeric cell ([Q6](../Language%20D.md#11-spec-ambiguities-and-decisions));
an integer result outside 64 bits is a runtime error
([Q27](../Language%20D.md#11-spec-ambiguities-and-decisions)).

---

## Index C — custom programs

Sixteen ordinary programs in [custom/](custom/), described in
[custom/Custom.md](custom/Custom.md). Where the blocks above verify one rule at a time,
these run the language the way real code does — many features at once, over many
iterations, with an answer at the end that is either right or wrong.

| # | Program | Computes |
|---|---|---|
| 01 | [`custom/01-fibonacci`](custom/01-fibonacci) | the first 15 Fibonacci numbers, their sum |
| 02 | [`custom/02-factorial`](custom/02-factorial) | 0! through 12! |
| 03 | [`custom/03-gcd-euclid`](custom/03-gcd-euclid) | GCD by subtraction and by remainder, LCM |
| 04 | [`custom/04-primes`](custom/04-primes) | primes below 50, by trial division and by sieve |
| 05 | [`custom/05-bubble-sort`](custom/05-bubble-sort) | bubble sort, with and without early termination |
| 06 | [`custom/06-insertion-sort`](custom/06-insertion-sort) | insertion sort and selection sort |
| 07 | [`custom/07-binary-search`](custom/07-binary-search) | binary search for present and absent values |
| 08 | [`custom/08-array-statistics`](custom/08-array-statistics) | count, sum, min, max, both averages |
| 09 | [`custom/09-reverse-and-palindrome`](custom/09-reverse-and-palindrome) | reverse an array two ways, palindrome test |
| 10 | [`custom/10-matrix-operations`](custom/10-matrix-operations) | transpose, 2×2 multiply, identity, trace |
| 11 | [`custom/11-higher-order-functions`](custom/11-higher-order-functions) | map, filter, reduce, function tables |
| 12 | [`custom/12-tuple-records`](custom/12-tuple-records) | aggregates and selections over records |
| 13 | [`custom/13-fizzbuzz`](custom/13-fizzbuzz) | FizzBuzz to 20, category counts to 100 |
| 14 | [`custom/14-collatz-and-powers`](custom/14-collatz-and-powers) | Collatz chains, fast exponentiation, integer √ |
| 15 | [`custom/15-string-building`](custom/15-string-building) | repeat, join, framed banner |
| 16 | [`custom/16-recursion-and-closures`](custom/16-recursion-and-closures) | recursive algorithms, counters, function factories, recursion over arrays |

Three things a normal language provides are missing from Language D — no modulo, no way
to ask an array for its size, and no short-circuit `and`/`or`
([Q29](../Language%20D.md#11-spec-ambiguities-and-decisions)). Every custom program works
around them the same way; the idioms are written out once in
[Custom.md](custom/Custom.md#idioms-these-programs-need). Recursion and closures are
available ([Q19](../Language%20D.md#11-spec-ambiguities-and-decisions)), and
[01-fibonacci](custom/01-fibonacci), [02-factorial](custom/02-factorial) and
[16-recursion-and-closures](custom/16-recursion-and-closures) use them.

---

## Index D — by decision

Start from a decision in [Language D.md §11](../Language%20D.md#11-spec-ambiguities-and-decisions),
find the tests that pin it down. Block-level `9xx` rows give the full ID lists.

| Decision | Verified in |
|---|---|
| Q1 `none` = "empty" | [02](02-declarations-and-scope.md#9xx--decided) `T-DECL-901` |
| Q2 full `Literal` production | [01](01-lexical-and-literals.md#0xx--literal-forms) `T-LEX-0xx` |
| Q3 `Factor` operator required | [18](18-expression-precedence.md#4xx--grammar-corrections) `T-PREC-401` |
| Q4 prefix on any `Unary` | [09](09-op-unary-sign.md#3xx--unary-on-a-reference) `T-UNS-3xx`, [10](10-op-logical.md#5xx--unary-negation-not) `T-LOG-515`–`519`, [18](18-expression-precedence.md#4xx--grammar-corrections) `T-PREC-402` |
| Q5 `=` `/=` on strings and booleans | [08](08-op-comparison.md#extended-cells--and--on-strings-and-booleans-q5) `T-CMP-027`–`036`, `T-CMP-1xx`, [18](18-expression-precedence.md#2xx--non-repeating-relation) `T-PREC-203` |
| Q6 division by zero | [07](07-op-division.md#4xx--division-by-zero) `T-DIV-4xx` |
| Q7 floor division | [07](07-op-division.md#2xx--integer-division-rounds-down) `T-DIV-205`–`210` |
| Q8 missing key reads `none` | [12](12-type-arrays.md#1xx--invalid-access-and-literals) `T-ARR-101`–`103`, `T-ARR-208`–`210`, `T-ARR-213` |
| Q9 `+` renumbers arrays | [04](04-op-addition.md#4xx--edge-cases) `T-ADD-405`, `T-ADD-409` |
| Q10 duplicate tuple names on `+` | [04](04-op-addition.md#4xx--edge-cases) `T-ADD-406`, [13](13-type-tuples.md#2xx--immutability-and-extension) `T-TUP-207` |
| Q11 tuple element assignment | [03](03-assignment.md#0xx--valid-assignment-targets) `T-ASGN-004`, `008`, `009`, `109`, [13](13-type-tuples.md#2xx--immutability-and-extension) `T-TUP-204`–`206`, `209`, `210` |
| Q12 real range bounds | [16](16-stmt-loops.md#1xx--for-loop-over-a-range) `T-LOOP-109`, `116`, [19](19-implicit-conversions.md#3xx--widening-is-one-directional) `T-CONV-305` |
| Q13 tuple iteration yields values | [16](16-stmt-loops.md#2xx--for-loop-over-an-aggregate) `T-LOOP-211`, [13](13-type-tuples.md#3xx--tuples-in-other-constructs) `T-TUP-306` |
| Q14 `return` outside a function | [14](14-type-functions.md#3xx--return) `T-FUN-307`, `308`, `311` |
| Q15 print format | [17](17-stmt-print.md#4xx--output-format-q15) `T-PRN-4xx`, [01](01-lexical-and-literals.md#0xx--literal-forms) `T-LEX-013`–`016` |
| Q16 `//` comments only | [01](01-lexical-and-literals.md#3xx--statement-separators-and-program-structure) `T-LEX-308`, `309`, `902`, `906` |
| Q17 arity mismatch | [14](14-type-functions.md#1xx--invalid-literals-and-calls) `T-FUN-110`, `111`, `113` |
| Q18 no value → `none` | [14](14-type-functions.md#3xx--return) `T-FUN-302`, `305`, `310` |
| Q19 closures and recursion | [14](14-type-functions.md#2xx--scope-inside-a-function-closures) `T-FUN-204`–`215`, [custom/16](custom/16-recursion-and-closures) |
| Q20 `is` on any `Primary` | [11](11-op-type-check-is.md#1xx--malformed-indicators-and-operands) `T-IS-108`, `109`, `111`, `112`, [18](18-expression-precedence.md#4xx--grammar-corrections) `T-PREC-403`–`405`, `408`–`410` |
| Q21 no input | every program in the suite |
| Q22 identifiers | [01](01-lexical-and-literals.md#4xx--identifiers) `T-LEX-4xx` — the base rule; extended to every other place `IDENT` is declared: [02](02-declarations-and-scope.md#9xx--decided) `T-DECL-108` (syntax error wins even over an undeclared name), [13](13-type-tuples.md#1xx--invalid-literals-and-access) `T-TUP-113` (no digit-named tuple element), [14](14-type-functions.md#1xx--invalid-literals-and-calls) `T-FUN-104` (no digit-named parameter), [16](16-stmt-loops.md#1xx--for-loop-over-a-range) `T-LOOP-117` (no digit-named loop variable) |
| Q23 string content | [01](01-lexical-and-literals.md#9xx--decided) `T-LEX-903`–`905` |
| Q24 syntax vs runtime error | [02](02-declarations-and-scope.md#1xx--invalid-declarations) `T-DECL-101`, [03](03-assignment.md#1xx--invalid-assignment-targets) `T-ASGN-108`, [13](13-type-tuples.md#1xx--invalid-literals-and-access) `T-TUP-101`, [14](14-type-functions.md#1xx--invalid-literals-and-calls) `T-FUN-105`, [16](16-stmt-loops.md#4xx--exit) `T-LOOP-405`–`407`, `411`, `412` |
| Q25 initialiser scope | [02](02-declarations-and-scope.md#2xx--scope-shadowing-dynamic-type) `T-DECL-210`, `904`, [14](14-type-functions.md#2xx--scope-inside-a-function-closures) `T-FUN-213` |
| Q26 aliasing | [03](03-assignment.md#2xx--semantics) `T-ASGN-203`, `208`–`210`, [02](02-declarations-and-scope.md#2xx--scope-shadowing-dynamic-type) `T-DECL-211`, [12](12-type-arrays.md#4xx--edge-cases) `T-ARR-403`, [13](13-type-tuples.md#4xx--edge-cases) `T-TUP-408` |
| Q27 64-bit integers | [04](04-op-addition.md#9xx--decided) `T-ADD-903`, [05](05-op-subtraction.md#9xx--decided) `T-SUB-901`, [06](06-op-multiplication.md#9xx--decided) `T-MUL-901`, `903`, [07](07-op-division.md#9xx--decided) `T-DIV-903`, [19](19-implicit-conversions.md#9xx--decided) `T-CONV-904` |
| Q28 IEEE doubles | [05](05-op-subtraction.md#9xx--decided) `T-SUB-902`, `903`, [06](06-op-multiplication.md#9xx--decided) `T-MUL-902`, [08](08-op-comparison.md#9xx--decided) `T-CMP-902`, [17](17-stmt-print.md#4xx--output-format-q15) `T-PRN-403`, `414` |
| Q29 no short-circuit | [10](10-op-logical.md#4xx--evaluation-order) `T-LOG-4xx`, `T-LOG-111` |
| Q30 one logical level | [10](10-op-logical.md#2xx--precedence-and-associativity) `T-LOG-201`, [18](18-expression-precedence.md#1xx--associativity) `T-PREC-108` |
| Q31 what `none` may do | [11](11-op-type-check-is.md#4xx--none-handling) `T-IS-410`–`418`, [17](17-stmt-print.md#1xx--several-expressions-and-invalid-forms) `T-PRN-109` |
| Q32 ascending key order | [12](12-type-arrays.md#2xx--dynamic-size-and-associative-keys) `T-ARR-211`, `212`, [16](16-stmt-loops.md#2xx--for-loop-over-an-aggregate) `T-LOOP-210`, `212`, `213`, [17](17-stmt-print.md#4xx--output-format-q15) `T-PRN-409` |
| Q33 no empty tuple literal | [13](13-type-tuples.md#1xx--invalid-literals-and-access) `T-TUP-108` |
| Q34 boolean conditions only | [15](15-stmt-conditionals.md#1xx--invalid-conditions-and-forms) `T-IF-101`–`104`, `111`, [16](16-stmt-loops.md#0xx--while-loop) `T-LOOP-005`, [19](19-implicit-conversions.md#4xx--conversions-that-do-not-happen) `T-CONV-411` |
| Q35 no empty body | [15](15-stmt-conditionals.md#1xx--invalid-conditions-and-forms) `T-IF-108`, `109`, [16](16-stmt-loops.md#0xx--while-loop) `T-LOOP-006` |
| Q36 `IfShort` extent | [15](15-stmt-conditionals.md#2xx--short-form-if-) `T-IF-206`–`212` |
| Q37 descending range | [16](16-stmt-loops.md#1xx--for-loop-over-a-range) `T-LOOP-106` |
| Q38 loop variable assignable | [16](16-stmt-loops.md#1xx--for-loop-over-a-range) `T-LOOP-108`, `115` |
| Q39 inclusive bounds | [16](16-stmt-loops.md#1xx--for-loop-over-a-range) `T-LOOP-101`, `105`, `113` |
| Q41 call as a statement | [14](14-type-functions.md#0xx--function-literals-and-calls) `T-FUN-009`, `T-FUN-018`, `T-FUN-019`, [03](03-assignment.md#1xx--invalid-assignment-targets) `T-ASGN-107` |
| Q40 no call on a literal | [14](14-type-functions.md#01x--functions-as-values) `T-FUN-016`, [18](18-expression-precedence.md#4xx--grammar-corrections) `T-PREC-413`, `414` |

---

### Resolved: the language reads no input

Every program in this suite has its data hard-coded, because the language has **no way
to read any** — `print` is the only I/O construct, and
[Q21](../Language%20D.md#11-spec-ambiguities-and-decisions) keeps it that way. A test is
therefore a single source file, never a (program, stdin) pair, and the suite needs no
fixture files. The options that were considered are kept in
[Custom.md](custom/Custom.md#resolved-question-do-we-need-to-read-input).
