# Block 11 — Type Check `is`

Prefix `T-IS` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§8.9 Type check `is`](../Language%20D.md#89-type-check-is), [§5.7 TypeIndicator](../Language%20D.md#57-typeindicator), [§7.8 none](../Language%20D.md#78-none)

**Program**: [`11-op-type-check-is`](11-op-type-check-is) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Unary
    : Reference is TypeIndicator        // as written
    | Primary [ is TypeIndicator ]      // as decided (Q20): any Primary
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

`is` is the special unary operation for checking the type of the current value of a
variable. It is also the **only** action allowed on a variable holding `none`.

Unlike every other operator in the suite, `is` has **no unsupported operand types** —
it accepts a value of any kind and answers `true` or `false`. Its `1xx` section is
therefore about malformed *type indicators* and malformed *operands*, not about
rejected value types.

## Result grid

Rows = the value held by the variable, columns = the `TypeIndicator`. Exactly one
`true` per row.

| value \ indicator | `int` | `real` | `bool` | `string` | `[ ]` | `{ }` | `func` | `none` |
|---|---|---|---|---|---|---|---|---|
| **integer** | true | false | false | false | false | false | false | false |
| **real** | false | true | false | false | false | false | false | false |
| **boolean** | false | false | true | false | false | false | false | false |
| **string** | false | false | false | true | false | false | false | false |
| **array** | false | false | false | false | true | false | false | false |
| **tuple** | false | false | false | false | false | true | false | false |
| **func** | false | false | false | false | false | false | true | false |
| **none** | false | false | false | false | false | false | false | true |

## 0xx — Matching indicator, one per row

| ID | Program | Expected |
|---|---|---|
| T-IS-001 | `var a := 1` <br> `print a is int` | `true` |
| T-IS-002 | `var a := 1.5` <br> `print a is real` | `true` |
| T-IS-003 | `var a := true` <br> `print a is bool` | `true` |
| T-IS-004 | `var a := "s"` <br> `print a is string` | `true` |
| T-IS-005 | `var a := [1,2]` <br> `print a is []` | `true` |
| T-IS-006 | `var a := {x := 1}` <br> `print a is {}` | `true` |
| T-IS-007 | `var a := func => 0` <br> `print a is func` | `true` |
| T-IS-008 | `var a` <br> `print a is none` | `true` |
| T-IS-009 | `var a := ''` <br> `print a is string` | `true` — single quotes make the same type |
| T-IS-010 | `var a := []` <br> `print a is []` | `true` — an empty array is still an array |
| T-IS-011 | `var r := (1 is int)` <br> `print r is bool` | `true` — the result of `is` is boolean |

## 02x — Non-matching indicator, representative negatives

| ID | Program | Expected |
|---|---|---|
| T-IS-021 | `var a := 1` <br> `print a is real` | `false` — an integer is **not** a real, despite widening in arithmetic |
| T-IS-022 | `var a := 1.0` <br> `print a is int` | `false` — `1.0` is real even with a zero fraction |
| T-IS-023 | `var a := 1` <br> `print a is bool` | `false` |
| T-IS-024 | `var a := "1"` <br> `print a is int` | `false` |
| T-IS-025 | `var a := 1` <br> `print a is none` | `false` |
| T-IS-026 | `var a := [1]` <br> `print a is {}` | `false` — array is not tuple |
| T-IS-027 | `var a := {x := 1}` <br> `print a is []` | `false` — tuple is not array |
| T-IS-028 | `var a := func => 0` <br> `print a is int` | `false` |
| T-IS-029 | `var a := true` <br> `print a is string` | `false` |

## 1xx — Malformed indicators and operands

| ID | Program | Expected |
|---|---|---|
| T-IS-101 | `var a := 1` <br> `print a is integer` | syntax error — the indicator is `int`, not `integer` |
| T-IS-102 | `var a := 1` <br> `print a is boolean` | syntax error — the indicator is `bool`, not `boolean` |
| T-IS-103 | `var a := 1` <br> `print a is array` | syntax error — the indicator is `[ ]` |
| T-IS-104 | `var a := 1` <br> `print a is tuple` | syntax error — the indicator is `{ }` |
| T-IS-105 | `var a := 1` <br> `print a is [1]` | syntax error — `[ ]` must be empty |
| T-IS-106 | `var a := 1` <br> `print a is` | syntax error — an indicator is required |
| T-IS-107 | `var a := 1, b := 2` <br> `print a is b` | syntax error — the right side is a type indicator, not an expression |
| T-IS-108 | `print 1 is int` | `true` — `is` accepts any `Primary`, including a literal ([Q20](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-IS-109 | `print (1 + 1) is int` | `true` — and a parenthesised expression |
| T-IS-110 | `var a := 1` <br> `print a is int is bool` | syntax error — `is` does not chain (`Primary [ is TypeIndicator ]` is optional, not repeating) |
| T-IS-111 | `print (1 is int) is bool` | `true` — chaining is expressed with parentheses |
| T-IS-112 | `print (func => 0) is func` | `true` — a function literal is a `Primary` |

## 2xx — Dynamic typing

| ID | Program | Expected |
|---|---|---|
| T-IS-201 | `var a := 1` <br> `print a is int` <br> `a := "s"` <br> `print a is int, a is string` | `true` then `false` `true` — the answer follows the current value |
| T-IS-202 | `var a := 1` <br> `a := a + 1.5` <br> `print a is real` | `true` — widening changes the runtime type |
| T-IS-203 | `var a := 7 / 2` <br> `print a is int` | `true` — integer division yields an integer |
| T-IS-204 | `var a := 7 / 2.0` <br> `print a is real` | `true` |
| T-IS-205 | `var a := 1 < 2` <br> `print a is bool` | `true` |
| T-IS-206 | `var a := [1] + [2]` <br> `print a is []` | `true` — concatenation preserves the kind |
| T-IS-207 | `var t := [ 1, "s", true ]` <br> `print t[1] is int, t[2] is string, t[3] is bool` | `true true true` — an array holds values of any type |

## 3xx — Reference forms as the operand

| ID | Program | Expected |
|---|---|---|
| T-IS-301 | `var a := [1]` <br> `print a[1] is int` | `true` — array element |
| T-IS-302 | `var t := {x := 1.5}` <br> `print t.x is real` | `true` — tuple element by name |
| T-IS-303 | `var t := {x := 1.5}` <br> `print t.1 is real` | `true` — tuple element by position |
| T-IS-304 | `var f := func => "s"` <br> `print f() is string` | `true` — call result |

## 4xx — `none` handling

`none` cannot act as an operand of operations; the only action on a variable with this
value is a type check with `is`.

| ID | Program | Expected |
|---|---|---|
| T-IS-401 | `var a` <br> `print a is none` | `true` — the allowed action |
| T-IS-402 | `var a` <br> `print a + 1` | runtime error |
| T-IS-403 | `var a` <br> `print a - 1` | runtime error |
| T-IS-404 | `var a` <br> `print a * 1` | runtime error |
| T-IS-405 | `var a` <br> `print a / 1` | runtime error |
| T-IS-406 | `var a` <br> `print a < 1` | runtime error |
| T-IS-407 | `var a` <br> `print a = 1` | runtime error |
| T-IS-408 | `var a` <br> `print a and true` | runtime error |
| T-IS-409 | `var a` <br> `print -(a)` | runtime error |
| T-IS-410 | `var a` <br> `print a` | `none` — printing is not an operation on the value; it renders as `none` ([Q31](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-IS-411 | `var a` <br> `var b := a` <br> `print b is none` | `true` — copying is not an operation either |
| T-IS-412 | `var a := none` <br> `print a is none` | `true` — `none` is also a literal, so it can be assigned explicitly |
| T-IS-413 | `var a := 1` <br> `a := none` <br> `print a is none` | `true` — a variable can be reset to `none` |
| T-IS-414 | `var a := [none]` <br> `print a[1] is none` | `true` — `none` may be an element of a composite |
| T-IS-415 | `var t := {x := none}` <br> `print t.x is none` | `true` — and of a tuple |
| T-IS-416 | `var f := func(x) => x` <br> `var n` <br> `print f(n) is none` | `true` — passing and returning `none` are allowed |
| T-IS-417 | `var n` <br> `if n then print 1 end` | runtime error — a condition is an operand position |
| T-IS-418 | `var a := [1], n` <br> `print a[n]` | runtime error — an index is an operand position |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-IS-901 | [Q20](../Language%20D.md#11-spec-ambiguities-and-decisions) — should `is` accept any expression, not just a `Reference`? | Any `Primary`: literal, function literal, parenthesised expression, reference. | `T-IS-108`, `T-IS-109`, `T-IS-111`, `T-IS-112` |
| T-IS-902 | [Q31](../Language%20D.md#11-spec-ambiguities-and-decisions) — do `print` and assignment count as "operations" that `none` may not take part in? | No. Only operators, conditions, indices, range bounds and iterated aggregates reject `none`. | `T-IS-410`, `T-IS-411`, `T-IS-416`–`T-IS-418` |
| T-IS-903 | [Q31](../Language%20D.md#11-spec-ambiguities-and-decisions) — may `none` appear inside an array or tuple literal? | Yes. | `T-IS-414`, `T-IS-415` |
| T-IS-904 | Is there an indicator that matches "any type"? | No — the eight indicators listed are all there are. Not a decision, just the answer. | — |
