# Block 17 — Print

Prefix `T-PRN` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§4.8 Print](../Language%20D.md#48-print)

**Program**: [`17-stmt-print`](17-stmt-print) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Print
    : print Expression { , Expression }
```

`print` is the language's only output statement. The description does not fix the
**format** of the output; it is decided by
[Q15](../Language%20D.md#11-spec-ambiguities-and-decisions) and written out in
[§4.8](../Language%20D.md#48-print): values separated by one space, one newline per
`print`, reals always with a `.`, strings without quotes, `[1, 2]`, `{a := 1, 2}`,
`func`, `none`. Section `4xx` verifies each rule of that format.

## 0xx — Single expression

| ID | Program | Expected |
|---|---|---|
| T-PRN-001 | `print 1` | `1` |
| T-PRN-002 | `print "Hello"` | `Hello` — without the quotes |
| T-PRN-003 | `print 'Hello'` | `Hello` — quote style does not affect the output |
| T-PRN-004 | `print true` | `true` |
| T-PRN-005 | `print 1.5` | `1.5` |
| T-PRN-006 | `print 1 + 2` | `3` — the argument is an expression |
| T-PRN-007 | `var a := 5` <br> `print a` | `5` — the argument is a reference |
| T-PRN-008 | `var f := func => 7` <br> `print f()` | `7` — the argument is a call |
| T-PRN-009 | `print 1 < 2` | `true` — a boolean result |
| T-PRN-010 | `var a := [1]` <br> `print a[1]` | `1` — an array element |
| T-PRN-011 | `var t := {x := 2}` <br> `print t.x` | `2` — a tuple element |

## 1xx — Several expressions and invalid forms

| ID | Program | Expected |
|---|---|---|
| T-PRN-101 | `print 1, 2, 3` | `1 2 3` — three values on one line, one space between |
| T-PRN-102 | `print "sum = ", 1 + 2` | `sum =  3` — the string's own trailing space plus the separator space |
| T-PRN-103 | `var a := 1, b := 2` <br> `print a, b` | `1 2` |
| T-PRN-104 | `print 1, "s", true, 1.5` | `1 s true 1.5` — values of different types |
| T-PRN-105 | `print` | syntax error — at least one expression is required |
| T-PRN-106 | `print 1,` | syntax error — trailing comma |
| T-PRN-107 | `print 1 2` | syntax error — missing comma |
| T-PRN-108 | `print ,1` | syntax error |
| T-PRN-109 | `var n` <br> `print n` | `none` — printing `none` is allowed ([Q31](../Language%20D.md#11-spec-ambiguities-and-decisions), [T-IS-410](11-op-type-check-is.md#4xx--none-handling)) |

## 2xx — Sequencing

| ID | Program | Expected |
|---|---|---|
| T-PRN-201 | `print 1` <br> `print 2` | `1` `2` on separate lines — each statement ends its own output |
| T-PRN-202 | `print 1; print 2` | `1` `2` on separate lines — separator style does not change the output |
| T-PRN-203 | `for i in 1..3 loop print i end` | `1` `2` `3` — one line per iteration |
| T-PRN-204 | `var f := func is print "in"; return 1 end` <br> `print f()` | `in` then `1` — the argument is evaluated first |
| T-PRN-205 | `print 1, 2` | `1 2` — one line; the comma list never breaks the line |
| T-PRN-206 | `print 1, 1 / 0` | runtime error, **nothing printed** — all arguments are evaluated before anything is written |

## 4xx — Output format ([Q15](../Language%20D.md#11-spec-ambiguities-and-decisions))

| ID | Program | Expected | Rule |
|---|---|---|---|
| T-PRN-401 | `print 1, 2` | `1 2` | one space between values |
| T-PRN-402 | `print 3.0` | `3.0` | a real always carries a `.` |
| T-PRN-403 | `print 1.0 / 3.0` | `0.3333333333333333` | shortest digit string that round-trips the double |
| T-PRN-404 | `print [1, 2]` | `[1, 2]` | array: `[`, `, `-separated, `]` |
| T-PRN-405 | `print {a := 1}` | `{a := 1}` | tuple: named element as `name := value` |
| T-PRN-406 | `print func => 0` | `func` | a function value is the word `func` |
| T-PRN-407 | `var n` <br> `print n` | `none` | `none` is the word `none` |
| T-PRN-408 | `print ""` | *(empty line)* | the newline is still written |
| T-PRN-409 | `var t := []` <br> `t[10] := 1` <br> `print t` | `[1]` | ascending key order, gaps not shown |
| T-PRN-410 | `print [[1],[2]]` | `[[1], [2]]` | nested arrays use the same rule |
| T-PRN-411 | `print ["a", 'b']` | `[a, b]` | strings inside composites have no quotes either |
| T-PRN-412 | `print {1, b := 2.5, "s", [true]}` | `{1, b := 2.5, s, [true]}` | unnamed elements as the bare value |
| T-PRN-413 | `print []` | `[]` | the empty array |
| T-PRN-414 | `print -0.5, 100.0, 0.1` | `-0.5 100.0 0.1` | sign, whole real, short fraction |
| T-PRN-415 | `print 1000000.0 * 1000000.0` | `1000000000000.0` | no exponent form |
| T-PRN-416 | `print [none]` | `[none]` | `none` inside a composite |
| T-PRN-417 | `print {f := func => 0}` | `{f := func}` | a function inside a composite |
| T-PRN-418 | `print "a", "", "b"` | `a  b` | an empty string still gets its separators |
| T-PRN-419 | `print 2.0 * 3` | `6.0` | the type decides the rendering, not the value |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-PRN-901 | [Q15](../Language%20D.md#11-spec-ambiguities-and-decisions) — the output format. | As in [§4.8](../Language%20D.md#48-print). | `T-PRN-401`–`T-PRN-419` |
| T-PRN-902 | Is there any output statement that does not append a newline? | No — `print` is the only output statement and it always ends the line. | `T-PRN-201`, `T-PRN-408` |
| T-PRN-903 | [Q28](../Language%20D.md#11-spec-ambiguities-and-decisions) — how many digits a real prints. | As many as needed to read back the same double, no more. | `T-PRN-403`, `T-PRN-414` |
