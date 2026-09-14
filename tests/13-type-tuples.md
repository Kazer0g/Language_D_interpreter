# Block 13 — Tuples

Prefix `T-TUP` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§7.6 Tuple](../Language%20D.md#76-tuple), [§6 References](../Language%20D.md#6-references-accessors)

**Program**: [`13-type-tuples`](13-type-tuples) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Tuple
    : { TupleElement { , TupleElement } }
TupleElement
    : [ Identifier := ] Expression
Access
    : Reference . IDENT
    | Reference . IntegerLiteral
```

- A tuple is a **fixed-size** collection of possibly named values of arbitrary types.
- Element names, if specified, are **unique within the tuple**.
- The structure **cannot be modified**; the only way to change the composition of the
  elements is to **add another tuple to it**.
- Access is by name or by **position, numbering starts with one**.
- Named and unnamed elements can follow in **any order**.

## 0xx — Literals and access

| ID | Program | Expected |
|---|---|---|
| T-TUP-001 | `var t := {a := 1, b := 2}` <br> `print t.a` | `1` — access by name |
| T-TUP-002 | `var t := {a := 1, b := 2}` <br> `print t.2` | `2` — access by position, starting at one |
| T-TUP-003 | `var t := {a := 1, b := 2}` <br> `print t.b, t.2` | `2` `2` — name and position address the same element |
| T-TUP-004 | `var t := {1, 2, 3}` <br> `print t.1` | `1` — all elements unnamed |
| T-TUP-005 | `var t := {1, a := 2, "s"}` <br> `print t.1, t.a, t.3` | `1` `2` `s` — named and unnamed in any order |
| T-TUP-006 | `var t := {a := 5, b := "sss", 12.34}` <br> `print t.b` | `sss` — the spec's own literal |
| T-TUP-007 | `var c := 1, d := 2` <br> `var t := {a := 1, b := 2, c + d}` <br> `print t.3` | `3` — an element is any expression |
| T-TUP-008 | `var t := {x := 1}` <br> `print t is {}` | `true` — the `{ }` type indicator |
| T-TUP-009 | `var t := {a := [1,2]}` <br> `print t.a[2]` | `2` — an array inside a tuple |
| T-TUP-010 | `var t := {a := {b := 7}}` <br> `print t.a.b` | `7` — nested tuples |
| T-TUP-011 | `var t := {f := func => 9}` <br> `print t.f()` | `9` — a function inside a tuple |
| T-TUP-012 | `var t := {a := 1, b := 2.5, c := "s", d := true}` <br> `print t.b` | `2.5` — arbitrary element types |

## 1xx — Invalid literals and access

| ID | Program | Expected |
|---|---|---|
| T-TUP-101 | `var t := {a := 1, a := 2}` | runtime error — element names must be unique within the tuple; raised when the literal is evaluated ([Q24](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-TUP-102 | `var t := {a := 1}` <br> `print t.b` | runtime error — no such element name |
| T-TUP-103 | `var t := {a := 1}` <br> `print t.2` | runtime error — position beyond the fixed size |
| T-TUP-104 | `var t := {a := 1}` <br> `print t.0` | runtime error — numbering starts with one |
| T-TUP-105 | `var t := {a := 1}` <br> `var i := 1` <br> `print t.i` | runtime error — `Reference . IDENT` reads `i` as an **element name**, not a variable |
| T-TUP-106 | `var t := {a := 1}` <br> `print t.(1)` | syntax error — the position must be an `IntegerLiteral`, not an expression |
| T-TUP-107 | `var t := {a := 1}` <br> `print t.1.5` | syntax error — the position must be an integer literal |
| T-TUP-108 | `var t := {}` | syntax error — the grammar requires at least one `TupleElement`; `{ }` exists only as a type indicator ([Q33](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-TUP-109 | `var t := {a := 1,}` | syntax error — trailing comma |
| T-TUP-110 | `var t := {a := 1}` <br> `print t[1]` | runtime error — a tuple is accessed with `.`, not `[ ]` |
| T-TUP-111 | `var a := 1` <br> `print a.b` | runtime error — dot access on a non-tuple |
| T-TUP-112 | `var t := {a = 1}` | syntax error — the separator is `:=`, not `=` |
| T-TUP-113 | `var t := {1 := 5}` | syntax error — a tuple element **name** must be an `Identifier`, and `1` tokenizes as an `IntegerLiteral`, never an identifier ([Q22](../Language%20D.md#11-spec-ambiguities-and-decisions)); contrast with `T-TUP-004`, where `1` is a valid **unnamed** element |

## 2xx — Immutability and extension

The structure of tuples cannot be modified; the only way to change the composition is to
add another tuple.

| ID | Program | Expected |
|---|---|---|
| T-TUP-201 | `var t := {a := 1, b := 2, 3}` <br> `t := t + {e := 3}` <br> `print t.e` | `3` — the spec's own extension example |
| T-TUP-202 | `var t := {a := 1}` <br> `t := t + {b := 2}` <br> `print t.1, t.2` | `1` `2` — positions continue across the concatenation |
| T-TUP-203 | `var t := {a := 1}` <br> `var u := t + {b := 2}` <br> `print t is {}, u.b` | `true` `2` — `+` produces a new tuple |
| T-TUP-204 | `var t := {a := 1}` <br> `t.a := 5` <br> `print t.a` | `5` — changing a value, not the structure, is allowed ([Q11](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-TUP-205 | `var t := {a := 1}` <br> `t.b := 5` | runtime error — this would modify the structure |
| T-TUP-206 | `var t := {a := 1}` <br> `t.1 := 5` <br> `print t.a` | `5` — positional assignment, same rule |
| T-TUP-207 | `var t := {a := 1}` <br> `print t + {a := 2}` | runtime error — duplicate names after concatenation ([Q10](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-TUP-208 | `var t := {1}` <br> `print t + {2}` | `{1, 2}` — unnamed elements concatenate positionally |
| T-TUP-209 | `var t := {a := 1}` <br> `t.2 := 5` | runtime error — position 2 does not exist; no growth by assignment |
| T-TUP-210 | `var t := {a := 1}` <br> `t.a := "s"` <br> `print t.a is string` | `true` — the element may change type; only the structure is fixed |
| T-TUP-211 | `var t := {a := 1, b := 2}` <br> `var u := t + {c := 3}` <br> `print u.1, u.2, u.3` | `1` `2` `3` — positions continue across the join |

## 3xx — Tuples in other constructs

| ID | Program | Expected |
|---|---|---|
| T-TUP-301 | `var t := {a := 1, b := 2}` <br> `var sum := 0` <br> `for v in t loop sum := sum + v end` <br> `print sum` | `3` — a single loop expression may be a tuple; the loop variable takes the **values** ([Q13](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-TUP-306 | `for v in {a := 1, 2, b := 3} loop print v end` | `1` `2` `3` — positional order, names not visible |
| T-TUP-302 | `var f := func(t) => t.a` <br> `print f({a := 4})` | `4` — a tuple passed as an argument |
| T-TUP-303 | `var f := func => {a := 4}` <br> `print f().a` | `4` — a tuple returned from a function |
| T-TUP-304 | `var a := [ {x := 1}, {x := 2} ]` <br> `print a[2].x` | `2` — chained accessors |
| T-TUP-305 | `var t := {a := 1}` <br> `print t.a + 1` | `2` — an element is an ordinary operand |

## 4xx — Edge cases

| ID | Program | Expected |
|---|---|---|
| T-TUP-401 | `var t := {1}` <br> `print t.1` | `1` — single unnamed element |
| T-TUP-402 | `var t := {a := 1}` <br> `print t.a` | `1` — single named element |
| T-TUP-403 | `var t := { {a := 1} }` <br> `print t.1.a` | `1` — a tuple whose only element is a tuple |
| T-TUP-404 | `var t := {a := 1}, u := t` <br> `u := u + {b := 2}` <br> `print t is {}` | `true` — extending the copy leaves the original a tuple |
| T-TUP-405 | `var t := {a := 1}` <br> `print t` | `{a := 1}` — rendering of a tuple ([Q15](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-TUP-406 | `var t := {a := 1, b := 2}` <br> `print t.a, t.b, t.1, t.2` | `1` `2` `1` `2` — both access forms agree |
| T-TUP-407 | `print {1, b := 2.0, "s"}` | `{1, b := 2.0, s}` — unnamed as the value, named as `name := value` |
| T-TUP-408 | `var t := {a := 1}, u := t` <br> `u.a := 5` <br> `print t.a` | `5` — tuples alias on assignment ([Q26](../Language%20D.md#11-spec-ambiguities-and-decisions)) |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-TUP-901 | [Q10](../Language%20D.md#11-spec-ambiguities-and-decisions) — concatenation producing duplicate names. | Runtime error. | `T-TUP-207` |
| T-TUP-902 | [Q11](../Language%20D.md#11-spec-ambiguities-and-decisions) — is `t.a := v` allowed? | Yes, for an existing element by name or position; a new name or position is a runtime error. | `T-TUP-204`–`T-TUP-206`, `T-TUP-209`, `T-TUP-210` |
| T-TUP-903 | [Q13](../Language%20D.md#11-spec-ambiguities-and-decisions) — iterating a tuple: are element names reachable? | No — values only, in positional order. | `T-TUP-301`, `T-TUP-306` |
| T-TUP-904 | [Q33](../Language%20D.md#11-spec-ambiguities-and-decisions) — is `{}` an empty tuple literal or only a type indicator? | Only a type indicator. | `T-TUP-108`, [T-IS-006](11-op-type-check-is.md#0xx--matching-indicator-one-per-row) |
| T-TUP-905 | Can a tuple element be addressed by a computed position? | No — the grammar allows only an `IntegerLiteral` after `.`; there is no decision to make. `t.i` looks up the **name** `i`. | `T-TUP-105`, `T-TUP-106` |
| T-TUP-906 | [Q22](../Language%20D.md#11-spec-ambiguities-and-decisions) — can a tuple element be **named** `1`, the way it can be **accessed** by position `1`? | No. `TupleElement : [ Identifier := ] Expression` — the name before `:=` must lex as `IDENT`, and `IDENT` cannot start with a digit. `1` always lexes as `INTEGER`, so `{1 := 5}` is a syntax error; `1` before `:=` is simply never derivable as a name. This is the same rule as `T-LEX-405` (`var 1a := 1`), applied to `Identifier` wherever the grammar uses it — variables, tuple element names, function parameters. | `T-TUP-113` |
