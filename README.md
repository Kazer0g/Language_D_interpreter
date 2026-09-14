# Language D — Specification

Dynamic, interpreted language. Source: [Project D.pdf](Project%20D.pdf).

Every topic below is given as **General form** (the production/rule exactly as the
description states it) followed by an **Example**, and a **Tests** link to the block
of the test suite that verifies it. Where the description is silent or contradicts
itself, a **Decision** paragraph states what this project does; every decision is
numbered and collected in [§11](#11-spec-ambiguities-and-decisions).

Test suite index: [tests/Tests.md](tests/Tests.md)

---

## Table of Contents

1. [Common Description](#1-common-description)
2. [Program Structure](#2-program-structure)
3. [Declarations](#3-declarations)
4. [Statements](#4-statements)
5. [Expressions](#5-expressions)
6. [References (Accessors)](#6-references-accessors)
7. [Types and Literals](#7-types-and-literals)
8. [Operator Semantics](#8-operator-semantics)
9. [Implicit Type Conversions](#9-implicit-type-conversions)
10. [Full Grammar Summary](#10-full-grammar-summary)
11. [Spec Ambiguities and Decisions](#11-spec-ambiguities-and-decisions)

---

## 1. Common Description

- **Dynamic typing**: object types are not specified and can change during program execution.
- The language assumes **interpretation**.
- Major notion: **variable** & **literal** (constant).
- Program structure: a sequence of declarations and statements.
- Builtin types: `integer`, `real`, `boolean`, `string`.
- User-defined types: `array`, `tuple`, `function`.
- Implicit type conversions are supported.
- Statements: assignment, `if`/`while`/`loop`, `exit`/`return`, output.

Because the language is dynamic, **the control of operand types and the selection of
the appropriate operation algorithm is performed dynamically during program
execution** — type errors are runtime errors, not compile-time errors.

---

## 2. Program Structure

The program is a sequence of statements. Statements can be separated by the semicolon
character or by newline characters. Statements execute one after the other, starting
with the first in text statement.

There are two categories of constructs: variable **declaration** and **statement**.

**General form**

```
Program : { Statement [ ; ] }
```

**Example**

```d
var a := 1; var b := 2
print a + b
```

**Decision** ([Q16](#11-spec-ambiguities-and-decisions), [Q22](#11-spec-ambiguities-and-decisions),
[Q24](#11-spec-ambiguities-and-decisions)) — lexical rules the description leaves out:

- `//` starts a comment that runs to the end of the line. There is no block comment.
- `IDENT : ( letter | _ ) { letter | digit | _ }`; identifiers and keywords are
  case-sensitive.
- Whitespace between tokens is insignificant, except that a newline separates statements
  and ends an `IfShort` body.
- Errors come in two classes. A **syntax error** is reported by the parser and nothing
  executes: grammar violations, `exit` outside a loop, `return` outside a function, a
  call as an assignment target. A **runtime error** is raised when the offending statement
  executes and aborts the program; output already produced stays.

**Tests** → [01-lexical-and-literals.md](tests/01-lexical-and-literals.md) (`T-LEX-3xx` separators)

---

## 3. Declarations

Declaration of a variable introduces a new named object to the current scope, perhaps
with an initial value.

### 3.1 Variable Declaration

Declaration of a variable could be in **any position** within the program. When
declaring a variable you can set its initial value; the initial value can subsequently
be changed using the assignment operator. If the initial value is not set, the variable
has the special value `none`.

A value `none` **cannot act as an operand of operations**; the only action on a variable
with this value is type checking using the `is` operation.

The type of the variable is **not fixed** during declaration — within its scope a
variable can take any type of value allowed in the language.

**General form**

```
Declaration
    : var VariableDefinition { , VariableDefinition }
VariableDefinition
    : IDENT [ := Expression ]
```

**Example**

```d
var a                       // a is none
var b := 10                 // b is integer
var c := 1, d := 2.5, e     // three definitions in one declaration
b := "now a string"         // the type of a variable is not fixed
```

**Decision** ([Q22](#11-spec-ambiguities-and-decisions), [Q24](#11-spec-ambiguities-and-decisions),
[Q25](#11-spec-ambiguities-and-decisions), [Q26](#11-spec-ambiguities-and-decisions)):

- `IDENT` (the name in `VariableDefinition`) can never start with a digit — `var 1a := 1`
  is a syntax error ([T-LEX-405](tests/01-lexical-and-literals.md#4xx--identifiers)),
  caught by the lexer before the declaration is even reached — even before an
  independently-bad initialiser is looked at ([T-DECL-108](tests/02-declarations-and-scope.md#1xx--invalid-declarations)).
- The initialiser is evaluated **before** the new name is introduced, so
  `var x := x + 1` in a nested scope reads the enclosing `x`.
- Declaring a name that already exists in the same scope is a **runtime error**, raised
  when the second declaration executes; so is a repeated name inside one declaration.
- `var b := a` **aliases** an array or tuple: both names denote the same object and a
  change through one is visible through the other. Simple values are immutable, so for
  them the distinction never shows.

**Tests** → [02-declarations-and-scope.md](tests/02-declarations-and-scope.md)

### 3.2 Scope and Visibility

A variable introduced by a declaration is **active within its scope**: from the point of
its declaration to the end of that scope. Within the scope, only one variable with the
given name is allowed.

If a certain scope contains other (nested) scopes, then variables declared in a nested
scope may have names that match the names in the enclosing scope. In this case,
variables from the nested scope **hide** variables with the same names from the
enclosing scope.

**General form**

```
Scope = program body | function body | loop body | branch body
Visibility = [ point of declaration .. end of enclosing scope ]
```

**Example**

```d
var x := 1
if true then
    var x := 2       // nested scope: hides the outer x
    print x          // 2
end
print x              // 1
```

**Decision** ([Q19](#11-spec-ambiguities-and-decisions), [Q25](#11-spec-ambiguities-and-decisions)):
"the point of its declaration" is the moment the declaration **executes**. A function
body resolves its free names when it is called, so it sees a variable of the enclosing
scope even if that variable was declared after the function literal — that is what
lets `var f := func … f(…) … end` recurse.

**Tests** → [02-declarations-and-scope.md](tests/02-declarations-and-scope.md) (`T-DECL-2xx`)

---

## 4. Statements

The language defines a common set of statements: besides the declarations (which also
have the status of statements), there are assignment, conditional statement, two kinds
of repeat statements, the function return statement and the print statement.

**General form**

```
Statement
    : Declaration
    | Assignment
    | If
    | IfShort
    | Loop
    | Exit
    | Return
    | Print
    | Call                  // added by decision Q41
```

**Decision** ([Q41](#11-spec-ambiguities-and-decisions)): a **call** may stand alone as a
statement — `Call : Reference ( Expression { , Expression } )`, value discarded. It is
the only expression form that can; `a = 2` or `1 + 1` on a line of their own are syntax
errors.

**Example**

```d
var i := 0                  // Declaration
i := i + 1                  // Assignment
if i > 0 then print i end   // If
print i                     // Print
var hello := func is print "hi" end
hello()                     // Call — a statement, its (none) result discarded
```

### 4.1 Assignment

Calculates a new value for some variable. The left-hand side is a `Reference`, which is
what distinguishes references from other operands — they can act as recipients of
values in assignment operators.

**General form**

```
Assignment
    : Reference := Expression
```

**Example**

```d
var a := 1
a := a + 1                  // simple variable
var t := []
t[10] := 25                 // array element
var f := func(x) => x + 1
a := f(3)                   // value produced by a call
```

**Decision** ([Q11](#11-spec-ambiguities-and-decisions), [Q24](#11-spec-ambiguities-and-decisions),
[Q26](#11-spec-ambiguities-and-decisions)):

- A tuple element is a valid target: `t.a := 5` and `t.1 := 5` replace the value of an
  **existing** element. A name that is not in the tuple would change its structure and
  is a runtime error.
- A call cannot be the last accessor of a target: `f() := 1` is a **syntax error**, even
  though `Reference` derives it.
- Assigning an array or tuple **aliases** it (see [§3.1](#31-variable-declaration)).
  Assigning to an array key that does not exist creates it.

**Tests** → [03-assignment.md](tests/03-assignment.md)

### 4.2 If

**General form**

```
If
    : if Expression then Body [ else Body ] end
```

**Example**

```d
var n := 5
if n > 0 then
    print "positive"
else
    print "non-positive"
end
```

**Decision** ([Q34](#11-spec-ambiguities-and-decisions), [Q35](#11-spec-ambiguities-and-decisions)):
the condition must evaluate to a **boolean** — any other type, including `none`, is a
runtime error; there is no truthiness rule. Each `Body` holds at least one statement, so
an empty branch is a syntax error.

**Tests** → [15-stmt-conditionals.md](tests/15-stmt-conditionals.md)

### 4.3 IfShort

**General form**

```
IfShort
    : if Expression => Body
```

**Example**

```d
var i := 100
if i = 100 => exit
```

**Decision** ([Q36](#11-spec-ambiguities-and-decisions)): the short form has no `else`
and no `end`. Its body extends **to the end of the line**, or to the first `end` /
`else` on that line, whichever comes first; semicolons inside it separate statements of
the body. So `if ok => var a := 1; print a` runs both statements when `ok` holds, and
the description's own `loop … if i = 100 => exit end` stays a valid one-liner.

**Tests** → [15-stmt-conditionals.md](tests/15-stmt-conditionals.md) (`T-IF-2xx`)

### 4.4 Body

**General form**

```
Body
    : Statement { [ ; ] Statement }
```

**Example**

```d
if true then
    var a := 1; var b := 2
    print a + b
end
```

**Decision** ([Q35](#11-spec-ambiguities-and-decisions)): a `Body` holds **at least one**
statement, as the production says — `if c then else … end` and `while c loop end` are
syntax errors. Every `Body` (branch, loop body, function body) is a **scope**
([§3.2](#32-scope-and-visibility)).

**Tests** → [01-lexical-and-literals.md](tests/01-lexical-and-literals.md) (`T-LEX-3xx`)

### 4.5 Loop

Two kinds of repeat statements.

**General form**

```
Loop
    : WhileLoop
    | ForLoop
WhileLoop
    : while Expression LoopBody
ForLoop
    : [ ForHeader ] LoopBody
ForHeader
    : for [ IDENT in ] Expression [ .. Expression ]
LoopBody
    : loop Body end
```

Rules:

- The `ForHeader` part in the for-loop **can be omitted**. In that case the loop is
  **infinite**; the only way of exiting such a loop is the `exit` statement.
- **Two expressions** in the loop header: the loop iterates over the range from `v1`
  until `v2`, where `v1` and `v2` are the current **integer** values of the corresponding
  expressions.
- If the **identifier** is specified in the loop header then it introduces a variable
  that takes values from the range sequentially on each loop iteration. **The scope of
  that variable is the loop body.**
- **A single expression** in the loop header: its type should be either an **array** or a
  **tuple**. In that case the loop iterates over all elements of the corresponding
  aggregate.

**Example**

```d
var i := 0
loop                        // infinite loop, no ForHeader
    print "Hello"
    i := i + 1
    if i = 100 => exit
end

for 1..3 loop               // range, no loop variable
    print "Hello"
end

var array := [1,2,3,4,5]
var sum := 0
for i in array loop         // iteration over an aggregate
    sum := sum + i
end

while sum > 0 loop          // while loop
    sum := sum - 1
end
```

**Decision** ([Q12](#11-spec-ambiguities-and-decisions), [Q13](#11-spec-ambiguities-and-decisions),
[Q22](#11-spec-ambiguities-and-decisions), [Q32](#11-spec-ambiguities-and-decisions),
[Q34](#11-spec-ambiguities-and-decisions), [Q37](#11-spec-ambiguities-and-decisions)–[Q39](#11-spec-ambiguities-and-decisions)):

- The `IDENT` in `ForHeader : for [ IDENT in ] …` is the same identifier token as
  everywhere else — it cannot start with a digit. `for 1i in 1..3 loop … end` is a
  syntax error; `1i` lexes as `INTEGER` then `IDENT`, not one name.
- A range `v1..v2` is **inclusive at both ends** and visits `v1, v1+1, …, v2`; when
  `v1 > v2` the body runs **zero times**. Both bounds must be **integers** — a real or
  any other type is a runtime error. The bounds are evaluated once, before the first
  iteration.
- Iterating an **array** visits its values in **ascending key order**, skipping gaps.
  The keys to visit are fixed at loop entry; the body may write to the array without
  changing the number of iterations. Iterating a **tuple** visits its values in
  positional order; element **names are not reachable** from the loop variable. In both
  cases the loop variable holds a copy of the element's value, not the slot.
- The loop variable is an ordinary variable of the body's scope: it **may be assigned**,
  and the next iteration overwrites it with the next value regardless.
- A `while` condition must be boolean, as for `if`.

**Tests** → [16-stmt-loops.md](tests/16-stmt-loops.md)

### 4.6 Exit

The `exit` statement can appear **only within loop statements**.

**General form**

```
Exit
    : exit
```

**Example**

```d
var i := 0
loop
    i := i + 1
    if i = 10 => exit
end
print i                     // 10
```

**Decision** ([Q24](#11-spec-ambiguities-and-decisions)): `exit` leaves the
**innermost** enclosing loop. An `exit` that is not inside a loop body — at program
level, in a branch outside any loop, or in a function body (a function body starts a
fresh context even when the literal sits inside a loop) — is a **syntax error**,
detected by the parser.

**Tests** → [16-stmt-loops.md](tests/16-stmt-loops.md) (`T-LOOP-4xx`)

### 4.7 Return

The return statement performs leaving a function, perhaps with returning a result value
to the calling context.

**General form**

```
Return
    : return [ Expression ]
```

**Example**

```d
var max := func(a, b) is
    if a > b then return a end
    return b
end
print max(3, 7)             // 7

var noResult := func() is
    return                  // leaves the function without a value
end
```

**Decision** ([Q14](#11-spec-ambiguities-and-decisions), [Q18](#11-spec-ambiguities-and-decisions),
[Q24](#11-spec-ambiguities-and-decisions)): `return` without an expression, and running
off the end of an `is … end` body, both produce **`none`** as the call's value. A
`return` that is not inside a function body is a **syntax error**, exactly like `exit`
outside a loop.

**Tests** → [14-type-functions.md](tests/14-type-functions.md) (`T-FUN-3xx`)

### 4.8 Print

Outputs values to the console.

**General form**

```
Print
    : print Expression { , Expression }
```

**Example**

```d
print "Hello"
print 1, 2, 3
print "sum = ", 1 + 2
```

**Decision** ([Q15](#11-spec-ambiguities-and-decisions), [Q28](#11-spec-ambiguities-and-decisions),
[Q31](#11-spec-ambiguities-and-decisions)) — the output format:

| Value       | Rendered as                                                                                                                                       | Example                              |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------ |
| integer     | decimal digits, `-` for negatives                                                                                                                 | `42`, `-7`                           |
| real        | the **shortest** digit string that reads back to the same double, **always containing a `.`**; no exponent form                                   | `3.0`, `0.5`, `12.34`, `0.3333333333333333` |
| string      | its characters, **no quotes** — at every nesting depth                                                                                            | `hello`                              |
| boolean     | `true` / `false`                                                                                                                                  | `true`                               |
| array       | `[` values in ascending key order, separated by `, ` `]`; gaps are not shown; empty is `[]`                                                       | `[1, 2, 3]`, `[]`, `[[1], [2]]`      |
| tuple       | `{` elements separated by `, `, a named element as `name := value` `}`                                                                            | `{a := 1, 2}`                        |
| function    | the word `func`                                                                                                                                   | `func`                               |
| none        | the word `none` — printing `none` is allowed                                                                                                      | `none`                               |

The values of one `print` are separated by **a single space** and followed by **one
newline**. `print ""` produces an empty line. There is no output statement that omits the
newline. All the expressions of one `print` are evaluated, left to right, **before**
anything is written, so a runtime error in the last one leaves the line unprinted. This
rendering is the only "conversion to string" in the language: it does not create string
values and `1 + "2"` stays an error.

```d
print 1, "s", true, 1.5     // 1 s true 1.5
print [1, "a", [2]]         // [1, a, [2]]
print {a := 1, 2.0}         // {a := 1, 2.0}
```

**Tests** → [17-stmt-print.md](tests/17-stmt-print.md)

---

## 5. Expressions

*Expression* is a syntax construct defining an algorithm for calculating new values. The
structure of expressions is traditional and includes several types of operands and a set
of infix and prefix operations on operands.

The productions below also define **operator precedence**, from lowest (`Expression`) to
highest (`Primary`).

**Decision** ([Q3](#11-spec-ambiguities-and-decisions), [Q4](#11-spec-ambiguities-and-decisions),
[Q20](#11-spec-ambiguities-and-decisions), [Q30](#11-spec-ambiguities-and-decisions)):
three productions are corrected — `Factor`, `Unary` and `Primary`. The description's
version is shown under each heading with the corrected form beside it; the corrected
grammar is the one in [§10](#10-full-grammar-summary) and the one the interpreter
implements. `or`, `and` and `xor` stay at one level as the description says.

### 5.1 Expression (logical level)

**General form**

```
Expression
    : Relation { ( or | and | xor ) Relation }
```

All three operators share **one level** and associate **left to right** — `and` does
**not** bind tighter than `or` ([Q30](#11-spec-ambiguities-and-decisions)). Both operands
are always evaluated; there is no short-circuiting ([Q29](#11-spec-ambiguities-and-decisions)).

**Example**

```d
print true and false        // false
print true or false xor true    // (true or false) xor true = false
print true or false and false   // (true or false) and false = false
```

### 5.2 Relation (comparison level)

Note the comparison is **optional and non-repeating** — at most one comparison operator
per relation, so relations do not chain.

**General form**

```
Relation
    : Factor [ ( < | <= | > | >= | = | /= ) Factor ]
```

**Example**

```d
print 1 < 2                 // true
print 3 /= 4                // true
```

### 5.3 Factor (additive level)

**General form**

```
Factor                              // as written in the description
    : Term { [ + | - ] Term }
Factor                              // corrected (Q3): the operator is not optional
    : Term { ( + | - ) Term }
```

**Example**

```d
print 1 + 2 - 3             // 0
// print 1 2                // syntax error — juxtaposition is not an operation
```

### 5.4 Term (multiplicative level)

**General form**

```
Term
    : Unary { ( * | / ) Unary }
```

**Example**

```d
print 2 * 3 / 4             // 1  (integer division rounds down)
```

### 5.5 Unary

**General form**

```
Unary                               // as written in the description
    : Reference
    | Reference is TypeIndicator
    | [ + | - | not ] Primary
Unary                               // corrected (Q4, Q20)
    : [ + | - | not ] Unary
    | Primary [ is TypeIndicator ]
```

As written, `Primary` does not derive `Reference`, so `-x` and `not x` on a variable
were not derivable, and `is` could not be applied to a literal or a parenthesised
expression. The corrected form lets a prefix operator apply to any operand, lets
prefixes stack (`- -5`), and lets `is` test any `Primary`. `is` stays at this level, so
it binds tighter than every binary operator and looser than a prefix operator:
`not a is int` is `not (a is int)`, and `a is int and true` is `(a is int) and true`.

**Example**

```d
var a := 5
print a                     // Reference
print a is int              // true
print -a                    // -5   unary minus on a Reference
print - -a                  // 5    prefixes stack
print not (a > 3)           // false
print 1 is int              // true — is on a literal
print (a + 1) is int        // true — is on a parenthesised expression
```

### 5.6 Primary

**General form**

```
Primary                             // as written in the description
    : Literal
    | FunctionLiteral
    | ( Expression )
Primary                             // corrected (Q4): a Reference is a Primary
    : Reference
    | Literal
    | FunctionLiteral
    | ( Expression )
```

Accessors (`[ ]`, `( )`, `.`) belong to `Reference` only, so they hang off an
identifier, never off a literal or a parenthesised expression:
`(func(x) => x)(1)` and `[1,2][1]` are syntax errors ([Q40](#11-spec-ambiguities-and-decisions)).

**Example**

```d
print 42                    // Literal
print func(x) => x * 2      // FunctionLiteral
print (1 + 2) * 3           // parenthesised Expression
var t := [1, 2]
print t[2]                  // Reference
```

### 5.7 TypeIndicator

**General form**

```
TypeIndicator
    : int
    | real
    | bool
    | string
    | none      // no type
    | [ ]       // vector type
    | { }       // tuple type
    | func      // functional type
```

**Example**

```d
var a := 1
print a is int              // true
print a is real             // false
var b
print b is none             // true
var arr := [1,2]
print arr is []             // true
var t := {x := 1}
print t is {}               // true
var f := func => 0
print f is func             // true
```

**Tests** → [11-op-type-check-is.md](tests/11-op-type-check-is.md), [18-expression-precedence.md](tests/18-expression-precedence.md)

---

## 6. References (Accessors)

The `Reference` production defines **accessors** — constructs that specify elementary
actions with variables: taking an array element, calling a function, and accessing
elements of objects. References can be operands of expressions and, unlike other
operands (constants, subexpressions, etc.), act as **recipients of values in assignment
operators**.

**General form**

```
Reference
    : IDENT
    | Reference [ Expression ]                      // array element
    | Reference ( Expression { , Expression } )     // call
    | Reference . IDENT                             // tuple element
    | Reference . IntegerLiteral                    // tuple element
```

**Example**

```d
var t := [1, 2, 3]
print t[1]                  // array element, numbering begins with one

var f := func(x) => x + 1
print f(10)                 // call

var p := {a := 5, b := "sss"}
print p.a                   // tuple element by name
print p.1                   // the same element, by position

var m := [ {a := 1}, {a := 2} ]
print m[2].a                // chained accessors
```

**Decision** ([Q24](#11-spec-ambiguities-and-decisions), [Q40](#11-spec-ambiguities-and-decisions),
[Q41](#11-spec-ambiguities-and-decisions)): a `Reference` is always rooted in an
identifier — accessors never attach to a literal or a parenthesised expression. As an
assignment target it must **not** end in a call (`f() := 1` is a syntax error); as a
statement on its own it **must** end in a call (`f()` and `t.f()` are statements,
`f().x` is not).

**Tests** → [12-type-arrays.md](tests/12-type-arrays.md), [13-type-tuples.md](tests/13-type-tuples.md), [14-type-functions.md](tests/14-type-functions.md)

---

## 7. Types and Literals

The language defines the values of **four simple types**, **two composite types**, and
**one special type**.

Simple types: `integer`, `real`, `string`, `boolean`.
Composite types: `array`, `tuple`.
Special: `none`. (Functions are values too — see 7.7.)

**General form**

```
Literal
    : INTEGER
    | REAL
    | STRING
    | Boolean
    | Tuple     // { a := 5, b := "sss", 12.34 }
    | Array     // [ 1, 2, 3 ]
    | none
```

**Tests** → [01-lexical-and-literals.md](tests/01-lexical-and-literals.md)

### 7.1 Integer

Integer literals are written as a sequence of decimal digits.

**General form**

```
INTEGER : digit { digit }       // decimal digits
```

**Example**

```d
var a := 0
var b := 42
var c := 1000000
```

**Decision** ([Q27](#11-spec-ambiguities-and-decisions)): integers are **64-bit signed**.
A result outside `-2^63 .. 2^63-1` is a runtime error; nothing wraps. A literal that does
not fit is a syntax error.

### 7.2 Real

Real literals are formed as **two sequences of decimal digits separated by the dot
character**. The first sequence denotes the integer part of the real, the second sequence
its fractional part.

**General form**

```
REAL : digit { digit } . digit { digit }
```

**Example**

```d
var a := 12.34
var b := 0.5
var c := 3.0
```

**Decision** ([Q28](#11-spec-ambiguities-and-decisions)): reals are **IEEE 754 doubles**
with ordinary double arithmetic, so `0.1 + 0.2 = 0.3` is `false`. There is no exponent
form (`1e5` is a syntax error). Printing is described in [§4.8](#48-print).

### 7.3 String

Strings are specified as a sequence of arbitrary characters enclosed in **single or
double quotes**.

**General form**

```
STRING : " { character } "
       | ' { character } '
```

**Example**

```d
var a := "hello"
var b := 'hello'
print a + b                 // hellohello
print "it's"                // it's — the other quote style is an ordinary character
```

**Decision** ([Q23](#11-spec-ambiguities-and-decisions)): there are **no escape
sequences** — `"a\nb"` is four characters, backslash included. A string literal must end
on the line it starts; a newline before the closing quote is a syntax error. Strings are
not indexable and have no length operation; the only operations are `+` and `=` / `/=`.

### 7.4 Boolean

Boolean values are represented by two keywords `true` and `false`.

**General form**

```
Boolean
    : true
    | false
```

**Example**

```d
var t := true
var f := false
print t and not f           // true
```

### 7.5 Array

**Array** is a linear composition of values of **any type**. The size of the array (number
of elements) is **not fixed and can be dynamically changed**. The array is presented as a
list of element values, separated by commas and enclosed in square brackets.

Access to the elements of the array is performed in the usual way: after the name of the
array in square brackets, the number of the element in the form of an **integer
expression** is specified. **The numbering of the elements of the array begins with one.**

Note that an array is a "real" **associative array** with keys representing integer
values. The values of the neighbouring keys in the array do not necessarily differ by
one.

**General form**

```
Array
    : [ [ Expression { , Expression } ] ]
Access
    : Reference [ Expression ]      // integer expression, 1-based
```

**Example**

```d
var a := [1, 2, 3]
print a[1]                  // 1  — numbering begins with one

var mixed := [1, 2.5, "s", true, [1], {x := 1}]   // any type

var t := []                 // empty array declaration
t[10] := 25
t[100] := func(x) => x + 1
t[1000] := {a := 1, b := 2.7}
```

**Decision** ([Q8](#11-spec-ambiguities-and-decisions), [Q9](#11-spec-ambiguities-and-decisions),
[Q26](#11-spec-ambiguities-and-decisions), [Q32](#11-spec-ambiguities-and-decisions)):

- Reading a key that holds no value yields **`none`** — that includes `a[0]`, negative
  keys, keys past the last element and gaps in a sparse array. Reading never creates a
  key; `a[k] is none` is the presence test. The index must be an integer; any other type
  is a runtime error.
- Writing to a missing key **creates** it; writing to an existing key replaces the value.
- `a + b` builds a **new** array: `a`'s values in ascending key order, then `b`'s, under
  fresh keys `1..n`. Neither operand changes.
- The order "ascending key" is used everywhere an array is walked: `+`, `print`, and
  `for v in a loop`.
- Arrays **alias** on assignment and argument passing.
- There is no operation that reports the number of elements or removes a key; count with
  a loop.

```d
var a := [1, 2, 3]
print a[4] is none          // true — never assigned
var t := []
t[10] := 1
print t + [2]               // [1, 2] — renumbered, the sparse key is not kept
var b := a
b[1] := 9
print a[1]                  // 9 — b aliases a
```

**Tests** → [12-type-arrays.md](tests/12-type-arrays.md)

### 7.6 Tuple

**Tuple** is a **fixed-size** collection of possibly named values of arbitrary types.
Element names, if they are specified, are **unique within the tuple**. A tuple is
specified as a comma-separated list of pairs of the form `name := value`. The name and
value are separated by the assignment token `:=`. The entire list is enclosed in braces.

The structure of tuples, unlike arrays, **cannot be modified**; the only way to change the
composition of the tuple elements is to **add another tuple to it**.

Access to the tuple elements is performed via **dot notation**, using the name of the
tuple variable and the name of the tuple element, or by using the **number** of the tuple
element within the tuple (**numbering starts with one**).

Named and unnamed elements in a tuple can follow in **any order**.

**General form**

```
Tuple
    : { TupleElement { , TupleElement } }
TupleElement
    : [ Identifier := ] Expression
Access
    : Reference . IDENT
    | Reference . IntegerLiteral
```

**Example**

```d
var c := 1, d := 2
var t := {a := 1, b := 2, c + d}
t := t + {e := 3}           // now t is {a := 1, b := 2, c+d, e := 3}

var x := t.b                // now x is 2
x := t.2                    // the same effect
var y2 := t.3               // now y2 has the value of c+d

var mixedOrder := {1, a := 2, "s"}   // named and unnamed in any order
```

**Decision** ([Q10](#11-spec-ambiguities-and-decisions), [Q11](#11-spec-ambiguities-and-decisions),
[Q13](#11-spec-ambiguities-and-decisions), [Q22](#11-spec-ambiguities-and-decisions),
[Q26](#11-spec-ambiguities-and-decisions), [Q33](#11-spec-ambiguities-and-decisions)):

- `t + u` builds a **new** tuple with `t`'s elements followed by `u`'s; positions
  continue across the join. If a name would occur twice in the result it is a **runtime
  error** — name uniqueness is an invariant of every tuple.
- A duplicate name inside one literal is likewise a runtime error.
- `t.a := v` and `t.1 := v` are allowed: they change a **value**, not the structure.
  Assigning to a name the tuple does not have is a runtime error.
- Reading a missing name or a position outside `1..size` is a runtime error (unlike
  arrays, a tuple has a fixed, known size).
- `{}` is only the type indicator after `is`; there is no empty tuple literal.
- **`Identifier` in `TupleElement` is the same token as `IDENT` everywhere else** in the
  grammar ([§2](#2-program-structure), [Q22](#11-spec-ambiguities-and-decisions)) — an
  identifier cannot start with a digit. A bare `1` always lexes as `INTEGER`, never as
  `IDENT`, so it can **never** be a tuple element's *name* — only its *value* (an unnamed
  element, `{1, 2, 3}`) or its *position* (`t.1`, via `IntegerLiteral`, a separate
  production). `{1 := 5}` is a **syntax error** for the same reason `var 1a := 1` is: the
  parser needs an `IDENT` before `:=` and finds an `INTEGER` instead. Position and name
  are never confusable — one is always a digit sequence, the other never starts with one.
- The position after `.` must be an integer **literal** — a computed position is not
  derivable.
- Tuples **alias** on assignment and argument passing, like arrays.

**Tests** → [13-type-tuples.md](tests/13-type-tuples.md)

### 7.7 Function

Functions in the D language are treated as **literals** (similarly to integer/real
constants) in the sense that they are considered as **constant values that can be
assigned to variables and passed as arguments to other functions**. The only operation
defined for functions is a **call**.

**General form**

```
FunctionLiteral
    : func [ ( IDENT { , IDENT } ) ] FunBody
FunBody
    : is Body end
    | => Expression
```

**Example**

```d
var inc := func(x) => x + 1             // expression body
print inc(1)                            // 2

var sum := func(a, b) is                // statement body
    return a + b
end
print sum(1, 2)                         // 3

var greet := func is print "hi" end     // no parameter list at all

var apply := func(f, v) => f(v)         // functions passed as arguments
print apply(inc, 10)                    // 11
```

**Decision** ([Q17](#11-spec-ambiguities-and-decisions)–[Q19](#11-spec-ambiguities-and-decisions),
[Q22](#11-spec-ambiguities-and-decisions), [Q24](#11-spec-ambiguities-and-decisions),
[Q40](#11-spec-ambiguities-and-decisions)):

- Each parameter `IDENT` follows the same rule as every other identifier — it cannot
  start with a digit. `func(1) => 1` is a syntax error, not a one-parameter function
  named `1`.
- **Closures.** A function literal captures the scope it is written in **by reference**.
  Free names in the body are resolved **when the body runs**, in that scope and its
  ancestors: the body sees every variable declared there by call time — even one
  declared after the literal — with its current value, and may assign it. Parameters and
  the body's own declarations live in a fresh scope per call and hide captured names.
- **Recursion** follows from that: in `var f := func(n) is … f(n - 1) … end`, the body
  looks `f` up when it runs, and by then the declaration has completed.
- The number of arguments must **equal** the number of parameters; otherwise a runtime
  error.
- A duplicate parameter name is a runtime error, raised when the literal is evaluated.
- `func` with no parameter list and `func()` are the same thing.
- A literal cannot be called in place — `(func(x) => x)(1)` is a syntax error; bind it
  to a variable first.
- Functions are values: `f is func` is `true`, and `print f` prints `func`. They have no
  other operation — no `=`, no `+`.

```d
var counter := 0
var bump := func is counter := counter + 1 end
bump(); bump()
print counter               // 2 — the body assigned the enclosing variable

var fact := func(n) is
    if n <= 1 then return 1 end
    return n * fact(n - 1)
end
print fact(5)               // 120

var adder := func(k) => func(x) => x + k
print adder(10)(5)          // 15 — k is captured by the inner literal
```

**Tests** → [14-type-functions.md](tests/14-type-functions.md)

### 7.8 none

If the initial value of a variable is not set, the variable has the special value `none`.
This value **cannot act as an operand of operations**; the only action on a variable with
this value is type checking using the `is` operation.

**General form**

```
none                        // the literal / the type indicator
Reference is none           // the only allowed action
```

**Example**

```d
var a                       // a is none
print a is none             // true
a := none                   // none is also a literal
// print a + 1              // error: none cannot be an operand
```

**Decision** ([Q31](#11-spec-ambiguities-and-decisions)): "operand of operations" means
operand of an **operator** — `+ - * /`, the comparisons, `and or xor not`, unary `+ -` —
and, by the same reasoning, use as a condition, an array index, a range bound or an
iterated aggregate. All of those are runtime errors on `none`. Everything that merely
**moves** a value is allowed: `none` can be printed (as `none`), assigned, passed as an
argument, returned from a function and stored in an array or tuple.

```d
var a
print a                     // none
var b := a                  // fine
var arr := [none, 1]        // fine
print arr[1] is none        // true
var f := func(x) => x
print f(a) is none          // true
```

**Tests** → [11-op-type-check-is.md](tests/11-op-type-check-is.md) (`T-IS-4xx`)

---

## 8. Operator Semantics

The semantics of expressions are generally typical of programming languages and depend on
the type of operands. Since the language is dynamic, the control of the types of operands
and the selection of the appropriate operation algorithm is performed **dynamically
during program execution**.

The type matrix used by all the tests below is defined once in
[tests/Tests.md](tests/Tests.md#type-matrix-convention).

### 8.1 Addition

**General form**

```
Addition: +
Integer + Integer -> Integer
Integer + Real    -> Real
Real    + Integer -> Real
Real    + Real    -> Real
String  + String  -> String     (string concatenation)
Tuple   + Tuple   -> Tuple      (tuple concatenation)
Array   + Array   -> Array      (array concatenation)
```

**Other types of operands are not allowed.**

**Example**

```d
print 2 + 3                 // 5        Integer + Integer -> Integer
print 2 + 3.5               // 5.5      Integer + Real    -> Real
print 1.5 + 2               // 3.5      Real    + Integer -> Real
print 1.5 + 2.5             // 4.0      Real    + Real    -> Real
print "ab" + "cd"           // abcd     String  + String  -> String
print [1,2] + [3]           // [1, 2, 3]        Array + Array -> Array
print {a := 1} + {b := 2}   // {a := 1, b := 2}  Tuple + Tuple -> Tuple

// print true + false       // error: boolean operands are not allowed
// print 1 + "2"            // error: mixed integer/string is not allowed
```

**Tests** → [04-op-addition.md](tests/04-op-addition.md)

### 8.2 Subtraction

**General form**

```
Subtraction: -
Integer - Integer -> Integer
Integer - Real    -> Real
Real    - Integer -> Real
Real    - Real    -> Real
```

**Other types of operands are not allowed.**

Note: unlike `+`, subtraction is **not** defined for `String`, `Array` or `Tuple`.

**Example**

```d
print 5 - 3                 // 2        Integer - Integer -> Integer
print 5 - 3.5               // 1.5      Integer - Real    -> Real
print 5.5 - 3               // 2.5      Real    - Integer -> Real
print 5.5 - 3.5             // 2.0      Real    - Real    -> Real

// print "ab" - "b"         // error: strings are not allowed
// print [1,2] - [2]        // error: arrays are not allowed
```

**Tests** → [05-op-subtraction.md](tests/05-op-subtraction.md)

### 8.3 Multiplication

**General form**

```
Multiplication: *
Integer * Integer -> Integer
Integer * Real    -> Real
Real    * Integer -> Real
Real    * Real    -> Real
```

**Other types of operands are not allowed.**

**Example**

```d
print 2 * 3                 // 6        Integer * Integer -> Integer
print 2 * 1.5               // 3.0      Integer * Real    -> Real
print 1.5 * 2               // 3.0      Real    * Integer -> Real
print 1.5 * 2.0             // 3.0      Real    * Real    -> Real

// print "ab" * 2           // error: strings are not allowed
```

**Tests** → [06-op-multiplication.md](tests/06-op-multiplication.md)

### 8.4 Division

**General form**

```
Division: /
Integer / Integer -> Integer    (round down)
Integer / Real    -> Real
Real    / Integer -> Real
Real    / Real    -> Real
```

**Other types of operands are not allowed.**

**Example**

```d
print 7 / 2                 // 3        Integer / Integer -> Integer (round down)
print 7 / 2.0               // 3.5      Integer / Real    -> Real
print 7.0 / 2               // 3.5      Real    / Integer -> Real
print 7.0 / 2.0             // 3.5      Real    / Real    -> Real

// print "ab" / 2           // error: strings are not allowed
```

**Decision** ([Q6](#11-spec-ambiguities-and-decisions), [Q7](#11-spec-ambiguities-and-decisions)):
"round down" is **floor** — toward negative infinity — so `-7 / 2` is `-4` and
`7 / -2` is `-4`. Division by zero, integer or real, is a **runtime error**; the language
has no infinities or NaN.

```d
print -7 / 2                // -4
print 7 / -2                // -4
print -7 / -2               // 3
// print 1 / 0              // runtime error
// print 1.0 / 0.0          // runtime error
```

**Tests** → [07-op-division.md](tests/07-op-division.md)

### 8.5 Comparisons

**General form**

```
Comparisons: <, >, <=, >=, =, /=
Integer op Integer -> boolean
Integer op Real    -> boolean
Real    op Integer -> boolean
Real    op Real    -> boolean
```

**Other types of operands are not allowed.**

**Decision** ([Q5](#11-spec-ambiguities-and-decisions)): as written, comparison is
defined only for numeric operands. This project **extends `=` and `/=`** to two more
cells, and leaves the ordering operators numeric-only:

```
Equality only: =, /=
String  op String  -> boolean   (same characters)
Boolean op Boolean -> boolean
```

Arrays, tuples, functions and `none` remain incomparable, and mixed pairs (`1 = "1"`,
`1 = true`, `"a" = true`) remain errors.

**Example**

```d
print 1 < 2                 // true     Integer op Integer -> boolean
print 1 <= 1.0              // true     Integer op Real    -> boolean
print 2.5 > 2               // true     Real    op Integer -> boolean
print 2.5 /= 2.5            // false    Real    op Real    -> boolean
print 3 = 3                 // true
print "a" = "a"             // true     String = String    -> boolean   (extension)
print true /= false         // true     Boolean /= Boolean -> boolean   (extension)

// print "a" < "b"          // error: no ordering of strings
// print [1] = [1]          // error: arrays are not comparable
// print 1 = "1"            // error: mixed pair
```

**Tests** → [08-op-comparison.md](tests/08-op-comparison.md)

### 8.6 Unary plus and minus

**General form**

```
Unary plus and minus: +, -
op Integer -> Integer
op Real    -> Real
```

**Other types of operands are not allowed.**

**Example**

```d
print -5                    // -5       op Integer -> Integer
print +5                    // 5        op Integer -> Integer
print -2.5                  // -2.5     op Real    -> Real
print +2.5                  // 2.5      op Real    -> Real

// print -true              // error: boolean operand is not allowed
// print -"ab"              // error: string operand is not allowed
```

**Tests** → [09-op-unary-sign.md](tests/09-op-unary-sign.md)

### 8.7 Logical operations

**Both operands must be Boolean.** The result of operations is always boolean. Other
types of operands are not allowed.

**General form**

```
Logical operations: and, or, xor
Boolean and Boolean -> Boolean
Boolean or  Boolean -> Boolean
Boolean xor Boolean -> Boolean
```

**Example**

```d
print true and false        // false
print true or false         // true
print true xor true         // false

// print 1 and true         // error: non-boolean operand is not allowed
```

**Decision** ([Q29](#11-spec-ambiguities-and-decisions), [Q30](#11-spec-ambiguities-and-decisions)):
**both operands are always evaluated** — "both operands must be Boolean" is checked
even when the left one already decides the result, so `true or n` with `n` holding
`none` is an error and `false and f()` calls `f`. All three operators share one
precedence level and associate left to right.

**Tests** → [10-op-logical.md](tests/10-op-logical.md)

### 8.8 Unary negation

**The operand must be Boolean.** The result of the operator is boolean. Other types of the
operand are not allowed.

**General form**

```
Unary negation: not
not Boolean -> Boolean
```

**Example**

```d
print not true              // false
print not false             // true

// print not 1              // error: non-boolean operand is not allowed
```

**Tests** → [10-op-logical.md](tests/10-op-logical.md) (`T-LOG-5xx`)

### 8.9 Type check: is

A special **unary operation** to check the type of the current value of a variable. It is
the **only** operation allowed on a variable holding `none`.

**General form**

```
Reference is TypeIndicator -> boolean
TypeIndicator : int | real | bool | string | none | [ ] | { } | func
```

**Example**

```d
var a := 1
print a is int              // true
a := 1.5
print a is real             // true
a := "s"
print a is string           // true
a := true
print a is bool             // true
a := [1]
print a is []               // true
a := {x := 1}
print a is {}               // true
a := func => 0
print a is func             // true
var b
print b is none             // true
print 1 is int              // true — any Primary, not only a Reference (Q20)
```

**Decision** ([Q20](#11-spec-ambiguities-and-decisions)): `is` accepts any `Primary`
as its operand, not only a `Reference`, and binds tighter than every binary operator
(see [§5.5](#55-unary)).

**Tests** → [11-op-type-check-is.md](tests/11-op-type-check-is.md)

---

## 9. Implicit Type Conversions

"Implicit type conversions are supported." The only conversion made explicit by the
operation table is **integer → real widening** in mixed-type arithmetic and comparisons:
whenever one operand is `Integer` and the other is `Real`, the integer operand is widened
to `Real` and the result is `Real` (or `boolean` for comparisons).

**General form**

```
Integer op Real -> (Real) op Real
Real op Integer -> Real op (Real)
where op in { +, -, *, /, <, <=, >, >=, =, /= }
```

**Example**

```d
print 1 + 2.5               // 3.5   the integer 1 is widened to 1.0
print 4 / 2.0               // 2.0   real division, not integer division
print 1 = 1.0               // true  widened before comparison
print 7 / 2                 // 3     no widening — both are integers, result rounds down
```

**Decision**: integer → real widening is the **only** implicit conversion. There is no
narrowing (an array index or a range bound of `1.0` is an error), no number ↔ string
conversion, no truthiness, and `print`'s rendering ([§4.8](#48-print)) does not produce
string values.

**Tests** → [19-implicit-conversions.md](tests/19-implicit-conversions.md)

---

## 10. Full Grammar Summary

The grammar **as decided** — the description's productions with the three corrections
of [§5](#5-expressions) applied (`Factor`, `Unary`, `Primary`) and the lexical rules of
[§2](#2-program-structure) added. This is what the parser implements.

```
// Lexical
IDENT   : ( letter | _ ) { letter | digit | _ }        // case-sensitive, not a keyword
INTEGER : digit { digit }
REAL    : digit { digit } . digit { digit }
STRING  : " { character } "  |  ' { character } '     // no escapes, one line
COMMENT : // { character } end-of-line                 // ignored

Keywords: var if then else end while for in loop exit return print
          func is not and or xor true false none int real bool string

Program : { Statement [ ; ] }

Statement
    : Declaration | Assignment | If | IfShort | Loop | Exit | Return | Print
    | Call                                              // added (Q41)
Call
    : Reference ( Expression { , Expression } )         // the call is the last accessor

Declaration
    : var VariableDefinition { , VariableDefinition }
VariableDefinition
    : IDENT [ := Expression ]

Assignment
    : Reference := Expression
If
    : if Expression then Body [ else Body ] end
IfShort
    : if Expression => Body
Body
    : Statement { [ ; ] Statement }
Loop
    : WhileLoop | ForLoop
WhileLoop
    : while Expression LoopBody
ForLoop
    : [ ForHeader ] LoopBody
ForHeader
    : for [ IDENT in ] Expression [ .. Expression ]
LoopBody
    : loop Body end
Exit
    : exit
Return
    : return [ Expression ]
Print
    : print Expression { , Expression }

Expression
    : Relation { ( or | and | xor ) Relation }          // one level, left to right
Relation
    : Factor [ ( < | <= | > | >= | = | /= ) Factor ]    // at most one comparison
Factor
    : Term { ( + | - ) Term }                           // corrected: operator required
Term
    : Unary { ( * | / ) Unary }
Unary
    : [ + | - | not ] Unary                             // corrected: prefixes stack
    | Primary [ is TypeIndicator ]                      // corrected: is on any Primary
Primary
    : Reference                                         // corrected: added
    | Literal | FunctionLiteral | ( Expression )
TypeIndicator
    : int | real | bool | string | none | [ ] | { } | func

FunctionLiteral
    : func [ ( IDENT { , IDENT } ) ] FunBody
FunBody
    : is Body end
    | => Expression

Literal
    : INTEGER | REAL | STRING | Boolean | Tuple | Array | none
Boolean
    : true | false
Array
    : [ [ Expression { , Expression } ] ]
Tuple
    : { TupleElement { , TupleElement } }
TupleElement
    : [ Identifier := ] Expression

Reference
    : IDENT
    | Reference [ Expression ]                      // array element
    | Reference ( Expression { , Expression } )     // call
    | Reference . IDENT                             // tuple element
    | Reference . IntegerLiteral                    // tuple element
```

Static checks the parser makes beyond the productions ([Q24](#11-spec-ambiguities-and-decisions)):
`exit` must be inside a `LoopBody`; `return` must be inside a `FunBody`; the target of
an `Assignment` must not end in a call accessor.

---

## 11. Spec Ambiguities and Decisions

Points where the source description is incomplete or self-contradictory, and the
decision taken for each. **All questions are resolved.** Every decision is also written
into the section of this document it affects (look for **Decision** paragraphs) and into
the tests that depend on it — the `9xx` row of each block names the question and
records the verdict, and the test that used to be marked `?` now states an expected
value.

The decisions follow one principle: **stay as close to the description as possible, and
where it is silent choose the reading that is simplest to implement and to explain.**

### Q1–Q21 — questions raised while reading the description

| #   | Topic                                      | Question                                                                                                                                                                                                                                                       | Decision                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| --- | ------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Q1  | `none` naming                              | The description says an uninitialised variable has the value `none`, then refers to it as "value **empty**".                                                                                                                                                   | **One value, `none`.** "Empty" is a slip in wording.                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| Q2  | `Literal` production                       | Given twice with different contents — `IntegerLiteral \| RealLiteral \| BooleanLiteral` on one page, and the full `INTEGER \| REAL \| STRING \| Boolean \| Tuple \| Array \| none` on the next.                                                               | **The full production is authoritative.** The short one is an earlier draft.                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| Q3  | `Factor` operator optional                 | `Factor : Term { [ + \| - ] Term }` makes the operator optional, so `a b` (juxtaposition) is derivable.                                                                                                                                                        | **Typo.** The production is `Factor : Term { ( + \| - ) Term }`. `print 1 2` is a syntax error.                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Q4  | Unary on a `Reference`                     | `Unary : ... \| [ + \| - \| not ] Primary` and `Primary` does not include `Reference`, so `-x` and `not x` for a variable `x` are not derivable.                                                                                                              | **Grammar fixed** (see [§5.5](#55-unary)): `Unary : [ + \| - \| not ] Unary \| Primary [ is TypeIndicator ]` and `Primary : Reference \| Literal \| FunctionLiteral \| ( Expression )`. Consequences: `-x`, `not x`, `-t[1]` and `- -5` all parse; a prefix operator applies to the whole operand that follows it, so `not a is int` means `not (a is int)`.                                                                                                                                                       |
| Q5  | Comparison of non-numerics                 | The table allows only numeric operands, so `"a" = "b"`, `true = false`, array/tuple equality are all "not allowed".                                                                                                                                            | **`=` and `/=` are extended to `String = String` and `Boolean = Boolean`.** Ordering (`< <= > >=`) stays numeric-only. Arrays, tuples, functions and `none` are still not comparable, and mixed pairs (`1 = "1"`, `1 = true`) are still errors.                                                                                                                                                                                                                                                                     |
| Q6  | Division by zero                           | Not specified for either `Integer / Integer` or real division.                                                                                                                                                                                                 | **Runtime error**, for integer and real division alike. No infinities or NaN ever exist in the language.                                                                                                                                                                                                                                                                                                                                                                                                           |
| Q7  | `Integer / Integer` rounding for negatives | "round down" — floor (`-7/2 = -4`) or truncation toward zero (`-7/2 = -3`)?                                                                                                                                                                                    | **Floor** (toward negative infinity): `-7 / 2 = -4`, `7 / -2 = -4`, `-7 / -2 = 3`. "Round down" is read literally.                                                                                                                                                                                                                                                                                                                                                                                                 |
| Q8  | Missing array key                          | Arrays are associative; reading a key that was never assigned is not specified (error vs `none`).                                                                                                                                                              | **Reads as `none`.** `a[0]`, `a[-1]`, `a[4]` on `[1,2,3]`, and a gap in a sparse array all yield `none`; `a[k] is none` is therefore the presence test. Reading does not create the key. A non-integer index is still a runtime error.                                                                                                                                                                                                                                                                             |
| Q9  | Array concatenation and keys               | `Array + Array -> Array` with sparse integer keys — are right-hand keys renumbered or merged?                                                                                                                                                                   | **Renumbered.** The result holds the left operand's values in ascending key order, then the right operand's, under fresh keys `1..n`. Keys are never merged, so no value is lost or overwritten.                                                                                                                                                                                                                                                                                                                    |
| Q10 | Tuple concatenation with duplicate names   | Element names are "unique within the tuple", but `{a:=1} + {a:=2}` is not specified.                                                                                                                                                                           | **Runtime error.** Uniqueness of names is an invariant of every tuple, so an operation that would break it fails.                                                                                                                                                                                                                                                                                                                                                                                                  |
| Q11 | Tuple element assignment                   | Tuple structure "cannot be modified" — is `t.a := 5` (changing a value, not the structure) allowed?                                                                                                                                                            | **Allowed.** `t.a := v` and `t.1 := v` replace the value of an existing element. `t.b := v` for a name that does not exist would change the structure and is a runtime error.                                                                                                                                                                                                                                                                                                                                       |
| Q12 | `for` over a range with real bounds        | Header says `v1`, `v2` are "current **integer** values" — behaviour for real expressions is unspecified.                                                                                                                                                       | **Runtime error.** Both bounds must evaluate to integers; there is no narrowing.                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Q13 | Loop variable over a tuple                 | Iterating a tuple yields element values — are names accessible?                                                                                                                                                                                                | **Values only,** in positional order. Names are not reachable from the loop variable.                                                                                                                                                                                                                                                                                                                                                                                                                              |
| Q14 | `return` outside a function                | Not specified (`exit` outside a loop is explicitly forbidden; `return` is not).                                                                                                                                                                                | **Forbidden, like `exit`.** Both are checked by the parser before execution ([Q24](#q22q41--questions-raised-while-writing-the-tests)).                                                                                                                                                                                                                                                                                                                                                                             |
| Q15 | `print` output format                      | Separator between values, and rendering of `real`, `array`, `tuple`, `func` and `none` values are not specified.                                                                                                                                              | Fixed in [§4.8](#48-print): values separated by **one space**, one **newline** after each `print`; integers as decimal digits; reals with the **shortest round-trip** digit string and **always a `.`** (`3.0`, `0.5`, `0.3333333333333333`); strings without quotes; `true`/`false`; arrays `[1, 2, 3]` (`[]` when empty, gaps not shown); tuples `{a := 1, 2}`; functions as `func`; `none` as `none`. The rule is the same at every nesting depth.                                                             |
| Q16 | Comments                                   | The examples use `//` line comments but the lexical rules never define them.                                                                                                                                                                                   | **`//` to end of line is a comment.** There is no block comment form; `/*` is a syntax error.                                                                                                                                                                                                                                                                                                                                                                                                                      |
| Q17 | Function call arity mismatch               | Calling with fewer/more arguments than parameters is not specified.                                                                                                                                                                                            | **Runtime error.** The argument count must equal the parameter count.                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| Q18 | Falling off the end of a function          | The result of a function that terminates without `return` is not specified.                                                                                                                                                                                    | **`none`** — for both `return` without an expression and running off the end of an `is … end` body.                                                                                                                                                                                                                                                                                                                                                                                                                |
| Q19 | Recursion / closures                       | Whether a function literal captures its enclosing scope is not specified.                                                                                                                                                                                      | **Lexical closures, captured by reference.** A function literal captures the scope it is written in; the body resolves free names **at call time** in that scope and its ancestors, so it sees every variable declared there by the time of the call — including ones declared after the literal — with their current values, and may assign them. The declaration `var f := func …` has completed by the time `f` is called, so `f` can call itself: recursion needs no extra rule.                                                                                                              |
| Q20 | `is` operand                               | `Unary : Reference is TypeIndicator` restricts `is` to references — `1 is int` on a literal is not derivable.                                                                                                                                                  | **`is` accepts any `Primary`** (part of the Q4 grammar fix). `1 is int` and `(1 + 1) is int` parse. `is` sits at the `Unary` level, so it binds tighter than every binary operator: `a is int and true` is `(a is int) and true`, and `a is int + 1` is `(a is int) + 1`, a runtime error.                                                                                                                                                                                                                          |
| Q21 | Reading input                              | There is no input construct at all: `print` is the only I/O in the statement list.                                                                                                                                                                             | **No input.** Programs are closed; all data is literal. A test is a single source file, and the suite needs no fixture files. The options that were considered are kept in [tests/custom/Custom.md](tests/custom/Custom.md#resolved-question-do-we-need-to-read-input) for the record.                                                                                                                                                                                                                              |

### Q22–Q41 — questions raised while writing the tests

These came up block by block; each is referenced from the `9xx` row of the block that
raised it.

| #   | Topic                                     | Question                                                                                                                                   | Decision                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| --- | ----------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Q22 | Identifier syntax                         | `IDENT` is never defined; underscores and case sensitivity are unspecified.                                                                | `IDENT : ( letter \| _ ) { letter \| digit \| _ }` — **an identifier can never start with a digit.** **Case-sensitive**, for identifiers and keywords alike (`True` is an identifier, `A` and `a` are different variables). Keywords cannot be identifiers. This same `IDENT` is what the grammar calls `Identifier` in `TupleElement` ([§7.6](#76-tuple)), what names a function parameter ([§7.7](#77-function)), and what names the `for` loop variable ([§4.5](#45-loop)) — one rule, applied everywhere a name is declared. Consequence: `var 1a := 1`, `{1 := 5}`, `func(1) => 1` and `for 1i in 1..3 loop … end` are all syntax errors for the identical reason — `1` (or `1i`) lexes as `INTEGER` (then `IDENT`), never as a single `IDENT` — so, in particular, a tuple element can never be **named** `1`, only accessed **at position** `1` (`t.1`, a different production, `IntegerLiteral`).                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Q23 | String literal content                    | "Arbitrary characters" — are there escape sequences, and may a string span lines?                                                          | **No escape sequences**; a backslash is an ordinary character. A string literal **ends on the line it starts** — a newline before the closing quote is a syntax error. The closing quote cannot appear inside the string; use the other quote style (`"it's"`).                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| Q24 | Syntax error vs runtime error             | Which violations are detected before execution?                                                                                            | **Syntax errors** (reported by the parser, nothing executes): every grammar violation, plus three placement rules the parser can check — `exit` outside a loop body, `return` outside a function body, and a call `f(…)` as the last accessor of an assignment target. **Everything else is a runtime error** raised when the offending statement executes: type errors, undeclared names, redeclaration in the same scope, duplicate names in one declaration, in a tuple literal or in a parameter list, arity mismatch, missing tuple elements. A runtime error aborts the program; output produced before it stays produced. |
| Q25 | Initialiser scope                         | In `var x := x + 1` inside a nested scope, does the initialiser see the outer `x` or the one being declared?                               | **The outer one.** The initialiser is evaluated first, in the scope as it is before the declaration; the new name is introduced afterwards. `var x := 1; if true then var x := x + 1; print x end` prints `2`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| Q26 | Value vs reference semantics              | Does `var b := a` copy an array or tuple, or alias it?                                                                                     | **Arrays and tuples alias.** Assignment, declaration and argument passing bind the same object; `b[1] := 9` is visible through `a`. Simple values (integer, real, string, boolean, `none`) and functions are immutable, so the distinction never shows for them. `+` always builds a **new** array or tuple.                                                                                                                                                                                                                                                                                                                                                                                                                         |
| Q27 | Integer range                             | Integer size and overflow behaviour are not specified.                                                                                     | **64-bit signed** (`-2^63 .. 2^63-1`). An arithmetic result outside that range is a **runtime error**; nothing wraps silently. Widening a large integer to real rounds to the nearest double and is not an error.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Q28 | Real representation                       | Precision of reals and of their printed form are not specified.                                                                            | **IEEE 754 double.** Arithmetic is ordinary double arithmetic, so `0.1 + 0.2 = 0.3` is `false` and `0.3 - 0.1` prints `0.19999999999999998`. Printing uses the shortest digit string that reads back to the same double ([Q15](#q1q21--questions-raised-while-reading-the-description)).                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Q29 | Short-circuit evaluation                  | Do `and` and `or` evaluate the right operand when the left one decides the result?                                                        | **No short-circuiting.** Both operands are always evaluated and both must be boolean, as the table says. `true or n` with `n` holding `none` is a runtime error; `false and f()` calls `f`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Q30 | Precedence of `and` / `or` / `xor`        | The grammar puts all three at one level; most languages bind `and` tighter than `or`.                                                      | **As the grammar says**: one level, left to right. `true or false and false` is `(true or false) and false` = `false`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| Q31 | What counts as an operation on `none`     | Are printing, assigning, passing, or storing a `none` value "operations" that `none` may not take part in?                                 | **No.** The restriction is on *operators* (`+ - * / < <= > >= = /= and or xor not`, unary `+ -`, and use as a condition, an index, a range bound or an iterated aggregate). `none` can be printed (renders as `none`), assigned, passed as an argument, returned, and stored in an array or tuple. `is` works on it as the description says.                                                                                                                                                                                                                                                                                                                                                                                             |
| Q32 | Iteration order over sparse keys          | `for v in t loop` over an array whose keys are not contiguous.                                                                             | **Ascending key order**, every present key, gaps skipped. The same order is used by `print` and by `+`. The set of keys to visit is fixed when the loop starts, so writes to the array inside the body do not change how many times it runs; the loop variable holds a copy of the element's value, not the slot, so assigning it does not write into the array.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| Q33 | Empty tuple                               | `Tuple` requires at least one element, but `{ }` is a valid `TypeIndicator`.                                                               | **`{}` is only the type indicator**; there is no empty tuple literal and `var t := {}` is a syntax error. The parser knows it is reading a type indicator because it is directly after `is`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| Q34 | Non-boolean condition                     | `if 1 then`, `while "s" loop` — error, or a truthiness rule?                                                                               | **Runtime error.** A condition must evaluate to a boolean; nothing is converted. This is the only reading consistent with the operator table, which forbids non-boolean logical operands.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Q35 | Empty body                                | `if c then else … end`, `while c loop end` — may a `Body` be empty?                                                                        | **No**, as the production says: a `Body` holds at least one `Statement`. An empty branch is a syntax error.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| Q36 | Extent of an `IfShort` body               | `if c => Body` has no terminator — where does the body end?                                                                                | **At the end of the line**, or at the first `end` / `else` on that line, whichever comes first. Semicolons inside it separate statements of the body: `if true => var a := 1; print a` prints `1`. This keeps the description's own `loop … if i = 100 => exit end` one-liner valid.                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| Q37 | Descending range                          | `for i in 3..1 loop` — empty, or counting down?                                                                                            | **Zero iterations.** A range `v1..v2` visits `v1, v1+1, …, v2` and nothing when `v1 > v2`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| Q38 | Assigning to the loop variable            | May the body assign to the variable a `for` header introduces?                                                                             | **Yes.** It is an ordinary variable of the body's scope. The assignment is visible for the rest of that iteration only; the next iteration takes the next value from the range or aggregate regardless.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Q39 | Range bounds                              | Is `v1..v2` inclusive at both ends?                                                                                                        | **Yes.** `for 1..3` iterates three times; `1..1` once.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| Q40 | Calling a function literal directly       | `(func(x) => x + 1)(1)` — a call accessor applies to a `Reference`, and `( Expression )` is not one.                                       | **Not derivable; syntax error.** Accessors (`[ ]`, `( )`, `.`) hang only off a `Reference`, which is rooted in an `IDENT`. Bind the literal to a variable first. The same applies to `[1,2][1]` and `{a := 1}.a`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Q41 | A call as a statement                     | `Statement` lists no expression statement, yet a function whose body only prints is useless unless `f()` can stand alone.                | **A call is a statement.** `Statement` gains `Call : Reference ( Expression { , Expression } )`, whose value is discarded — the call must be the **last** accessor (`t.f()`, `a[1]()` are fine; `f().x` alone is not). No other expression is a statement: `a = 2` and `1 + 1` on their own are syntax errors. The parser reads a `Reference`; `:=` after it makes an assignment, a trailing call accessor makes a call statement, anything else is an error.                                                                                                                                                                                                                                                                             |
