# Block 05 — Subtraction `-`

Prefix `T-SUB` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§8.2 Subtraction](../Language%20D.md#82-subtraction)

**Program**: [`05-op-subtraction`](05-op-subtraction) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Subtraction: -
Integer - Integer -> Integer
Integer - Real    -> Real
Real    - Integer -> Real
Real    - Real    -> Real
```

**Other types of operands are not allowed.**

## Contrast with addition

`-` accepts **only the four numeric cells**. Every case that
[addition](04-op-addition.md) allows beyond those — `String + String`,
`Array + Array`, `Tuple + Tuple` — is an **error** here. Tests
`T-SUB-110`–`T-SUB-112` are the direct counterparts of `T-ADD-005`–`T-ADD-007`.

The numeric half of the grid is identical to
[multiplication](06-op-multiplication.md) and [division](07-op-division.md); the
positive tests below therefore mirror `T-MUL-001`–`T-MUL-004` and
`T-DIV-001`–`T-DIV-004` cell for cell.

## Result grid

Rows = left operand, columns = right operand.

| `-` | integer | real | string | boolean | array | tuple | func | none |
|---|---|---|---|---|---|---|---|---|
| **integer** | Integer | Real | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
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
| T-SUB-001 | Integer − Integer → Integer | `print 5 - 3` | `2` |
| T-SUB-002 | Integer − Real → Real | `print 5 - 3.5` | `1.5` |
| T-SUB-003 | Real − Integer → Real | `print 5.5 - 3` | `2.5` |
| T-SUB-004 | Real − Real → Real | `print 5.5 - 3.5` | `2.0` |
| T-SUB-005 | negative result | `print 3 - 5` | `-2` |
| T-SUB-006 | zero result | `print 5 - 5` | `0` |
| T-SUB-007 | via variables | `var a := 5, b := 3` <br> `print a - b` | `2` |
| T-SUB-008 | with unary minus | `print 5 - -3` | `8` |

## 1xx — Unsupported types

### 1a. Operand kinds `-` never accepts

| ID | Program | Expected |
|---|---|---|
| T-SUB-101 | `print true - false` | runtime error — boolean is not an operand of `-` |
| T-SUB-102 | `print true - 1` | runtime error |
| T-SUB-103 | `print 1 - true` | runtime error |
| T-SUB-104 | `var f := func => 0` <br> `print f - f` | runtime error |
| T-SUB-105 | `var f := func => 0` <br> `print 1 - f` | runtime error |
| T-SUB-106 | `var n` <br> `print n - 1` | runtime error — `none` cannot act as an operand |
| T-SUB-107 | `var n` <br> `print 1 - n` | runtime error |
| T-SUB-108 | `var n` <br> `print n - n` | runtime error |

### 1b. Counterparts of the cases addition allows

| ID | Program | Expected | Counterpart |
|---|---|---|---|
| T-SUB-110 | `print "ab" - "b"` | runtime error — no string subtraction | [T-ADD-005](04-op-addition.md#0xx--supported-types) |
| T-SUB-111 | `print [1,2] - [2]` | runtime error — no array subtraction | [T-ADD-006](04-op-addition.md#0xx--supported-types) |
| T-SUB-112 | `print {a := 1, b := 2} - {b := 2}` | runtime error — no tuple subtraction | [T-ADD-007](04-op-addition.md#0xx--supported-types) |

### 1c. Mismatched pairs

| ID | Program | Expected |
|---|---|---|
| T-SUB-120 | `print 1 - "2"` | runtime error |
| T-SUB-121 | `print "2" - 1` | runtime error |
| T-SUB-122 | `print 1.5 - "s"` | runtime error |
| T-SUB-123 | `print 1 - [1]` | runtime error |
| T-SUB-124 | `print [1] - 1` | runtime error |
| T-SUB-125 | `print 1 - {a := 1}` | runtime error |
| T-SUB-126 | `print {a := 1} - 1` | runtime error |
| T-SUB-127 | `print [1] - {a := 1}` | runtime error |
| T-SUB-128 | `print true - [1]` | runtime error |

## 2xx — Semantics

| ID | Program | Expected |
|---|---|---|
| T-SUB-201 | `print 10 - 3 - 2` | `5` — left associative, not `10 - (3 - 2)` = `9` |
| T-SUB-202 | `print 10 - 3 + 2` | `9` — `+` and `-` are the same level, left to right |
| T-SUB-203 | `print 10 - (3 - 2)` | `9` — parentheses override |
| T-SUB-204 | `print 10 - 2 * 3` | `4` — `*` binds tighter |
| T-SUB-205 | `print 5 - 2.5 - 1` | `1.5` — the result stays Real once widened |
| T-SUB-206 | `print 1 - 2 - 3 - 4` | `-8` — chained |

## 9xx — Decided

| ID | Question | Decision | Program | Expected |
|---|---|---|---|---|
| T-SUB-901 | [Q27](../Language%20D.md#11-spec-ambiguities-and-decisions) — integer underflow. | 64-bit signed; a result below `-2^63` is a runtime error. | `print -9223372036854775807 - 2` | runtime error |
| T-SUB-902 | [Q28](../Language%20D.md#11-spec-ambiguities-and-decisions) — real precision of the printed result. | IEEE double arithmetic, shortest round-trip rendering. | `print 0.3 - 0.1` | `0.19999999999999998` |
| T-SUB-903 | [Q28](../Language%20D.md#11-spec-ambiguities-and-decisions) — a whole-valued real result. | Always printed with a `.`. | `print 2.5 - 0.5` | `2.0` |
