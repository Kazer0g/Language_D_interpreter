# Block 09 — Unary Plus and Minus `+` `-`

Prefix `T-UNS` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§8.6 Unary plus and minus](../Language%20D.md#86-unary-plus-and-minus), [§5.5 Unary](../Language%20D.md#55-unary)

**Program**: [`09-op-unary-sign`](09-op-unary-sign) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Unary plus and minus: +, -
op Integer -> Integer
op Real    -> Real
```

**Other types of operands are not allowed.**

```
Unary                               // as written
    : Reference
    | Reference is TypeIndicator
    | [ + | - | not ] Primary
Unary                               // as decided (Q4)
    : [ + | - | not ] Unary
    | Primary [ is TypeIndicator ]
Primary
    : Reference | Literal | FunctionLiteral | ( Expression )
```

As written, `Primary` did not include `Reference`, so `-x` for a variable `x` was not
derivable. The grammar is corrected by
[Q4](../Language%20D.md#11-spec-ambiguities-and-decisions): a prefix operator applies to
any `Unary`, so `-x`, `-t[1]` and `- -5` all parse. The `3xx` tests verify that.

## Result grid

One operand, so a single column.

| operand | unary `+` | unary `-` |
|---|---|---|
| **integer** | Integer | Integer |
| **real** | Real | Real |
| **string** | ✗ | ✗ |
| **boolean** | ✗ | ✗ |
| **array** | ✗ | ✗ |
| **tuple** | ✗ | ✗ |
| **func** | ✗ | ✗ |
| **none** | ✗ | ✗ |

## 0xx — Supported types

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-UNS-001 | `-` Integer → Integer | `print -5` | `-5` |
| T-UNS-002 | `+` Integer → Integer | `print +5` | `5` |
| T-UNS-003 | `-` Real → Real | `print -2.5` | `-2.5` |
| T-UNS-004 | `+` Real → Real | `print +2.5` | `2.5` |
| T-UNS-005 | `-` of a negative | `print -(-5)` | `5` |
| T-UNS-006 | `-` of zero | `print -0` | `0` |
| T-UNS-007 | `-` of a parenthesised expression | `print -(2 + 3)` | `-5` |
| T-UNS-008 | type is preserved | `var a := -5` <br> `print a is int` | `true` |
| T-UNS-009 | type is preserved | `var a := -5.0` <br> `print a is real` | `true` |

## 1xx — Unsupported types

| ID | Program | Expected |
|---|---|---|
| T-UNS-101 | `print -true` | runtime error — boolean is not an operand of unary `-` |
| T-UNS-102 | `print +false` | runtime error |
| T-UNS-103 | `print -"ab"` | runtime error — string is not an operand |
| T-UNS-104 | `print +"ab"` | runtime error |
| T-UNS-105 | `print -[1,2]` | runtime error — array is not an operand |
| T-UNS-106 | `print -{a := 1}` | runtime error — tuple is not an operand |
| T-UNS-107 | `print -(func => 0)` | runtime error — func is not an operand |
| T-UNS-108 | `var n` <br> `print -(n)` | runtime error — `none` cannot act as an operand |
| T-UNS-109 | `print -none` | runtime error |
| T-UNS-110 | `var n` <br> `print -n` | runtime error — same, on the bare reference |

## 2xx — Semantics and precedence

| ID | Program | Expected |
|---|---|---|
| T-UNS-201 | `print -2 + 3` | `1` — unary binds tighter than binary `+` |
| T-UNS-202 | `print -2 * 3` | `-6` — unary binds tighter than `*` |
| T-UNS-203 | `print 5 - -3` | `8` — binary minus followed by unary minus |
| T-UNS-204 | `print 5 + -3` | `2` |
| T-UNS-205 | `print -2 < 0` | `true` — unary binds tighter than the comparison |
| T-UNS-206 | `print -(2 + 3) * 2` | `-10` |
| T-UNS-207 | `print - -5` | `5` — prefixes stack: `Unary : [ + \| - \| not ] Unary` ([Q4](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-UNS-208 | `print -+-5` | `5` — three stacked prefixes |
| T-UNS-209 | `var a := 5` <br> `print -a is int` | runtime error — a prefix applies to the whole `Unary` after it and `is` is inside that operand, so this is `-(a is int)`: unary minus on a boolean ([Q20](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-UNS-210 | `var a := 5` <br> `print (-a) is int` | `true` — parenthesise to test the negated value |

## 3xx — Unary on a reference

Derivable by the corrected grammar ([Q4](../Language%20D.md#11-spec-ambiguities-and-decisions)).

| ID | Program | Expected |
|---|---|---|
| T-UNS-301 | `var a := 5` <br> `print -a` | `-5` — unary minus on a bare reference |
| T-UNS-302 | `var a := 5` <br> `print -(a)` | `-5` — the parenthesised form, also fine |
| T-UNS-303 | `var a := 5` <br> `print 0 - a` | `-5` — the binary form |
| T-UNS-304 | `var t := [5]` <br> `print -t[1]` | `-5` — the accessor binds before the prefix |
| T-UNS-305 | `var f := func => 5` <br> `print -f()` | `-5` — a call result |
| T-UNS-306 | `var t := {x := 2.5}` <br> `print -t.x` | `-2.5` — a tuple element |
| T-UNS-307 | `var a := 5` <br> `print +a` | `5` — unary plus on a reference |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-UNS-901 | [Q4](../Language%20D.md#11-spec-ambiguities-and-decisions) — should `Primary` include `Reference`, or `Unary` be `[ + \| - \| not ] Unary`? | Both: `Unary : [ + \| - \| not ] Unary \| Primary [ is TypeIndicator ]` and `Primary` gains `Reference`. | `T-UNS-301`, `T-UNS-304`–`T-UNS-307` |
| T-UNS-902 | Is `- -5` derivable without parentheses? | Yes — prefixes stack. | `T-UNS-207`, `T-UNS-208` |
| T-UNS-903 | [Q20](../Language%20D.md#11-spec-ambiguities-and-decisions) — how a prefix and `is` combine. | `is` is inside the operand: `-a is int` = `-(a is int)`. | `T-UNS-209`, `T-UNS-210` |
