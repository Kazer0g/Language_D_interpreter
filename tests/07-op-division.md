# Block 07 — Division `/`

Prefix `T-DIV` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§8.4 Division](../Language%20D.md#84-division)

**Program**: [`07-op-division`](07-op-division) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Division: /
Integer / Integer -> Integer    (round down)
Integer / Real    -> Real
Real    / Integer -> Real
Real    / Real    -> Real
```

**Other types of operands are not allowed.**

Same four numeric cells as [subtraction](05-op-subtraction.md) and
[multiplication](06-op-multiplication.md), with one behaviour unique to `/`:
**`Integer / Integer` rounds down** rather than producing a Real.

## Result grid

Rows = left operand, columns = right operand.

| `/` | integer | real | string | boolean | array | tuple | func | none |
|---|---|---|---|---|---|---|---|---|
| **integer** | Integer *(round down)* | Real | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **real** | Real | Real | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **string** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **boolean** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **array** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **tuple** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **func** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **none** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |

## 0xx — Supported types

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-DIV-001 | Integer ÷ Integer → Integer | `print 6 / 2` | `3` |
| T-DIV-002 | Integer ÷ Real → Real | `print 7 / 2.0` | `3.5` |
| T-DIV-003 | Real ÷ Integer → Real | `print 7.0 / 2` | `3.5` |
| T-DIV-004 | Real ÷ Real → Real | `print 7.0 / 2.0` | `3.5` |
| T-DIV-005 | via variables | `var a := 6, b := 2` <br> `print a / b` | `3` |
| T-DIV-006 | identity | `print 5 / 1` | `5` |

## 1xx — Unsupported types

### 1a. Operand kinds `/` never accepts

| ID | Program | Expected |
|---|---|---|
| T-DIV-101 | `print true / false` | runtime error |
| T-DIV-102 | `print true / 2` | runtime error |
| T-DIV-103 | `print 2 / true` | runtime error |
| T-DIV-104 | `var f := func => 0` <br> `print f / 2` | runtime error |
| T-DIV-105 | `var f := func => 0` <br> `print f / f` | runtime error |
| T-DIV-106 | `var n` <br> `print n / 2` | runtime error — `none` cannot act as an operand |
| T-DIV-107 | `var n` <br> `print 2 / n` | runtime error |

### 1b. Counterparts of the cases addition allows

| ID | Program | Expected | Counterpart |
|---|---|---|---|
| T-DIV-110 | `print "ab" / "b"` | runtime error | [T-ADD-005](04-op-addition.md#0xx--supported-types) |
| T-DIV-111 | `print [1,2] / [2]` | runtime error | [T-ADD-006](04-op-addition.md#0xx--supported-types) |
| T-DIV-112 | `print {a := 1} / {b := 2}` | runtime error | [T-ADD-007](04-op-addition.md#0xx--supported-types) |

### 1c. Mismatched pairs

| ID | Program | Expected |
|---|---|---|
| T-DIV-120 | `print "ab" / 2` | runtime error |
| T-DIV-121 | `print 2 / "ab"` | runtime error |
| T-DIV-122 | `print 1.5 / "s"` | runtime error |
| T-DIV-123 | `print [1] / 2` | runtime error |
| T-DIV-124 | `print 2 / [1]` | runtime error |
| T-DIV-125 | `print {a := 1} / 2` | runtime error |
| T-DIV-126 | `print [1] / {a := 1}` | runtime error |
| T-DIV-127 | `print true / [1]` | runtime error |

## 2xx — Integer division rounds down

The behaviour that separates `/` from the other arithmetic operators.

| ID | Program | Expected |
|---|---|---|
| T-DIV-201 | `print 7 / 2` | `3` — rounded down, not `3.5` |
| T-DIV-202 | `print 1 / 2` | `0` |
| T-DIV-203 | `print 9 / 10` | `0` |
| T-DIV-204 | `print 7 / 2.0` | `3.5` — one Real operand suppresses the rounding |
| T-DIV-205 | `print -7 / 2` | `-4` — floor toward negative infinity ([Q7](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-DIV-206 | `print 7 / -2` | `-4` — floor |
| T-DIV-207 | `print -7 / -2` | `3` |
| T-DIV-208 | `print 6 / 3` | `2` — exact division is unaffected |
| T-DIV-209 | `print -1 / 2` | `-1` — floor, not `0` |
| T-DIV-210 | `print -6 / 3` | `-2` — exact negative division |

## 3xx — Precedence and associativity

| ID | Program | Expected |
|---|---|---|
| T-DIV-301 | `print 100 / 10 / 2` | `5` — left associative, not `100 / (10 / 2)` = `20` |
| T-DIV-302 | `print 2 * 6 / 4` | `3` — same level as `*`, left to right |
| T-DIV-303 | `print 6 / 4 * 2` | `2` — `6/4` = `1` first, then `* 2` |
| T-DIV-304 | `print 1 + 6 / 2` | `4` — `/` binds tighter than `+` |
| T-DIV-305 | `print (1 + 6) / 2` | `3` — parentheses override |

## 4xx — Division by zero

Every form is a runtime error ([Q6](../Language%20D.md#11-spec-ambiguities-and-decisions));
the language has no infinities or NaN.

| ID | Program | Expected |
|---|---|---|
| T-DIV-401 | `print 1 / 0` | runtime error |
| T-DIV-402 | `print 1 / 0.0` | runtime error — real division too |
| T-DIV-403 | `print 1.0 / 0` | runtime error |
| T-DIV-404 | `print 0 / 0` | runtime error |
| T-DIV-405 | `print 0.0 / 0.0` | runtime error — no NaN |
| T-DIV-406 | `var z := 0` <br> `print 5 / z` | runtime error — the divisor is checked at run time, not at parse time |
| T-DIV-407 | `print "a"; print 1 / 0` | `a` then runtime error — output before the error stays |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-DIV-901 | [Q6](../Language%20D.md#11-spec-ambiguities-and-decisions) — division by zero: runtime error, or a special real value? | Runtime error, integer and real alike. | `T-DIV-401`–`T-DIV-407` |
| T-DIV-902 | [Q7](../Language%20D.md#11-spec-ambiguities-and-decisions) — "round down" for negative results: floor or truncation? | Floor. | `T-DIV-205`–`T-DIV-207`, `T-DIV-209`, `T-DIV-210` |
| T-DIV-903 | [Q27](../Language%20D.md#11-spec-ambiguities-and-decisions) — the one integer division that overflows. | `-9223372036854775807 - 1` divided by `-1` is a runtime error. | `print (-9223372036854775807 - 1) / -1` → runtime error |
