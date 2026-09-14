# Block 01 — Lexical Elements and Literals

Prefix `T-LEX` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§2 Program Structure](../Language%20D.md#2-program-structure), [§4.4 Body](../Language%20D.md#44-body), [§7 Types and Literals](../Language%20D.md#7-types-and-literals)

**Program**: [`01-lexical-and-literals`](01-lexical-and-literals) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Program : { Statement [ ; ] }
Body    : Statement { [ ; ] Statement }
Literal : INTEGER | REAL | STRING | Boolean | Tuple | Array | none
Boolean : true | false
```

- Integer literals: a sequence of decimal digits.
- Real literals: **two** sequences of decimal digits separated by the dot character.
- Strings: arbitrary characters enclosed in **single or double** quotes.
- Statements are separated by the semicolon character **or** by newline characters.

Decided lexical rules ([Q16](../Language%20D.md#11-spec-ambiguities-and-decisions),
[Q22](../Language%20D.md#11-spec-ambiguities-and-decisions),
[Q23](../Language%20D.md#11-spec-ambiguities-and-decisions)): `//` line comments only;
`IDENT : ( letter | _ ) { letter | digit | _ }`, case-sensitive; strings have no escape
sequences and end on the line they start.

## 0xx — Literal forms

| ID | Program | Expected |
|---|---|---|
| T-LEX-001 | `print 0` | `0` |
| T-LEX-002 | `print 42` | `42` |
| T-LEX-003 | `print 1000000` | `1000000` |
| T-LEX-004 | `print 12.34` | `12.34` |
| T-LEX-005 | `print 0.5` | `0.5` |
| T-LEX-006 | `print 3.0` | `3.0` |
| T-LEX-007 | `print "hello"` | `hello` |
| T-LEX-008 | `print 'hello'` | `hello` |
| T-LEX-009 | `print ""` | *(empty line)* |
| T-LEX-010 | `print "a b, c!"` | `a b, c!` — arbitrary characters |
| T-LEX-011 | `print true` | `true` |
| T-LEX-012 | `print false` | `false` |
| T-LEX-013 | `print [1, 2, 3]` | `[1, 2, 3]` — array literal, rendering per [Q15](../Language%20D.md#11-spec-ambiguities-and-decisions) |
| T-LEX-014 | `print {a := 1}` | `{a := 1}` — tuple literal |
| T-LEX-015 | `var a := none` <br> `print a is none` | `true` |
| T-LEX-016 | `print func => 0` | `func` — a function literal is a value |

## 1xx — Malformed literals

| ID | Program | Expected |
|---|---|---|
| T-LEX-101 | `print 1.` | syntax error — a real needs a fractional digit sequence |
| T-LEX-102 | `print .5` | syntax error — a real needs an integer digit sequence |
| T-LEX-103 | `print 1.2.3` | syntax error |
| T-LEX-104 | `print "unterminated` | syntax error |
| T-LEX-105 | `print 'mixed"` | syntax error — quotes must match |
| T-LEX-106 | `print 1e5` | syntax error — no exponent form in the spec |
| T-LEX-107 | `print True` | runtime error — `True` is an undeclared identifier, not the keyword |

## 3xx — Statement separators and program structure

| ID | Program | Expected |
|---|---|---|
| T-LEX-301 | `var a := 1; print a` | `1` — semicolon separator |
| T-LEX-302 | `var a := 1` <br> `print a` | `1` — newline separator |
| T-LEX-303 | `var a := 1;` <br> `print a;` | `1` — trailing semicolon is allowed |
| T-LEX-304 | `var a := 1; var b := 2; print a + b` | `3` — several statements on one line |
| T-LEX-305 | `print 1` <br> *(blank line)* <br> `print 2` | `1` `2` — blank lines are not statements |
| T-LEX-306 | `if true then` <br> `  var a := 1; print a` <br> `end` | `1` — a `Body` uses the same separators |
| T-LEX-307 | *(empty file)* | no output, exit success |
| T-LEX-308 | `print 1 // comment` | `1` — `//` line comment, see [Q16](../Language%20D.md#11-spec-ambiguities-and-decisions) |
| T-LEX-309 | `// only a comment` | no output |
| T-LEX-310 | `var a := 1  ;  print   a` | `1` — whitespace around tokens is insignificant |

## 4xx — Identifiers

| ID | Program | Expected |
|---|---|---|
| T-LEX-401 | `var a := 1` <br> `print a` | `1` — single letter |
| T-LEX-402 | `var myVar1 := 1` <br> `print myVar1` | `1` — letters and digits |
| T-LEX-403 | `var _x := 1` <br> `print _x` | `1` — underscore is an identifier character, see [Q22](../Language%20D.md#11-spec-ambiguities-and-decisions) |
| T-LEX-404 | `var var := 1` | syntax error — keyword cannot be an identifier |
| T-LEX-405 | `var 1a := 1` | syntax error — identifier cannot start with a digit ([Q22](../Language%20D.md#11-spec-ambiguities-and-decisions)); the same `IDENT` rule governs tuple element names ([T-TUP-113](13-type-tuples.md#1xx--invalid-literals-and-access)), function parameters ([T-FUN-104](14-type-functions.md#1xx--invalid-literals-and-calls)) and the `for` loop variable ([T-LOOP-117](16-stmt-loops.md#1xx--for-loop-over-a-range)) |
| T-LEX-406 | `var A := 1; print a` | runtime error — identifiers are case-sensitive, see [Q22](../Language%20D.md#11-spec-ambiguities-and-decisions) |
| T-LEX-407 | `var a_1 := 1` <br> `print a_1` | `1` — underscore in the middle of an identifier |

## 9xx — Decided

Tests that pin down a question the description left open. Each row names the question
and the decision it verifies.

| ID | Question | Program | Expected |
|---|---|---|---|
| T-LEX-901 | [Q15](../Language%20D.md#11-spec-ambiguities-and-decisions) — real rendering | `print 3.0` | `3.0` — a real always prints with a `.` |
| T-LEX-902 | [Q16](../Language%20D.md#11-spec-ambiguities-and-decisions) — comment forms | `/* c */ print 1` | syntax error — only `//` line comments exist |
| T-LEX-903 | [Q23](../Language%20D.md#11-spec-ambiguities-and-decisions) — escape sequences | `print "a\nb"` | `a\nb` — the backslash is an ordinary character, no escapes |
| T-LEX-904 | [Q23](../Language%20D.md#11-spec-ambiguities-and-decisions) — string spanning lines | `print "a` <br> `b"` | syntax error — a string ends on the line it starts |
| T-LEX-905 | [Q23](../Language%20D.md#11-spec-ambiguities-and-decisions) — the other quote inside | `print "it's"` | `it's` |
| T-LEX-906 | [Q16](../Language%20D.md#11-spec-ambiguities-and-decisions) — comment inside a string | `print "a // b"` | `a // b` — `//` inside a string is not a comment |
