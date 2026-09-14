# Block 08 — Comparisons `<` `>` `<=` `>=` `=` `/=`

Prefix `T-CMP` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§8.5 Comparisons](../Language%20D.md#85-comparisons), [§5.2 Relation](../Language%20D.md#52-relation-comparison-level)

**Program**: [`08-op-comparison`](08-op-comparison) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Comparisons: <, >, <=, >=, =, /=
Integer op Integer -> boolean
Integer op Real    -> boolean
Real    op Integer -> boolean
Real    op Real    -> boolean
```

**Other types of operands are not allowed.**

```
Relation
    : Factor [ ( < | <= | > | >= | = | /= ) Factor ]
```

Two consequences of that production, both tested in `3xx`:

- the comparison is **optional**, so a `Relation` may be a bare `Factor`;
- it does **not repeat**, so comparisons **do not chain** (`1 < 2 < 3` is not derivable).

The operand table covers **only numeric operands**. By decision
[Q5](../Language%20D.md#11-spec-ambiguities-and-decisions) this project **extends `=`
and `/=`** to `String = String` and `Boolean = Boolean`; the four ordering operators stay
numeric-only, and arrays, tuples, functions and `none` are never comparable.

## Result grid

Rows = left operand, columns = right operand. Two grids, because the extension applies
to equality only.

`<` `>` `<=` `>=` — as the description states:

| ordering | integer | real | string | boolean | array | tuple | func | none |
|---|---|---|---|---|---|---|---|---|
| **integer** | boolean | boolean | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **real** | boolean | boolean | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **string** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **boolean** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **array** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **tuple** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **func** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **none** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |

`=` `/=` — with the two extended cells in bold:

| equality | integer | real | string | boolean | array | tuple | func | none |
|---|---|---|---|---|---|---|---|---|
| **integer** | boolean | boolean | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **real** | boolean | boolean | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **string** | ✗ | ✗ | **boolean** | ✗ | ✗ | ✗ | ✗ | ✗ |
| **boolean** | ✗ | ✗ | ✗ | **boolean** | ✗ | ✗ | ✗ | ✗ |
| **array** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **tuple** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **func** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |
| **none** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ |

## 0xx — Supported types

Four allowed cells × six operators.

### `<`

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-CMP-001 | Integer, Integer | `print 1 < 2` | `true` |
| T-CMP-002 | Integer, Real | `print 1 < 1.5` | `true` |
| T-CMP-003 | Real, Integer | `print 2.5 < 2` | `false` |
| T-CMP-004 | Real, Real | `print 1.5 < 2.5` | `true` |

### `>`

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-CMP-005 | Integer, Integer | `print 2 > 1` | `true` |
| T-CMP-006 | Integer, Real | `print 2 > 1.5` | `true` |
| T-CMP-007 | Real, Integer | `print 1.5 > 2` | `false` |
| T-CMP-008 | Real, Real | `print 2.5 > 1.5` | `true` |

### `<=`

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-CMP-009 | Integer, Integer | `print 2 <= 2` | `true` — boundary |
| T-CMP-010 | Integer, Real | `print 2 <= 2.0` | `true` — boundary after widening |
| T-CMP-011 | Real, Integer | `print 2.5 <= 2` | `false` |
| T-CMP-012 | Real, Real | `print 2.0 <= 2.0` | `true` |

### `>=`

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-CMP-013 | Integer, Integer | `print 2 >= 2` | `true` — boundary |
| T-CMP-014 | Integer, Real | `print 2 >= 2.5` | `false` |
| T-CMP-015 | Real, Integer | `print 2.0 >= 2` | `true` — boundary after widening |
| T-CMP-016 | Real, Real | `print 1.5 >= 2.5` | `false` |

### `=`

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-CMP-017 | Integer, Integer | `print 3 = 3` | `true` |
| T-CMP-018 | Integer, Real | `print 3 = 3.0` | `true` — widened before comparison |
| T-CMP-019 | Real, Integer | `print 3.5 = 3` | `false` |
| T-CMP-020 | Real, Real | `print 3.5 = 3.5` | `true` |

### `/=`

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-CMP-021 | Integer, Integer | `print 3 /= 4` | `true` |
| T-CMP-022 | Integer, Real | `print 3 /= 3.0` | `false` |
| T-CMP-023 | Real, Integer | `print 3.5 /= 3` | `true` |
| T-CMP-024 | Real, Real | `print 3.5 /= 3.5` | `false` |

### Result is always boolean

| ID | Program | Expected |
|---|---|---|
| T-CMP-025 | `var r := 1 < 2` <br> `print r is bool` | `true` |
| T-CMP-026 | `var a := 1, b := 2` <br> `print a < b` | `true` — operand types are checked dynamically |

### Extended cells: `=` and `/=` on strings and booleans ([Q5](../Language%20D.md#11-spec-ambiguities-and-decisions))

| ID | Cell | Program | Expected |
|---|---|---|---|
| T-CMP-027 | String = String | `print "a" = "a"` | `true` |
| T-CMP-028 | String = String | `print "a" = "b"` | `false` |
| T-CMP-029 | String /= String | `print "a" /= "b"` | `true` |
| T-CMP-030 | String = String | `print 'a' = "a"` | `true` — quote style is not part of the value |
| T-CMP-031 | String = String | `print "" = ""` | `true` — empty strings are equal |
| T-CMP-032 | String = String | `print "A" = "a"` | `false` — comparison is exact, case-sensitive |
| T-CMP-033 | Boolean = Boolean | `print true = true` | `true` |
| T-CMP-034 | Boolean = Boolean | `print true = false` | `false` |
| T-CMP-035 | Boolean /= Boolean | `print true /= false` | `true` |
| T-CMP-036 | via variables | `var s := "ab"` <br> `print s = "a" + "b"` | `true` — compares the resulting value |

## 1xx — Unsupported types

One representative per row kind, for a representative operator; each case applies to all
six operators alike.

### 1a. Operand kinds comparison never accepts

| ID | Program | Expected |
|---|---|---|
| T-CMP-101 | `print true < false` | runtime error — no ordering of booleans |
| T-CMP-102 | `print true = false` | `false` — boolean equality is an extended cell ([Q5](../Language%20D.md#11-spec-ambiguities-and-decisions)); kept here so the verdict change is visible |
| T-CMP-103 | `print "a" < "b"` | runtime error — no ordering of strings |
| T-CMP-104 | `print "a" = "a"` | `true` — string equality is an extended cell ([Q5](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-CMP-105 | `print [1] = [1]` | runtime error — no array equality |
| T-CMP-111 | `print "a" <= "a"` | runtime error — `<=` is ordering, not extended |
| T-CMP-112 | `print true >= false` | runtime error |
| T-CMP-106 | `print [1] < [2]` | runtime error |
| T-CMP-107 | `print {a := 1} = {a := 1}` | runtime error — no tuple equality |
| T-CMP-108 | `var f := func => 0` <br> `print f = f` | runtime error — the only operation on a function is a call |
| T-CMP-109 | `var n` <br> `print n = 1` | runtime error — `none` cannot act as an operand |
| T-CMP-110 | `var n, m` <br> `print n = m` | runtime error — even `none` to `none` |

### 1b. Mismatched pairs

| ID | Program | Expected |
|---|---|---|
| T-CMP-120 | `print 1 < "2"` | runtime error |
| T-CMP-121 | `print "2" > 1` | runtime error |
| T-CMP-122 | `print 1 = "1"` | runtime error |
| T-CMP-123 | `print 1 = true` | runtime error |
| T-CMP-124 | `print 1.5 /= true` | runtime error |
| T-CMP-125 | `print 1 < [1]` | runtime error |
| T-CMP-126 | `print [1] = {a := 1}` | runtime error |
| T-CMP-127 | `print "s" = [1]` | runtime error |
| T-CMP-128 | `print "true" = true` | runtime error — string and boolean do not mix, even when they look alike |
| T-CMP-129 | `print "1" = 1.0` | runtime error |

## 3xx — Relation structure

| ID | Program | Expected |
|---|---|---|
| T-CMP-301 | `print 1 < 2 < 3` | syntax error — `Relation` allows **at most one** comparison |
| T-CMP-302 | `print 1 = 2 = 3` | syntax error — comparisons do not chain |
| T-CMP-303 | `print (1 < 2) and (2 < 3)` | `true` — chaining is expressed with a logical operator |
| T-CMP-304 | `print 1 + 1 < 3` | `true` — `Factor` binds tighter than the comparison |
| T-CMP-305 | `print 1 < 2 and 3 < 4` | `true` — comparison binds tighter than `and` |
| T-CMP-306 | `print 2 * 2 = 4` | `true` — `Term` binds tighter than the comparison |
| T-CMP-307 | `print 1` | `1` — a `Relation` may be a bare `Factor`, the comparison is optional |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-CMP-901 | [Q5](../Language%20D.md#11-spec-ambiguities-and-decisions) — should `=` and `/=` be extended beyond numerics? | To strings and booleans only. `T-CMP-102` and `T-CMP-104` changed verdict to positive; `T-CMP-105`, `T-CMP-107`, `T-CMP-110` stay errors. | `T-CMP-027`–`T-CMP-036`, `T-CMP-101`–`T-CMP-112` |
| T-CMP-902 | [Q28](../Language%20D.md#11-spec-ambiguities-and-decisions) — exact equality of reals after arithmetic. | IEEE doubles compare exactly; `0.1 + 0.2 = 0.3` is `false`. | `print 0.1 + 0.2 = 0.3` → `false` |
| T-CMP-903 | [Q5](../Language%20D.md#11-spec-ambiguities-and-decisions) — comparing a boolean **expression** result. | `(1 < 2) = true` is boolean equality, allowed. | `print (1 < 2) = true` → `true` (also [T-PREC-203](18-expression-precedence.md#2xx--non-repeating-relation)) |
