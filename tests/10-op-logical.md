# Block 10 — Logical Operations `and` `or` `xor` `not`

Prefix `T-LOG` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§8.7 Logical operations](../Language%20D.md#87-logical-operations), [§8.8 Unary negation](../Language%20D.md#88-unary-negation)

**Program**: [`10-op-logical`](10-op-logical) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Logical operations: and, or, xor
Boolean and Boolean -> Boolean
Boolean or  Boolean -> Boolean
Boolean xor Boolean -> Boolean

Unary negation: not
not Boolean -> Boolean
```

Both operands must be Boolean; the result of operations is always boolean.
**Other types of operands are not allowed.**

This block is the mirror image of the arithmetic blocks: `boolean` is the **only**
accepted kind here, and it is rejected by every arithmetic operator.

## Result grid

Rows = left operand, columns = right operand. Identical for `and`, `or` and `xor`.

| `and` / `or` / `xor` | integer | real | string | boolean | array | tuple | func | none |
|---|---|---|---|---|---|---|---|---|
| **integer** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **real** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **string** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **boolean** | ✗ | ✗ | ✗ | **Boolean** | ✗ | ✗ | ✗ | ✗ |
| **array** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **tuple** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **func** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **none** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |

## 0xx — `and`, full truth table

| ID | Program | Expected |
|---|---|---|
| T-LOG-001 | `print true and true` | `true` |
| T-LOG-002 | `print true and false` | `false` |
| T-LOG-003 | `print false and true` | `false` |
| T-LOG-004 | `print false and false` | `false` |

## 01x — `or`, full truth table

| ID | Program | Expected |
|---|---|---|
| T-LOG-011 | `print true or true` | `true` |
| T-LOG-012 | `print true or false` | `true` |
| T-LOG-013 | `print false or true` | `true` |
| T-LOG-014 | `print false or false` | `false` |

## 02x — `xor`, full truth table

| ID | Program | Expected |
|---|---|---|
| T-LOG-021 | `print true xor true` | `false` |
| T-LOG-022 | `print true xor false` | `true` |
| T-LOG-023 | `print false xor true` | `true` |
| T-LOG-024 | `print false xor false` | `false` |

## 03x — Result type and dynamic operands

| ID | Program | Expected |
|---|---|---|
| T-LOG-031 | `var r := true and false` <br> `print r is bool` | `true` — the result is always boolean |
| T-LOG-032 | `var a := true, b := false` <br> `print a or b` | `true` — operand types are checked dynamically |
| T-LOG-033 | `print (1 < 2) and (3 < 4)` | `true` — comparison results are valid operands |

## 1xx — Unsupported types for `and` / `or` / `xor`

| ID | Program | Expected |
|---|---|---|
| T-LOG-101 | `print 1 and true` | runtime error — integer is not a logical operand |
| T-LOG-102 | `print true and 1` | runtime error |
| T-LOG-103 | `print 1 and 0` | runtime error — no truthiness conversion of integers |
| T-LOG-104 | `print 1.5 or true` | runtime error |
| T-LOG-105 | `print "s" and true` | runtime error |
| T-LOG-106 | `print "" or false` | runtime error — no truthiness conversion of strings |
| T-LOG-107 | `print [1] and true` | runtime error |
| T-LOG-108 | `print {a := 1} or true` | runtime error |
| T-LOG-109 | `var f := func => 0` <br> `print f and true` | runtime error |
| T-LOG-110 | `var n` <br> `print n and true` | runtime error — `none` cannot act as an operand |
| T-LOG-111 | `var n` <br> `print true or n` | runtime error — even if the left operand alone would decide it, see `T-LOG-402` |
| T-LOG-112 | `print 1 xor 2` | runtime error — `xor` is logical, not bitwise |

## 2xx — Precedence and associativity

| ID | Program | Expected |
|---|---|---|
| T-LOG-201 | `print true or false and false` | `false` — `Expression : Relation { ( or \| and \| xor ) Relation }` puts all three at **one** level, left to right: `(true or false) and false` ([Q30](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOG-202 | `print true and false or true` | `true` — left to right: `(true and false) or true` |
| T-LOG-203 | `print true or (false and false)` | `true` — parentheses override |
| T-LOG-204 | `print 1 < 2 and 3 > 4` | `false` — comparison binds tighter than the logical level |
| T-LOG-205 | `print true and true and false` | `false` — chained |
| T-LOG-206 | `print false xor true or false` | `true` — mixed operators at one level, left to right |

## 4xx — Evaluation order

No short-circuiting ([Q29](../Language%20D.md#11-spec-ambiguities-and-decisions)):
both operands are always evaluated, and both must be boolean.

| ID | Program | Expected |
|---|---|---|
| T-LOG-401 | `var c := 0` <br> `var f := func is c := c + 1; return false end` <br> `print false and f()` <br> `print c` | `false` then `1` — `f` was called although the left operand already decided the result |
| T-LOG-402 | `var n` <br> `print true or n` | runtime error — the right operand is evaluated and checked |
| T-LOG-403 | `var c := 0` <br> `var f := func is c := c + 1; return true end` <br> `print true or f()` <br> `print c` | `true` then `1` — same for `or` |
| T-LOG-404 | `print false and 1` | runtime error — the right operand's type is checked even when the left is `false` |
| T-LOG-405 | `var c := 0` <br> `var f := func is c := c + 1; return true end` <br> `print f() and f()` <br> `print c` | `true` then `2` — left operand first, then right |

## 5xx — Unary negation `not`

| operand | `not` |
|---|---|
| **boolean** | Boolean |
| every other kind | ✗ |

| ID | Program | Expected |
|---|---|---|
| T-LOG-501 | `print not true` | `false` |
| T-LOG-502 | `print not false` | `true` |
| T-LOG-503 | `print not (true and false)` | `true` |
| T-LOG-504 | `var r := not true` <br> `print r is bool` | `true` |
| T-LOG-505 | `print not 1` | runtime error — integer operand is not allowed |
| T-LOG-506 | `print not 0` | runtime error — no truthiness conversion |
| T-LOG-507 | `print not 1.5` | runtime error |
| T-LOG-508 | `print not "s"` | runtime error |
| T-LOG-509 | `print not ""` | runtime error |
| T-LOG-510 | `print not [1]` | runtime error |
| T-LOG-511 | `print not {a := 1}` | runtime error |
| T-LOG-512 | `print not (func => 0)` | runtime error |
| T-LOG-513 | `print not none` | runtime error — `none` cannot act as an operand |
| T-LOG-514 | `print not (1 < 2)` | `false` — a parenthesised comparison is a valid operand |
| T-LOG-515 | `var b := true` <br> `print not b` | `false` — `not` on a bare reference, derivable by the corrected grammar ([Q4](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOG-516 | `var b := true` <br> `print not (b)` | `false` — the parenthesised form, also fine |
| T-LOG-517 | `print not not true` | `true` — prefixes stack |
| T-LOG-518 | `var a := 1` <br> `print not a is int` | `false` — `not (a is int)`: `is` is inside the operand ([Q20](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOG-519 | `var t := [true]` <br> `print not t[1]` | `false` — the accessor binds before the prefix |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-LOG-901 | [Q29](../Language%20D.md#11-spec-ambiguities-and-decisions) — are `and` and `or` short-circuiting? | No. Both operands are always evaluated and type-checked. | `T-LOG-401`–`T-LOG-405`, `T-LOG-111` |
| T-LOG-902 | [Q30](../Language%20D.md#11-spec-ambiguities-and-decisions) — is `and` really at the same level as `or` and `xor`? | Yes, as the grammar states; left to right. | `T-LOG-201`, `T-LOG-206` |
| T-LOG-903 | [Q4](../Language%20D.md#11-spec-ambiguities-and-decisions) — `not x` on a bare reference. | Derivable; grammar corrected. | `T-LOG-515`, `T-LOG-517`–`T-LOG-519` |
