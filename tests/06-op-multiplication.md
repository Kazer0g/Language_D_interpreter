# Block 06 — Multiplication `*`

Prefix `T-MUL` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§8.3 Multiplication](../Language%20D.md#83-multiplication)

**Program**: [`06-op-multiplication`](06-op-multiplication) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Multiplication: *
Integer * Integer -> Integer
Integer * Real    -> Real
Real    * Integer -> Real
Real    * Real    -> Real
```

**Other types of operands are not allowed.**

The type rules are **identical** to [subtraction](05-op-subtraction.md) and
[division](07-op-division.md): four numeric cells, everything else an error. In
particular there is no "string repetition" (`"ab" * 3`) and no array repetition.

## Result grid

Rows = left operand, columns = right operand.

| `*` | integer | real | string | boolean | array | tuple | func | none |
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
| T-MUL-001 | Integer × Integer → Integer | `print 2 * 3` | `6` |
| T-MUL-002 | Integer × Real → Real | `print 2 * 1.5` | `3.0` |
| T-MUL-003 | Real × Integer → Real | `print 1.5 * 2` | `3.0` |
| T-MUL-004 | Real × Real → Real | `print 1.5 * 2.0` | `3.0` |
| T-MUL-005 | zero | `print 5 * 0` | `0` |
| T-MUL-006 | negative | `print -2 * 3` | `-6` |
| T-MUL-007 | two negatives | `print -2 * -3` | `6` |
| T-MUL-008 | via variables | `var a := 2, b := 3` <br> `print a * b` | `6` |
| T-MUL-009 | identity | `print 7 * 1` | `7` |

## 1xx — Unsupported types

### 1a. Operand kinds `*` never accepts

| ID | Program | Expected |
|---|---|---|
| T-MUL-101 | `print true * false` | runtime error |
| T-MUL-102 | `print true * 2` | runtime error |
| T-MUL-103 | `print 2 * true` | runtime error |
| T-MUL-104 | `var f := func => 0` <br> `print f * 2` | runtime error |
| T-MUL-105 | `var f := func => 0` <br> `print f * f` | runtime error |
| T-MUL-106 | `var n` <br> `print n * 2` | runtime error — `none` cannot act as an operand |
| T-MUL-107 | `var n` <br> `print 2 * n` | runtime error |

### 1b. Counterparts of the cases addition allows

| ID | Program | Expected | Counterpart |
|---|---|---|---|
| T-MUL-110 | `print "ab" * "cd"` | runtime error | [T-ADD-005](04-op-addition.md#0xx--supported-types) |
| T-MUL-111 | `print [1] * [2]` | runtime error | [T-ADD-006](04-op-addition.md#0xx--supported-types) |
| T-MUL-112 | `print {a := 1} * {b := 2}` | runtime error | [T-ADD-007](04-op-addition.md#0xx--supported-types) |

### 1c. Mismatched pairs — no repetition operator

| ID | Program | Expected |
|---|---|---|
| T-MUL-120 | `print "ab" * 3` | runtime error — no string repetition |
| T-MUL-121 | `print 3 * "ab"` | runtime error |
| T-MUL-122 | `print [1] * 3` | runtime error — no array repetition |
| T-MUL-123 | `print 3 * [1]` | runtime error |
| T-MUL-124 | `print 1.5 * "s"` | runtime error |
| T-MUL-125 | `print {a := 1} * 2` | runtime error |
| T-MUL-126 | `print [1] * {a := 1}` | runtime error |
| T-MUL-127 | `print true * [1]` | runtime error |

## 2xx — Semantics

| ID | Program | Expected |
|---|---|---|
| T-MUL-201 | `print 2 * 3 * 4` | `24` — left associative, chained |
| T-MUL-202 | `print 2 + 3 * 4` | `14` — `*` binds tighter than `+` |
| T-MUL-203 | `print (2 + 3) * 4` | `20` — parentheses override |
| T-MUL-204 | `print 2 * 3 / 4` | `1` — `*` and `/` are the same level, left to right: `6 / 4` |
| T-MUL-205 | `print 2 * 1.5 * 2` | `6.0` — the result stays Real once widened |
| T-MUL-206 | `print -2 * 3 + 1` | `-5` — unary binds tighter than `*` |

## 9xx — Decided

| ID | Question | Decision | Program | Expected |
|---|---|---|---|---|
| T-MUL-901 | [Q27](../Language%20D.md#11-spec-ambiguities-and-decisions) — integer overflow. | 64-bit signed; a result outside the range is a runtime error. | `print 4611686018427387904 * 2` | runtime error |
| T-MUL-902 | [Q28](../Language%20D.md#11-spec-ambiguities-and-decisions) — real precision of the printed result. | IEEE double arithmetic, shortest round-trip rendering. | `print 0.1 * 3` | `0.30000000000000004` |
| T-MUL-903 | [Q27](../Language%20D.md#11-spec-ambiguities-and-decisions) — the largest product that fits. | No error at the boundary. | `print 3037000499 * 3037000499` | `9223372030926249001` |
