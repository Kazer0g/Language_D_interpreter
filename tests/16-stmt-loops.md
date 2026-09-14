# Block 16 — Loops and `exit`

Prefix `T-LOOP` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§4.5 Loop](../Language%20D.md#45-loop), [§4.6 Exit](../Language%20D.md#46-exit)

**Program**: [`16-stmt-loops`](16-stmt-loops) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Loop
    : WhileLoop | ForLoop
WhileLoop
    : while Expression LoopBody
ForLoop
    : [ ForHeader ] LoopBody
ForHeader
    : for [ IDENT in ] Expression [ .. Expression ]
LoopBody
    : loop Body end
Exit
    : exit
```

- The `ForHeader` may be **omitted** — the loop is then **infinite** and only `exit`
  leaves it.
- **Two expressions**: iterate the range from `v1` until `v2`, the current **integer**
  values of those expressions.
- **An identifier** in the header introduces a variable taking the range values
  sequentially; **its scope is the loop body**.
- **A single expression**: its type must be an **array** or a **tuple**, and the loop
  iterates over all its elements.
- `exit` can appear **only within loop statements**.

## 0xx — While loop

| ID | Program | Expected |
|---|---|---|
| T-LOOP-001 | `var i := 0` <br> `while i < 3 loop i := i + 1 end` <br> `print i` | `3` |
| T-LOOP-002 | `while false loop print "x" end` | no output — the condition is checked first |
| T-LOOP-003 | `var i := 3` <br> `while i > 0 loop print i; i := i - 1 end` | `3` `2` `1` |
| T-LOOP-004 | `var i := 0` <br> `while i < 2 and true loop i := i + 1 end` <br> `print i` | `2` — the condition is any expression |
| T-LOOP-005 | `while 1 loop exit end` | runtime error — non-boolean condition, as for [T-IF-101](15-stmt-conditionals.md#1xx--invalid-conditions-and-forms) ([Q34](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOOP-006 | `while true loop end` | syntax error — a `Body` requires at least one `Statement` |
| T-LOOP-007 | `while i < 3 i := i + 1 end` | syntax error — `loop` is required |

## 1xx — For loop over a range

| ID | Program | Expected |
|---|---|---|
| T-LOOP-101 | `for 1..3 loop print "Hello" end` | `Hello` ×3 — the spec's own example, no loop variable |
| T-LOOP-102 | `for i in 1..3 loop print i end` | `1` `2` `3` — with a loop variable |
| T-LOOP-103 | `var s := 0` <br> `for i in 1..5 loop s := s + i end` <br> `print s` | `15` |
| T-LOOP-104 | `var a := 1, b := 3` <br> `for i in a..b loop print i end` | `1` `2` `3` — the bounds are expressions |
| T-LOOP-105 | `for i in 1..1 loop print i end` | `1` — a single-value range |
| T-LOOP-106 | `for i in 3..1 loop print i end` | no output — a descending range runs zero times ([Q37](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOOP-107 | `for i in 1..3 loop print i end` <br> `print i` | runtime error — the loop variable's scope is the loop body |
| T-LOOP-108 | `for i in 1..2 loop i := 9; print i end` | `9` `9` — the loop variable may be assigned; the next iteration overwrites it ([Q38](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOOP-109 | `for i in 1.5..3.5 loop print i end` | runtime error — the bounds must be **integer** values ([Q12](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOOP-110 | `for i in "a".."c" loop print i end` | runtime error — the bounds must be integer |
| T-LOOP-111 | `for i in 1..3 loop` <br> `  print i` <br> `end` | `1` `2` `3` — multi-line body |
| T-LOOP-112 | `var n := 3, c := 0` <br> `for i in 1..n loop n := 10; c := c + 1 end` <br> `print c` | `3` — the bounds are evaluated once, before the first iteration |
| T-LOOP-113 | `for i in 0..0 loop print i end` | `0` — inclusive at both ends ([Q39](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOOP-114 | `for i in -2..0 loop print i end` | `-2` `-1` `0` — negative bounds |
| T-LOOP-115 | `for i in 1..3 loop i := i + 10; print i end` | `11` `12` `13` — assignment does not affect the sequence |
| T-LOOP-116 | `for i in 1..2.0 loop print i end` | runtime error — one real bound is enough to fail |
| T-LOOP-117 | `for 1i in 1..3 loop print 1i end` | syntax error — the loop variable is an `IDENT`, and `1i` lexes as `INTEGER` then `IDENT`, not one identifier ([Q22](../Language%20D.md#11-spec-ambiguities-and-decisions)); same rule as `T-LEX-405`, `T-TUP-113`, `T-FUN-104` |

## 2xx — For loop over an aggregate

A single header expression must be an array or a tuple.

| ID | Program | Expected |
|---|---|---|
| T-LOOP-201 | `var array := [1,2,3,4,5]` <br> `var sum := 0` <br> `for i in array loop sum := sum + i end` <br> `print sum` | `15` — the spec's own example |
| T-LOOP-202 | `for v in [10, 20] loop print v end` | `10` `20` — an array literal in the header |
| T-LOOP-203 | `for v in {a := 1, b := 2} loop print v end` | `1` `2` — a tuple is also iterable |
| T-LOOP-204 | `for v in [1, "s", true] loop print v end` | `1` `s` `true` — elements of any type |
| T-LOOP-205 | `for v in [] loop print v end` | no output — an empty array |
| T-LOOP-206 | `for [1,2] loop print "x" end` | `x` `x` — an aggregate without a loop variable |
| T-LOOP-207 | `for v in 5 loop print v end` | runtime error — the type must be array or tuple |
| T-LOOP-208 | `for v in "abc" loop print v end` | runtime error — a string is not an iterable aggregate |
| T-LOOP-209 | `var n` <br> `for v in n loop print v end` | runtime error — `none` cannot act as an operand |
| T-LOOP-210 | `var t := []` <br> `t[100] := 2; t[10] := 1` <br> `for v in t loop print v end` | `1` `2` — ascending key order, not insertion order ([Q32](../Language%20D.md#11-spec-ambiguities-and-decisions), [T-ARR-904](12-type-arrays.md#9xx--decided)) |
| T-LOOP-211 | `for v in {a := 1} loop print v end` | `1` — the loop variable takes the **value**; names are not reachable ([Q13](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOOP-212 | `var a := [1, 2]` <br> `for v in a loop v := 0 end` <br> `print a` | `[1, 2]` — the loop variable is a copy of the element, not an alias of the slot |
| T-LOOP-213 | `var a := [1]` <br> `for v in a loop a[2] := 2 end` <br> `print a` | `[1, 2]` — the body may modify the array; the iteration visits the elements present at loop entry (one pass) |

## 3xx — Infinite loop, header omitted

| ID | Program | Expected |
|---|---|---|
| T-LOOP-301 | `var i := 0` <br> `loop print "Hello"; i := i + 1; if i = 100 => exit end` <br> `print i` | `Hello` ×100 then `100` — the spec's own example |
| T-LOOP-302 | `var i := 0` <br> `loop i := i + 1; if i = 1 => exit end` <br> `print i` | `1` — the header is optional |
| T-LOOP-303 | `loop print "x" end` | does not terminate — no `exit`, an accepted program that hangs |

## 4xx — `exit`

`exit` can appear **only within loop statements**.

| ID | Program | Expected |
|---|---|---|
| T-LOOP-401 | `var i := 0` <br> `loop i := i + 1; if i = 10 => exit end` <br> `print i` | `10` |
| T-LOOP-402 | `while true loop exit end` <br> `print "done"` | `done` — `exit` leaves a while loop |
| T-LOOP-403 | `for i in 1..10 loop if i = 3 => exit end` <br> `print "done"` | `done` — `exit` leaves a for loop |
| T-LOOP-404 | `for i in 1..3 loop print i; exit end` | `1` — `exit` on the first iteration |
| T-LOOP-405 | `exit` | syntax error — `exit` outside a loop, rejected by the parser ([Q24](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-LOOP-406 | `if true then exit end` | syntax error — `exit` outside a loop |
| T-LOOP-407 | `var f := func is exit end` <br> `f()` | syntax error — a function body is not a loop |
| T-LOOP-411 | `loop var f := func is exit end; exit end` | syntax error — the function body starts a fresh context even inside a loop |
| T-LOOP-412 | `print 1` <br> `exit` | syntax error, **no output** — the check happens before execution |
| T-LOOP-408 | `loop loop exit end print "inner done"; exit end` <br> `print "done"` | `inner done` `done` — `exit` leaves the **innermost** loop |
| T-LOOP-409 | `for i in 1..3 loop var f := func => 1; exit end` <br> `print "done"` | `done` — `exit` inside a loop that also declares a function |
| T-LOOP-410 | `loop print "a"; exit; print "b" end` | `a` — statements after `exit` do not run |

## 5xx — Nesting and scope

| ID | Program | Expected |
|---|---|---|
| T-LOOP-501 | `var s := 0` <br> `for i in 1..2 loop for j in 1..2 loop s := s + 1 end end` <br> `print s` | `4` — nested for loops |
| T-LOOP-502 | `var x := 1` <br> `for i in 1..1 loop var x := 9 end` <br> `print x` | `1` — the loop body is a scope |
| T-LOOP-503 | `var s := 0` <br> `for i in [1,2] loop for j in [10,20] loop s := s + j end end` <br> `print s` | `60` — nested aggregate loops |
| T-LOOP-504 | `for i in 1..2 loop for i in 1..2 loop print i end end` | `1` `2` `1` `2` — the inner `i` hides the outer one |
| T-LOOP-505 | `var i := 5` <br> `for i in 1..2 loop print i end` <br> `print i` | `1` `2` `5` — the loop variable hides an outer variable of the same name |
| T-LOOP-506 | `var s := 0` <br> `for i in 1..3 loop if i = 2 => s := s + 10` <br> `end` <br> `print s` | `10` — a short `if` inside a loop body |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-LOOP-901 | [Q12](../Language%20D.md#11-spec-ambiguities-and-decisions) — real-valued range bounds. | Runtime error. | `T-LOOP-109`, `T-LOOP-116` |
| T-LOOP-902 | [Q37](../Language%20D.md#11-spec-ambiguities-and-decisions) — a descending range: empty loop, or does it count down? | Zero iterations. | `T-LOOP-106` |
| T-LOOP-903 | [Q38](../Language%20D.md#11-spec-ambiguities-and-decisions) — assigning to the loop variable inside the body. | Allowed; overwritten on the next iteration. | `T-LOOP-108`, `T-LOOP-115` |
| T-LOOP-904 | [Q13](../Language%20D.md#11-spec-ambiguities-and-decisions) — element names when iterating a tuple. | Values only. | `T-LOOP-211`, [T-TUP-306](13-type-tuples.md#3xx--tuples-in-other-constructs) |
| T-LOOP-905 | [Q24](../Language%20D.md#11-spec-ambiguities-and-decisions) — is `exit` outside a loop a syntax error or a runtime error? | Syntax error, before execution. | `T-LOOP-405`–`T-LOOP-407`, `T-LOOP-411`, `T-LOOP-412` |
| T-LOOP-906 | [Q39](../Language%20D.md#11-spec-ambiguities-and-decisions) — are the range bounds inclusive on both ends? | Yes. | `T-LOOP-101`, `T-LOOP-105`, `T-LOOP-113` |
| T-LOOP-907 | [Q32](../Language%20D.md#11-spec-ambiguities-and-decisions) — iteration order over sparse keys, and writes during iteration. | Ascending key order; the keys to visit are fixed at loop entry. | `T-LOOP-210`, `T-LOOP-212`, `T-LOOP-213` |
| T-LOOP-908 | [Q22](../Language%20D.md#11-spec-ambiguities-and-decisions) — can the `for` loop variable start with a digit? | No — `ForHeader : for [ IDENT in ] …` uses the same `IDENT` as every other declared name; `1i` is two tokens, not one. Syntax error. | `T-LOOP-117` |
