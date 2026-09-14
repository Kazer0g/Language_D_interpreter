# Block 14 — Functions and `return`

Prefix `T-FUN` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§7.7 Function](../Language%20D.md#77-function), [§4.7 Return](../Language%20D.md#47-return), [§6 References](../Language%20D.md#6-references-accessors)

**Program**: [`14-type-functions`](14-type-functions) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
FunctionLiteral
    : func [ ( IDENT { , IDENT } ) ] FunBody
FunBody
    : is Body end
    | => Expression
Return
    : return [ Expression ]
Call
    : Reference ( Expression { , Expression } )
```

Functions are treated as **literals** — constant values that can be assigned to variables
and passed as arguments to other functions. **The only operation defined for functions is
a call.**

## 0xx — Function literals and calls

| ID | Program | Expected |
|---|---|---|
| T-FUN-001 | `var f := func(x) => x + 1` <br> `print f(1)` | `2` — expression body |
| T-FUN-002 | `var f := func(a, b) is return a + b end` <br> `print f(1, 2)` | `3` — statement body |
| T-FUN-003 | `var f := func => 7` <br> `print f()` | `7` — the parameter list is optional |
| T-FUN-004 | `var f := func() => 7` <br> `print f()` | `7` — an empty parameter list is also allowed |
| T-FUN-005 | `var f := func is return 7 end` <br> `print f()` | `7` — no parameters, statement body |
| T-FUN-006 | `var f := func(a, b, c) => a + b + c` <br> `print f(1, 2, 3)` | `6` — several parameters |
| T-FUN-007 | `var f := func(x) => x * 2` <br> `print f(1 + 2)` | `6` — arguments are expressions |
| T-FUN-008 | `var f := func(x) => x` <br> `print f("s"), f(1), f(true)` | `s` `1` `true` — parameters are dynamically typed |
| T-FUN-009 | `var f := func(x) is print x end` <br> `f(1)` | `1` — a call as a statement; the one expression form that is a `Statement` ([Q41](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-010 | `var f := func => 0` <br> `print f is func` | `true` — the `func` type indicator |

## 01x — Functions as values

| ID | Program | Expected |
|---|---|---|
| T-FUN-011 | `var apply := func(f, v) => f(v)` <br> `var inc := func(x) => x + 1` <br> `print apply(inc, 10)` | `11` — a function passed as an argument |
| T-FUN-012 | `var make := func => func(x) => x * 2` <br> `print make()(5)` | `10` — a function returned from a function |
| T-FUN-013 | `var a := [func => 1, func => 2]` <br> `print a[2]()` | `2` — a function stored in an array |
| T-FUN-014 | `var t := {f := func => 3}` <br> `print t.f()` | `3` — a function stored in a tuple |
| T-FUN-015 | `var f := func => 1` <br> `var g := f` <br> `print g()` | `1` — a function value is assignable |
| T-FUN-016 | `print (func(x) => x + 1)(1)` | syntax error — a call accessor hangs off a `Reference` only, and `( Expression )` is not one ([Q40](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-017 | `var f := func(x) => x` <br> `print f(f)(1)` | `1` — `f` returns its argument, itself, which is then called |
| T-FUN-018 | `var t := {f := func is print 1 end}` <br> `t.f()` | `1` — a call statement through an accessor chain |
| T-FUN-019 | `var f := func => 5` <br> `f() + 1` | syntax error — only a **call** is a statement, not an expression built on one ([Q41](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-020 | `var f := func => {x := 1}` <br> `f().x` | syntax error — the call must be the last accessor |

## 1xx — Invalid literals and calls

| ID | Program | Expected |
|---|---|---|
| T-FUN-101 | `var f := func(x)` | syntax error — a `FunBody` is required |
| T-FUN-102 | `var f := func(x) => ` | syntax error — `=>` requires an expression |
| T-FUN-103 | `var f := func(x) is return x` | syntax error — `is` body must be closed with `end` |
| T-FUN-104 | `var f := func(1) => 1` | syntax error — parameters are identifiers, and `1` lexes as `INTEGER`, not `IDENT` ([Q22](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-105 | `var f := func(x, x) => x` | runtime error — duplicate parameter name, raised when the literal is evaluated ([Q24](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-106 | `var f := func(x) is x + 1 end` | syntax error — a `Body` holds statements, and a bare expression is not a `Statement` |
| T-FUN-107 | `var a := 1` <br> `print a(1)` | runtime error — calling a non-function |
| T-FUN-108 | `var f := func => 0` <br> `print f + f` | runtime error — the only operation on a function is a call |
| T-FUN-109 | `var f := func => 0` <br> `print f = f` | runtime error — no equality on functions |
| T-FUN-110 | `var f := func(x) => x` <br> `print f()` | runtime error — too few arguments ([Q17](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-111 | `var f := func(x) => x` <br> `print f(1, 2)` | runtime error — too many arguments |
| T-FUN-112 | `var f := func => 0` <br> `print f` | `func` — a function value renders as the word `func` ([Q15](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-113 | `var f := func => 0` <br> `print f(1)` | runtime error — an argument to a function with no parameters |

## 2xx — Scope inside a function: closures

A function literal captures the scopes enclosing it **by reference**
([Q19](../Language%20D.md#11-spec-ambiguities-and-decisions)): the body sees those
variables with their values at call time, and may assign them. Parameters and the body's
own declarations live in a fresh scope per call.

| ID | Program | Expected |
|---|---|---|
| T-FUN-201 | `var x := 1` <br> `var f := func is var x := 9; return x end` <br> `print f(), x` | `9` `1` — the body is a scope; its own `x` hides the outer one |
| T-FUN-202 | `var f := func(x) is return x end` <br> `print f(5)` | `5` — a parameter is visible in the body |
| T-FUN-203 | `var f := func is var y := 1; return y end` <br> `print f()` <br> `print y` | `1` then runtime error — `y` is dead outside the body |
| T-FUN-204 | `var x := 1` <br> `var f := func => x + 1` <br> `print f()` | `2` — the body reads the enclosing `x` |
| T-FUN-205 | `var x := 1` <br> `var f := func => x` <br> `x := 2` <br> `print f()` | `2` — captured by reference: the value at call time |
| T-FUN-206 | `var f := func(n) is if n <= 1 then return 1 end return n * f(n - 1) end` <br> `print f(5)` | `120` — recursion: `f` is looked up when the body runs |
| T-FUN-207 | `var c := 0` <br> `var bump := func is c := c + 1 end` <br> `bump(); bump()` <br> `print c` | `2` — the body assigns the enclosing variable |
| T-FUN-208 | `var adder := func(k) => func(x) => x + k` <br> `print adder(10)(5)` | `15` — the inner literal captures the parameter `k` of the outer call |
| T-FUN-209 | `var adder := func(k) => func(x) => x + k` <br> `var a1 := adder(1), a2 := adder(2)` <br> `print a1(0), a2(0)` | `1` `2` — each call has its own `k` |
| T-FUN-210 | `var x := 1` <br> `var f := func(x) => x` <br> `print f(5), x` | `5` `1` — a parameter hides a captured name of the same name |
| T-FUN-211 | `var fib := func(n) is if n <= 2 then return 1 end return fib(n-1) + fib(n-2) end` <br> `print fib(10)` | `55` — two recursive calls per activation |
| T-FUN-212 | `var g` <br> `var f := func => g()` <br> `g := func => 7` <br> `print f()` | `7` — `g` is resolved at call time, so it may be assigned after `f` is created |
| T-FUN-213 | `var f := func => y` <br> `var y := 3` <br> `print f()` | `3` — `y` was declared after the literal but before the call; names are resolved at call time in the captured scope. This is the same mechanism that makes `T-FUN-206` work: `f` itself is declared after its literal is evaluated ([Q25](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-214 | `var mk := func is var n := 0; return func is n := n + 1; return n end end` <br> `var c := mk()` <br> `print c(), c()` | `1` `2` — a captured local outlives the call that created it |
| T-FUN-215 | `var f := func => y` <br> `print f()` | runtime error — `y` is not declared at call time |

## 3xx — `return`

| ID | Program | Expected |
|---|---|---|
| T-FUN-301 | `var f := func is return 5 end` <br> `print f()` | `5` — return with a value |
| T-FUN-302 | `var f := func is return end` <br> `print f() is none` | `true` — return without a value yields `none` ([Q18](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-303 | `var f := func is print "a"; return 1; print "b" end` <br> `print f()` | `a` `1` — statements after `return` do not run |
| T-FUN-304 | `var f := func(a, b) is if a > b then return a end return b end` <br> `print f(3, 7)` | `7` — early return from a branch |
| T-FUN-305 | `var f := func is print "a" end` <br> `print f() is none` | `a` then `true` — falling off the end yields `none` |
| T-FUN-306 | `var f := func is var i := 0` <br> `  loop i := i + 1; if i = 3 => return i end` <br> `end` <br> `print f()` | `3` — `return` leaves the function from inside a loop |
| T-FUN-307 | `return 1` | syntax error — `return` at program level ([Q14](../Language%20D.md#11-spec-ambiguities-and-decisions), [Q24](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-FUN-308 | `if true then return 1 end` | syntax error — `return` outside a function body |
| T-FUN-309 | `var f := func => 1 + 2` <br> `print f()` | `3` — `=>` returns the expression value implicitly |
| T-FUN-310 | `var f := func is print "x" end` <br> `print f()` | `x` then `none` — the `none` result is printable |
| T-FUN-311 | `loop return 1 end` | syntax error — a loop body is not a function body |

## 4xx — Edge cases

| ID | Program | Expected |
|---|---|---|
| T-FUN-401 | `var f := func => func => func => 1` <br> `print f()()()` | `1` — chained calls |
| T-FUN-402 | `var f := func(x) => x` <br> `print f(f)( 1 )` | `1` — passing a function to itself; same as `T-FUN-017` with spacing inside the call |
| T-FUN-403 | `var f := func(x) is return x end` <br> `print f([1,2])[2]` | `2` — indexing a call result |
| T-FUN-404 | `var f := func(x) is return {a := x} end` <br> `print f(3).a` | `3` — dot access on a call result |
| T-FUN-405 | `var f := func(x, y) => x` <br> `print f(1, 2 + 3)` | `1` — a second argument that is an expression |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-FUN-901 | [Q17](../Language%20D.md#11-spec-ambiguities-and-decisions) — arity mismatch on a call. | Runtime error; the counts must be equal. | `T-FUN-110`, `T-FUN-111`, `T-FUN-113` |
| T-FUN-902 | [Q18](../Language%20D.md#11-spec-ambiguities-and-decisions) — result of `return` without a value and of falling off the end. | `none` in both cases. | `T-FUN-302`, `T-FUN-305`, `T-FUN-310` |
| T-FUN-903 | [Q19](../Language%20D.md#11-spec-ambiguities-and-decisions) — closures and recursion. | Lexical closures by reference; names resolved at call time; recursion follows. | `T-FUN-204`–`T-FUN-215` |
| T-FUN-904 | [Q14](../Language%20D.md#11-spec-ambiguities-and-decisions) — `return` outside a function. | Syntax error, like `exit` outside a loop. | `T-FUN-307`, `T-FUN-308`, `T-FUN-311` |
| T-FUN-905 | [Q40](../Language%20D.md#11-spec-ambiguities-and-decisions) — can a function literal be called directly? | No — syntax error; bind it first. | `T-FUN-016` |
| T-FUN-906 | [Q41](../Language%20D.md#11-spec-ambiguities-and-decisions) — is a call a statement, given `Statement` lists none? | Yes, a call and only a call; its value is discarded. | `T-FUN-009`, `T-FUN-018`–`T-FUN-020`, [T-ASGN-107](03-assignment.md#1xx--invalid-assignment-targets) |
