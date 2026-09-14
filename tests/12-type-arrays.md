# Block 12 — Arrays

Prefix `T-ARR` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§7.5 Array](../Language%20D.md#75-array), [§6 References](../Language%20D.md#6-references-accessors)

**Program**: [`12-type-arrays`](12-type-arrays) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Array
    : [ [ Expression { , Expression } ] ]
Access
    : Reference [ Expression ]      // integer expression, 1-based
```

- An array is a linear composition of values of **any type**.
- The size of the array is **not fixed** and can be dynamically changed.
- **The numbering of the elements begins with one.**
- An array is a **"real" associative array** with keys representing integer values;
  the values of neighbouring keys do not necessarily differ by one.

## 0xx — Literals and access

| ID | Program | Expected |
|---|---|---|
| T-ARR-001 | `var a := [1, 2, 3]` <br> `print a[1]` | `1` — numbering begins with one |
| T-ARR-002 | `var a := [1, 2, 3]` <br> `print a[3]` | `3` |
| T-ARR-003 | `var a := []` <br> `print a is []` | `true` — an empty array literal |
| T-ARR-004 | `var a := [1]` <br> `print a[1]` | `1` — single element |
| T-ARR-005 | `var a := [1, 2.5, "s", true]` <br> `print a[2]` | `2.5` — elements of any type |
| T-ARR-006 | `var a := [[1,2],[3,4]]` <br> `print a[2][1]` | `3` — nested arrays |
| T-ARR-007 | `var a := [{x := 1}]` <br> `print a[1].x` | `1` — a tuple inside an array |
| T-ARR-008 | `var a := [func => 7]` <br> `print a[1]()` | `7` — a function inside an array |
| T-ARR-009 | `var a := [1+1, 2*2]` <br> `print a[1], a[2]` | `2` `4` — elements are expressions |
| T-ARR-010 | `var i := 2` <br> `var a := [10, 20, 30]` <br> `print a[i]` | `20` — the index is any integer expression |
| T-ARR-011 | `var a := [10, 20, 30]` <br> `print a[1 + 1]` | `20` |
| T-ARR-012 | `var a := [1,2] + [3]` <br> `print a[3]` | `3` — concatenation, see [T-ADD-006](04-op-addition.md#0xx--supported-types) |

## 1xx — Invalid access and literals

| ID | Program | Expected |
|---|---|---|
| T-ARR-101 | `var a := [1,2,3]` <br> `print a[0]` | `none` — index 0 holds nothing; a missing key reads as `none`, not an error ([Q8](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ARR-102 | `var a := [1,2,3]` <br> `print a[4]` | `none` — past the last element |
| T-ARR-103 | `var a := [1,2,3]` <br> `print a[-1]` | `none` — a negative key is just another missing key |
| T-ARR-104 | `var a := [1,2,3]` <br> `print a[1.5]` | runtime error — the index must be an integer expression |
| T-ARR-105 | `var a := [1,2,3]` <br> `print a["1"]` | runtime error — the index must be an integer expression |
| T-ARR-106 | `var a := [1,2,3]` <br> `print a[true]` | runtime error |
| T-ARR-107 | `var a := 1` <br> `print a[1]` | runtime error — indexing a non-array |
| T-ARR-108 | `var a := "abc"` <br> `print a[1]` | runtime error — strings are not indexable |
| T-ARR-109 | `var a := {x := 1}` <br> `print a[1]` | runtime error — a tuple is accessed with `.`, not `[ ]` |
| T-ARR-110 | `var a := [1, 2,]` | syntax error — trailing comma |
| T-ARR-111 | `var a := [1 2]` | syntax error — missing comma |
| T-ARR-112 | `var a := [1,2]` <br> `print a[]` | syntax error — an index expression is required |

## 2xx — Dynamic size and associative keys

An array is a real associative array; neighbouring keys need not differ by one.

| ID | Program | Expected |
|---|---|---|
| T-ARR-201 | `var t := []` <br> `t[10] := 25` <br> `print t[10]` | `25` — the spec's own example |
| T-ARR-202 | `var t := []` <br> `t[100] := func(x) => x + 1` <br> `print t[100](1)` | `2` — a function at a sparse key |
| T-ARR-203 | `var t := []` <br> `t[1000] := {a := 1, b := 2.7}` <br> `print t[1000].b` | `2.7` — a tuple at a sparse key |
| T-ARR-204 | `var t := []` <br> `t[10] := 25; t[100] := 1; t[1000] := 2` <br> `print t[10], t[100], t[1000]` | `25` `1` `2` — the full spec example |
| T-ARR-205 | `var a := [1,2,3]` <br> `a[2] := 9` <br> `print a[2]` | `9` — an existing element is replaced |
| T-ARR-206 | `var a := [1,2,3]` <br> `a[4] := 4` <br> `print a[4]` | `4` — the array grows |
| T-ARR-207 | `var a := [1]` <br> `a[1] := "s"` <br> `print a[1] is string` | `true` — an element may change type |
| T-ARR-208 | `var a := []` <br> `a[2] := 2` <br> `print a[1]` | `none` — a gap left by a sparse write ([Q8](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ARR-209 | `var a := [1,2,3]` <br> `print a[4] is none` | `true` — the presence test |
| T-ARR-210 | `var a := [1]` <br> `print a[5] is none` <br> `print a` | `true` then `[1]` — reading a missing key does not create it |
| T-ARR-211 | `var t := []` <br> `t[10] := 1; t[3] := 2` <br> `print t` | `[2, 1]` — printed in ascending key order, gaps not shown ([Q32](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ARR-212 | `var t := []` <br> `t[10] := 1; t[3] := 2` <br> `for v in t loop print v end` | `2` `1` — iterated in ascending key order |
| T-ARR-213 | `var a := [1,2,3]` <br> `print a[1] + a[4]` | runtime error — the missing key reads as `none`, which cannot be an operand |

## 3xx — Arrays in other constructs

| ID | Program | Expected |
|---|---|---|
| T-ARR-301 | `var array := [1,2,3,4,5]` <br> `var sum := 0` <br> `for i in array loop sum := sum + i end` <br> `print sum` | `15` — the spec's own iteration example |
| T-ARR-302 | `var a := [1]` <br> `print a is []` | `true` — the `[ ]` type indicator |
| T-ARR-303 | `var f := func(x) => x[1]` <br> `print f([7])` | `7` — an array passed as an argument |
| T-ARR-304 | `var f := func => [1,2]` <br> `print f()[2]` | `2` — an array returned from a function |
| T-ARR-305 | `for i in [] loop print "x" end` | no output — iterating an empty array |

## 4xx — Edge cases

| ID | Program | Expected |
|---|---|---|
| T-ARR-401 | `var a := []` <br> `print a + []` | `[]` |
| T-ARR-402 | `var a := [[[1]]]` <br> `print a[1][1][1]` | `1` — deep nesting |
| T-ARR-403 | `var a := [1], b := a` <br> `b[1] := 9` <br> `print a[1]` | `9` — arrays alias on assignment ([Q26](../Language%20D.md#11-spec-ambiguities-and-decisions), [T-ASGN-203](03-assignment.md#2xx--semantics)) |
| T-ARR-404 | `var a := [1,2]` <br> `print a[1] + a[2]` | `3` — elements are ordinary operands |
| T-ARR-405 | `var a := []` <br> `print a` | `[]` — rendering of an empty array ([Q15](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-ARR-406 | `print [1, "a", true, 1.5, [2], {x := 3}]` | `[1, a, true, 1.5, [2], {x := 3}]` — one rendering rule at every depth |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-ARR-901 | [Q8](../Language%20D.md#11-spec-ambiguities-and-decisions) — reading a key that was never assigned: runtime error or `none`? | `none`; reading does not create the key. | `T-ARR-101`–`T-ARR-103`, `T-ARR-208`–`T-ARR-210`, `T-ARR-213` |
| T-ARR-902 | [Q9](../Language%20D.md#11-spec-ambiguities-and-decisions) — key handling on concatenation of sparse arrays. | Renumbered from 1. | [T-ADD-405](04-op-addition.md#4xx--edge-cases), [T-ADD-409](04-op-addition.md#4xx--edge-cases) |
| T-ARR-903 | Is there a way to query the size of an array, or to delete a key? | No — neither exists; count with a loop, and `a[k] is none` tests presence. Not a decision, just the answer. | — |
| T-ARR-904 | [Q32](../Language%20D.md#11-spec-ambiguities-and-decisions) — iteration order over sparse keys. | Ascending key order, gaps skipped; the same order for `print` and `+`. | `T-ARR-211`, `T-ARR-212` |
