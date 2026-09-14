# Block 19 — Implicit Type Conversions

Prefix `T-CONV` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids), [type matrix](Tests.md#type-matrix-convention)
Spec: [§9 Implicit Type Conversions](../Language%20D.md#9-implicit-type-conversions)

**Program**: [`19-implicit-conversions`](19-implicit-conversions) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

"Implicit type conversions are supported." The only conversion the operation table makes
concrete is **integer → real widening** in mixed-type arithmetic and comparison:

```
Integer op Real -> (Real) op Real
Real op Integer -> Real op (Real)
where op in { +, -, *, /, <, <=, >, >=, =, /= }
```

Where the operator blocks 04–09 verify *which* mixed pairs are accepted, this block
verifies *what the widening does to the result* — and, just as importantly, that no
**other** conversion happens: no number↔string, no truthiness, no real→integer narrowing.

## 0xx — Integer → real widening

| ID | Program | Expected |
|---|---|---|
| T-CONV-001 | `print 1 + 2.5` | `3.5` — the integer is widened |
| T-CONV-002 | `var r := 1 + 2.5` <br> `print r is real` | `true` — the result type is Real |
| T-CONV-003 | `var r := 1 + 2` <br> `print r is int` | `true` — no widening between two integers |
| T-CONV-004 | `print 5 - 2.5` | `2.5` |
| T-CONV-005 | `print 2 * 1.5` | `3.0` |
| T-CONV-006 | `print 2.5 * 2` | `5.0` |
| T-CONV-007 | `var r := 3 * 2.0` <br> `print r is real` | `true` — Real even when the value is whole |
| T-CONV-008 | `print 1 = 1.0` | `true` — widened before comparison |
| T-CONV-009 | `print 2 < 2.5` | `true` |
| T-CONV-010 | `print 3 >= 3.0` | `true` — boundary after widening |

## 1xx — Division: where widening changes the answer

`Integer / Integer` rounds down; one Real operand suppresses that.

| ID | Program | Expected |
|---|---|---|
| T-CONV-101 | `print 7 / 2` | `3` — no widening, integer division |
| T-CONV-102 | `print 7 / 2.0` | `3.5` — the right operand forces widening |
| T-CONV-103 | `print 7.0 / 2` | `3.5` — the left operand forces widening |
| T-CONV-104 | `var r := 7 / 2` <br> `print r is int` | `true` |
| T-CONV-105 | `var r := 7 / 2.0` <br> `print r is real` | `true` |
| T-CONV-106 | `print 1 / 2 + 0.5` | `0.5` — `1/2` is `0` first, then widened |
| T-CONV-107 | `print 1 / 2.0 + 0.5` | `1.0` — widening happens before the division |

## 2xx — Widening propagates through an expression

| ID | Program | Expected |
|---|---|---|
| T-CONV-201 | `print 1 + 2 + 3.0` | `6.0` — Real once, Real after |
| T-CONV-202 | `print 1.0 + 2 + 3` | `6.0` |
| T-CONV-203 | `print (1 + 2) * 1.5` | `4.5` |
| T-CONV-204 | `var a := 1` <br> `a := a + 0.5` <br> `print a is real` | `true` — a variable's runtime type changes |
| T-CONV-205 | `var a := 1.5` <br> `a := a * 2` <br> `print a is real` | `true` — a Real stays Real |
| T-CONV-206 | `var s := 0` <br> `for i in 1..3 loop s := s + 1.5 end` <br> `print s is real` | `true` — the widening persists across iterations |

## 3xx — Widening is one-directional

There is no narrowing: a Real never becomes an Integer, and `is` reports the value's
actual type rather than a compatible one.

| ID | Program | Expected |
|---|---|---|
| T-CONV-301 | `var a := 1` <br> `print a is real` | `false` — an integer is not a real |
| T-CONV-302 | `var a := 1.0` <br> `print a is int` | `false` — even a whole real is not an integer |
| T-CONV-303 | `var a := 2.0 + 1.0` <br> `print a is int` | `false` — a whole result stays Real |
| T-CONV-304 | `var a := [1,2,3]` <br> `print a[1.0]` | runtime error — an index is not narrowed to an integer |
| T-CONV-305 | `for i in 1.0..3.0 loop print i end` | runtime error — range bounds are not narrowed ([Q12](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-CONV-306 | `var t := {a := 1}` <br> `print t.1` | `1` — a position is an integer literal; `t.1.0` would be a syntax error, there is nothing to narrow |

## 4xx — Conversions that do *not* happen

| ID | Program | Expected |
|---|---|---|
| T-CONV-401 | `print 1 + "2"` | runtime error — no number → string conversion |
| T-CONV-402 | `print "1" + 2` | runtime error — no string → number conversion |
| T-CONV-403 | `print "1" + "2"` | `12` — string concatenation, **not** arithmetic |
| T-CONV-404 | `print 1 + true` | runtime error — no boolean → number conversion |
| T-CONV-405 | `print 1 and true` | runtime error — no number → boolean conversion |
| T-CONV-406 | `print not 1` | runtime error — no truthiness |
| T-CONV-407 | `print "s" + [1]` | runtime error — no string ↔ array conversion |
| T-CONV-408 | `print 1 = "1"` | runtime error — no conversion before comparison |
| T-CONV-409 | `var n` <br> `print n + 0` | runtime error — `none` is never converted |
| T-CONV-410 | `print [1] + {a := 1}` | runtime error — array and tuple do not convert into each other |
| T-CONV-411 | `if 1 then print "y" end` | runtime error — a non-boolean condition is not converted ([Q34](../Language%20D.md#11-spec-ambiguities-and-decisions), [T-IF-901](15-stmt-conditionals.md#9xx--decided)) |
| T-CONV-412 | `print "a" = 1` | runtime error — no conversion for the extended equality either ([Q5](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-CONV-413 | `print true = 1` | runtime error — boolean and integer never convert |
| T-CONV-414 | `print 1, "1"` | `1 1` — `print` renders both the same way but they stay different values |
| T-CONV-415 | `print 1 = 1.0, 1 is int, 1.0 is real` | `true true true` — widening compares them equal without changing either's type |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-CONV-901 | Is integer → real widening the only implicit conversion? | Yes ([§9](../Language%20D.md#9-implicit-type-conversions)). | the whole `4xx` section |
| T-CONV-902 | [Q15](../Language%20D.md#11-spec-ambiguities-and-decisions) — does `print` convert values to strings? | It renders them; no string value is created and no other operation sees a conversion. | `T-CONV-414`, [T-PRN-4xx](17-stmt-print.md#4xx--output-format-q15) |
| T-CONV-903 | [Q34](../Language%20D.md#11-spec-ambiguities-and-decisions) — does a condition context convert its operand to boolean? | No; runtime error. | `T-CONV-411` |
| T-CONV-904 | [Q27](../Language%20D.md#11-spec-ambiguities-and-decisions) — precision loss when a large integer is widened to real. | Rounds to the nearest double, silently. | `print 9007199254740993 + 0.0` → `9007199254740992.0` |
