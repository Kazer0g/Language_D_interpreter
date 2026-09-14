# Block 04 — Addition `+`

Prefix `T-ADD` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§8.1 Addition](../Language%20D.md#81-addition)

**Program**: [`04-op-addition`](04-op-addition) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

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

`+` is the **widest** of the binary operators: it is the only one that accepts `string`,
`array` and `tuple`. Compare with [subtraction](05-op-subtraction.md#contrast-with-addition).

## Result grid

Rows = left operand, columns = right operand.

| `+` | integer | real | string | boolean | array | tuple | func | none |
|---|---|---|---|---|---|---|---|---|
| **integer** | Integer | Real | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **real** | Real | Real | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **string** | ✗ | ✗ | String | ✗ | ✗ | ✗ | ✗ | ✗ |
| **boolean** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **array** | ✗ | ✗ | ✗ | ✗ | Array | ✗ | ✗ | ✗ |
| **tuple** | ✗ | ✗ | ✗ | ✗ | ✗ | Tuple | ✗ | ✗ |
| **func** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **none** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |

## 0xx — Supported types

One test per allowed cell of the grid.

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-ADD-001 | Integer + Integer → Integer | `print 2 + 3` | `5` |
| T-ADD-002 | Integer + Real → Real | `print 2 + 3.5` | `5.5` |
| T-ADD-003 | Real + Integer → Real | `print 1.5 + 2` | `3.5` |
| T-ADD-004 | Real + Real → Real | `print 1.5 + 2.5` | `4.0` |
| T-ADD-005 | String + String → String | `print "ab" + "cd"` | `abcd` |
| T-ADD-006 | Array + Array → Array | `print [1,2] + [3]` | `[1, 2, 3]` |
| T-ADD-007 | Tuple + Tuple → Tuple | `print {a := 1} + {b := 2}` | `{a := 1, b := 2}` |
| T-ADD-008 | Integer + Integer, negative | `print 2 + -3` | `-1` |
| T-ADD-009 | Real + Real, negative | `print 1.5 + -2.5` | `-1.0` |
| T-ADD-010 | String, single quotes | `print 'ab' + "cd"` | `abcd` — quote style is not part of the type |
| T-ADD-011 | via variables | `var a := 2, b := 3` <br> `print a + b` | `5` — operand types are checked dynamically |
| T-ADD-012 | Integer + Integer, zero | `print 0 + 0` | `0` |

## 1xx — Unsupported types

### 1a. Operand kinds `+` never accepts (boolean, func, none)

| ID | Program | Expected |
|---|---|---|
| T-ADD-101 | `print true + false` | runtime error — boolean is not an operand of `+` |
| T-ADD-102 | `print true + 1` | runtime error |
| T-ADD-103 | `print 1 + true` | runtime error |
| T-ADD-104 | `var f := func => 0` <br> `print f + f` | runtime error — func is not an operand of `+` |
| T-ADD-105 | `var f := func => 0` <br> `print 1 + f` | runtime error |
| T-ADD-106 | `var n` <br> `print n + 1` | runtime error — `none` cannot act as an operand |
| T-ADD-107 | `var n` <br> `print 1 + n` | runtime error |
| T-ADD-108 | `var n` <br> `print n + n` | runtime error |
| T-ADD-109 | `print none + none` | runtime error |

### 1b. Mismatched pairs of otherwise supported kinds

| ID | Program | Expected |
|---|---|---|
| T-ADD-110 | `print 1 + "2"` | runtime error — no integer/string mixing |
| T-ADD-111 | `print "2" + 1` | runtime error |
| T-ADD-112 | `print 1.5 + "s"` | runtime error |
| T-ADD-113 | `print "s" + 1.5` | runtime error |
| T-ADD-114 | `print "s" + true` | runtime error |
| T-ADD-115 | `print 1 + [1]` | runtime error |
| T-ADD-116 | `print [1] + 1` | runtime error |
| T-ADD-117 | `print "s" + [1]` | runtime error |
| T-ADD-118 | `print [1] + "s"` | runtime error |
| T-ADD-119 | `print 1 + {a := 1}` | runtime error |
| T-ADD-120 | `print {a := 1} + 1` | runtime error |
| T-ADD-121 | `print [1] + {a := 1}` | runtime error — array and tuple do not mix |
| T-ADD-122 | `print {a := 1} + [1]` | runtime error |
| T-ADD-123 | `print "s" + {a := 1}` | runtime error |
| T-ADD-124 | `print true + [1]` | runtime error |

## 2xx — Semantics

| ID | Program | Expected |
|---|---|---|
| T-ADD-201 | `print 1 + 2 + 3` | `6` — `Factor : Term { [+\|-] Term }` repeats, left to right |
| T-ADD-202 | `print 1 + 2.5 + 1` | `4.5` — result becomes Real and stays Real |
| T-ADD-203 | `print "a" + "b" + "c"` | `abc` — concatenation chains |
| T-ADD-204 | `print [1] + [2] + [3]` | `[1, 2, 3]` |
| T-ADD-205 | `print (1 + 2) + 3` | `6` — parentheses do not change the result |
| T-ADD-206 | `print 2 + 3 * 4` | `14` — `*` binds tighter than `+` |
| T-ADD-207 | `var t := {a := 1, b := 2}` <br> `t := t + {e := 3}` <br> `print t.e` | `3` — the spec's own tuple-extension example |
| T-ADD-208 | `var a := [1]` <br> `var b := a + [2]` <br> `print a` | `[1]` — `+` produces a new value, it does not mutate |

## 4xx — Edge cases

| ID | Program | Expected |
|---|---|---|
| T-ADD-401 | `print [] + []` | `[]` — empty arrays concatenate |
| T-ADD-402 | `print [] + [1]` | `[1]` |
| T-ADD-403 | `print [1] + []` | `[1]` |
| T-ADD-404 | `print "" + "a"` | `a` — empty string is the identity |
| T-ADD-405 | `var t := []` <br> `t[10] := 1` <br> `print t + [2]` | `[1, 2]` — sparse keys are renumbered on concatenation ([Q9](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ADD-406 | `print {a := 1} + {a := 2}` | runtime error — duplicate names ([Q10](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ADD-407 | `print {1} + {2}` | `{1, 2}` — unnamed elements concatenate positionally |
| T-ADD-408 | `print [ [1] ] + [ [2] ]` | `[[1], [2]]` — nesting is preserved, not flattened |
| T-ADD-409 | `var t := []` <br> `t[5] := "a"; t[2] := "b"` <br> `print t + t` | `[b, a, b, a]` — both sides walked in ascending key order |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-ADD-901 | [Q9](../Language%20D.md#11-spec-ambiguities-and-decisions) — array concatenation when keys are sparse: renumber or merge? | Renumber: left values in ascending key order, then right, under keys `1..n`. | `T-ADD-405`, `T-ADD-409` |
| T-ADD-902 | [Q10](../Language%20D.md#11-spec-ambiguities-and-decisions) — tuple concatenation producing duplicate element names. | Runtime error. | `T-ADD-406` |
| T-ADD-903 | [Q27](../Language%20D.md#11-spec-ambiguities-and-decisions) — integer overflow. | 64-bit signed; a result outside the range is a runtime error. | `print 9223372036854775807 + 1` → runtime error |
