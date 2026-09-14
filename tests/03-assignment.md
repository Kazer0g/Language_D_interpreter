# Block 03 — Assignment

Prefix `T-ASGN` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§4.1 Assignment](../Language%20D.md#41-assignment), [§6 References](../Language%20D.md#6-references-accessors)

**Program**: [`03-assignment`](03-assignment) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Assignment
    : Reference := Expression
Reference
    : IDENT
    | Reference [ Expression ]                      // array element
    | Reference ( Expression { , Expression } )     // call
    | Reference . IDENT                             // tuple element
    | Reference . IntegerLiteral                    // tuple element
```

Only a `Reference` can be the recipient of a value — this is what distinguishes
references from other operands (constants, subexpressions, etc.).

## 0xx — Valid assignment targets

| ID | Program | Expected |
|---|---|---|
| T-ASGN-001 | `var a := 1` <br> `a := 2` <br> `print a` | `2` — plain identifier |
| T-ASGN-002 | `var a := []` <br> `a[1] := 5` <br> `print a[1]` | `5` — array element |
| T-ASGN-003 | `var a := [[0]]` <br> `a[1][1] := 7` <br> `print a[1][1]` | `7` — nested array element |
| T-ASGN-004 | `var a := [ {x := 1} ]` <br> `a[1].x := 9` <br> `print a[1].x` | `9` — tuple element through an array; a value change is allowed ([Q11](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ASGN-005 | `var a := 1` <br> `a := 1 + 2 * 3` <br> `print a` | `7` — right side is any expression |
| T-ASGN-006 | `var a := 1, b := 2` <br> `a := b` <br> `print a` | `2` — right side is a reference |
| T-ASGN-007 | `var f := func => 5` <br> `var a := 0` <br> `a := f()` <br> `print a` | `5` — right side is a call |
| T-ASGN-008 | `var t := {x := 1}` <br> `t.x := 9` <br> `print t.x` | `9` — tuple element by name |
| T-ASGN-009 | `var t := {x := 1}` <br> `t.1 := 9` <br> `print t.x` | `9` — tuple element by position |

## 1xx — Invalid assignment targets

| ID | Program | Expected |
|---|---|---|
| T-ASGN-101 | `1 := 2` | syntax error — a literal is not a `Reference` |
| T-ASGN-102 | `(a) := 2` | syntax error — a parenthesised expression is not a `Reference` |
| T-ASGN-103 | `a + b := 2` | syntax error — an expression is not a `Reference` |
| T-ASGN-104 | `"s" := 2` | syntax error |
| T-ASGN-105 | `b := 1` *(no declaration of `b`)* | runtime error — assignment does not declare |
| T-ASGN-106 | `var a := 1` <br> `a := ` | syntax error — `:=` requires an expression |
| T-ASGN-107 | `var a := 1` <br> `a = 2` | syntax error — `=` is comparison, not assignment, and a bare expression is not a `Statement`; only a call may stand alone ([Q41](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ASGN-108 | `var f := func => 1` <br> `f() := 1` | syntax error — a call cannot be the last accessor of a target ([Q24](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ASGN-109 | `var t := {a := 1}` <br> `t.b := 5` | runtime error — the tuple has no element `b`; adding one would change its structure ([Q11](../Language%20D.md#11-spec-ambiguities-and-decisions)) |

## 2xx — Semantics

| ID | Program | Expected |
|---|---|---|
| T-ASGN-201 | `var a := 1` <br> `a := "s"` <br> `print a is string` | `true` — assignment may change the type |
| T-ASGN-202 | `var a := 1, b := 2` <br> `a := b` <br> `b := 3` <br> `print a` | `2` — assigning a simple value copies it |
| T-ASGN-203 | `var a := [1], b := a` <br> `b[1] := 9` <br> `print a[1]` | `9` — composite values **alias** on assignment ([Q26](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ASGN-204 | `var a := 1` <br> `a := a + 1` <br> `print a` | `2` — the target may appear on the right side |
| T-ASGN-205 | `var a := 1` <br> `a := none` <br> `print a is none` | `true` — `none` can be assigned |
| T-ASGN-206 | `var a := []` <br> `a[3] := 1` <br> `print a[3]` | `1` — assignment creates a missing array key |
| T-ASGN-207 | `var i := 0, a := [0,0]` <br> `a[i + 1] := 5` <br> `print a[1]` | `5` — index is any integer expression |
| T-ASGN-208 | `var t := {a := 1}, u := t` <br> `u.a := 5` <br> `print t.a` | `5` — tuples alias too |
| T-ASGN-209 | `var a := [1]` <br> `var f := func(x) is x[1] := 9 end` <br> `f(a)` <br> `print a[1]` | `9` — argument passing aliases as well |
| T-ASGN-210 | `var a := [1], b := a` <br> `b := b + [2]` <br> `print a` | `[1]` — `+` builds a new array; rebinding `b` does not touch `a` |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-ASGN-901 | [Q26](../Language%20D.md#11-spec-ambiguities-and-decisions) — value vs reference semantics for arrays, tuples and functions on assignment. | Arrays and tuples alias; simple values and functions are immutable so it never shows. `+` builds a new object. | `T-ASGN-203`, `T-ASGN-208`–`T-ASGN-210` |
| T-ASGN-902 | [Q11](../Language%20D.md#11-spec-ambiguities-and-decisions) — `t.a := v` on a tuple. | Allowed for an existing element; a new name is a runtime error. | `T-ASGN-004`, `T-ASGN-008`, `T-ASGN-009`, `T-ASGN-109` |
| T-ASGN-903 | [Q24](../Language%20D.md#11-spec-ambiguities-and-decisions) — is `f() := 1` a syntax error, given the grammar permits it? | Yes: the parser rejects a target whose last accessor is a call. | `T-ASGN-108` |
