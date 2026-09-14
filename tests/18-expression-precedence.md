# Block 18 — Expression Precedence and Associativity

Prefix `T-PREC` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§5 Expressions](../Language%20D.md#5-expressions), [§6 References](../Language%20D.md#6-references-accessors)

**Program**: [`18-expression-precedence`](18-expression-precedence) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Expression : Relation { ( or | and | xor ) Relation }      // lowest
Relation   : Factor [ ( < | <= | > | >= | = | /= ) Factor ]
Factor     : Term { ( + | - ) Term }                       // Q3: operator required
Term       : Unary { ( * | / ) Unary }
Unary      : [ + | - | not ] Unary | Primary [ is TypeIndicator ]   // Q4, Q20
Primary    : Reference | Literal | FunctionLiteral | ( Expression )  // highest
```

This is the grammar **as decided** ([§10](../Language%20D.md#10-full-grammar-summary));
the three corrections against the description are marked. Five levels. Where the other
blocks check *what each operator does*, this block checks *how the levels compose*.

| Level | Operators | Repeats? |
|---|---|---|
| 1 `Expression` | `or` `and` `xor` | yes — all three at one level ([Q30](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| 2 `Relation` | `<` `<=` `>` `>=` `=` `/=` | **no** — at most one |
| 3 `Factor` | `+` `-` | yes |
| 4 `Term` | `*` `/` | yes |
| 5 `Unary` | prefix `+` `-` `not` | yes — prefixes stack |
| 5 `Unary` | postfix `is` | **no** — at most one, and it binds inside a prefix |
| 6 `Reference` | `[ ]` `( )` `.` | yes — accessors chain, only off an identifier |

## 0xx — Level against level

| ID | Program | Expected | Shows |
|---|---|---|---|
| T-PREC-001 | `print 2 + 3 * 4` | `14` | `Term` binds tighter than `Factor` |
| T-PREC-002 | `print 2 * 3 + 4` | `10` | same, other order |
| T-PREC-003 | `print (2 + 3) * 4` | `20` | `Primary` parentheses override |
| T-PREC-004 | `print 1 + 2 < 4` | `true` | `Factor` binds tighter than `Relation` |
| T-PREC-005 | `print 2 * 2 = 4` | `true` | `Term` binds tighter than `Relation` |
| T-PREC-006 | `print 1 < 2 and 3 < 4` | `true` | `Relation` binds tighter than `Expression` |
| T-PREC-007 | `print 1 + 1 = 2 and 2 + 2 = 4` | `true` | three levels at once |
| T-PREC-008 | `print -2 + 3` | `1` | `Unary` binds tighter than `Factor` |
| T-PREC-009 | `print -2 * 3` | `-6` | `Unary` binds tighter than `Term` |
| T-PREC-010 | `print not (1 < 2)` | `false` | `not` applies to a parenthesised `Primary` |
| T-PREC-011 | `print 6 / 3 - 1` | `1` | `Term` binds tighter than `Factor` |
| T-PREC-012 | `print 1 + 6 / 2 * 2` | `7` | `/` then `*` then `+` |

## 1xx — Associativity

| ID | Program | Expected | Shows |
|---|---|---|---|
| T-PREC-101 | `print 10 - 3 - 2` | `5` | `-` is left associative, not `9` |
| T-PREC-102 | `print 100 / 10 / 2` | `5` | `/` is left associative, not `20` |
| T-PREC-103 | `print 2 * 6 / 4` | `3` | `*` and `/` share a level, left to right |
| T-PREC-104 | `print 6 / 4 * 2` | `2` | `6/4` = `1` first — integer division makes the order visible |
| T-PREC-105 | `print 10 - 3 + 2` | `9` | `+` and `-` share a level, left to right |
| T-PREC-106 | `print 1 + 2 + 3` | `6` | chained `+` |
| T-PREC-107 | `print true and false or true` | `true` | all logical operators share one level, left to right |
| T-PREC-108 | `print true or false and false` | `false` | `(true or false) and false` — one level, left to right ([Q30](../Language%20D.md#11-spec-ambiguities-and-decisions), [T-LOG-902](10-op-logical.md#9xx--decided)) |
| T-PREC-109 | `print 1 - (2 - 3)` | `2` | parentheses defeat left associativity |

## 2xx — Non-repeating relation

| ID | Program | Expected |
|---|---|---|
| T-PREC-201 | `print 1 < 2 < 3` | syntax error — `Relation` allows at most one comparison |
| T-PREC-202 | `print 1 = 2 = 3` | syntax error |
| T-PREC-203 | `print (1 < 2) = true` | `true` — the inner relation is a `Primary`, and boolean equality is allowed ([Q5](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-PREC-204 | `print 1 < 2 and 2 < 3` | `true` — the derivable way to chain |

## 3xx — Reference accessors bind tightest

Accessors are part of `Reference`, below `Primary` in the tree.

| ID | Program | Expected |
|---|---|---|
| T-PREC-301 | `var a := [1,2]` <br> `print a[1] + a[2]` | `3` — indexing before `+` |
| T-PREC-302 | `var f := func(x) => x` <br> `print f(2) * 3` | `6` — the call before `*` |
| T-PREC-303 | `var t := {x := 2}` <br> `print t.x + 1` | `3` — dot access before `+` |
| T-PREC-304 | `var a := [[1,2]]` <br> `print a[1][2]` | `2` — accessors chain left to right |
| T-PREC-305 | `var a := [ {x := 5} ]` <br> `print a[1].x` | `5` — mixed accessor chain |
| T-PREC-306 | `var f := func => [1,2]` <br> `print f()[2]` | `2` — call then index |
| T-PREC-307 | `var a := [1,2,3]` <br> `print a[1 + 1]` | `2` — the index is a full `Expression` |

## 4xx — Grammar corrections

The three places where the decided grammar differs from the description.

| ID | Program | Expected |
|---|---|---|
| T-PREC-401 | `print 1 2` | syntax error — the `Factor` operator is required; juxtaposition is not an operation ([Q3](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-PREC-402 | `var a := 5` <br> `print -a` | `-5` — `Primary` derives `Reference` ([Q4](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-PREC-403 | `print 1 is int` | `true` — `is` accepts any `Primary` ([Q20](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-PREC-404 | `var a := 1` <br> `print a is int and true` | `true` — `is` sits in `Unary`, so it binds tighter than `and`: `(a is int) and true` |
| T-PREC-405 | `var a := 1` <br> `print a is int + 1` | runtime error — `is` binds tighter than `+`: `(a is int) + 1` is `true + 1` |
| T-PREC-406 | `print ((((1))))` | `1` — deep parenthesis nesting |
| T-PREC-407 | `print (1 + 2` | syntax error — unbalanced parenthesis |
| T-PREC-408 | `var a := 1` <br> `print a + 1 is int` | runtime error — `a + (1 is int)` is `1 + true` |
| T-PREC-409 | `var a := 1` <br> `print (a + 1) is int` | `true` — parenthesise the arithmetic to test its result |
| T-PREC-410 | `var a := 1` <br> `print not a is int` | `false` — a prefix applies to the whole `Unary` after it: `not (a is int)` |
| T-PREC-411 | `print - -1 + 3` | `4` — stacked prefixes, then `+` |
| T-PREC-412 | `print -2 * -3` | `6` — a prefix on each `Term` operand |
| T-PREC-413 | `print [1,2][1]` | syntax error — accessors hang off a `Reference`, not a literal ([Q40](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-PREC-414 | `print (func => 1)()` | syntax error — same for a parenthesised expression |
| T-PREC-415 | `print 1 + +1` | `2` — binary `+` followed by unary `+` |
| T-PREC-416 | `print 1 - - - 1` | `0` — binary `-` then two stacked unary minuses |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-PREC-901 | [Q3](../Language%20D.md#11-spec-ambiguities-and-decisions) — optional operator in `Factor`. | Typo; the operator is required. | `T-PREC-401` |
| T-PREC-902 | [Q4](../Language%20D.md#11-spec-ambiguities-and-decisions) — `Primary` does not derive `Reference`. | It does now; prefixes apply to any `Unary` and stack. | `T-PREC-402`, `T-PREC-410`–`T-PREC-412`, `T-PREC-415`, `T-PREC-416` |
| T-PREC-903 | [Q20](../Language%20D.md#11-spec-ambiguities-and-decisions) — `is` restricted to references. | Any `Primary`. | `T-PREC-403`, `T-PREC-409` |
| T-PREC-904 | [Q30](../Language%20D.md#11-spec-ambiguities-and-decisions) — is `and` really the same level as `or` and `xor`? | Yes, as the grammar states. | `T-PREC-107`, `T-PREC-108` |
| T-PREC-905 | [Q20](../Language%20D.md#11-spec-ambiguities-and-decisions) — the position of `is` relative to the arithmetic levels. | `is` is at the `Unary` level: tighter than every binary operator, inside a prefix. | `T-PREC-404`, `T-PREC-405`, `T-PREC-408`–`T-PREC-410` |
| T-PREC-906 | [Q40](../Language%20D.md#11-spec-ambiguities-and-decisions) — accessors on non-references. | Not derivable; syntax error. | `T-PREC-413`, `T-PREC-414` |
