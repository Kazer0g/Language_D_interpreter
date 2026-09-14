# Block 15 — Conditionals `if` and `if =>`

Prefix `T-IF` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§4.2 If](../Language%20D.md#42-if), [§4.3 IfShort](../Language%20D.md#43-ifshort), [§4.4 Body](../Language%20D.md#44-body)

**Program**: [`15-stmt-conditionals`](15-stmt-conditionals) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
If
    : if Expression then Body [ else Body ] end
IfShort
    : if Expression => Body
Body
    : Statement { [ ; ] Statement }
```

The condition is an `Expression` and **must evaluate to a boolean**
([Q34](../Language%20D.md#11-spec-ambiguities-and-decisions)) — there is no truthiness
rule. A `Body` holds at least one statement
([Q35](../Language%20D.md#11-spec-ambiguities-and-decisions)). The short form's body
extends to the end of the line or to the first `end`/`else` on it
([Q36](../Language%20D.md#11-spec-ambiguities-and-decisions)).

## 0xx — Full form

| ID | Program | Expected |
|---|---|---|
| T-IF-001 | `if true then print "y" end` | `y` |
| T-IF-002 | `if false then print "y" end` | no output |
| T-IF-003 | `if true then print "y" else print "n" end` | `y` |
| T-IF-004 | `if false then print "y" else print "n" end` | `n` |
| T-IF-005 | `var n := 5` <br> `if n > 0 then print "positive" else print "non-positive" end` | `positive` |
| T-IF-006 | `if 1 < 2 and 3 < 4 then print "y" end` | `y` — the condition is any expression |
| T-IF-007 | `var b := true` <br> `if b then print "y" end` | `y` — a variable as the condition |
| T-IF-008 | `if true then var a := 1; print a end` | `1` — several statements in the body |
| T-IF-009 | `if true then` <br> `  print 1` <br> `  print 2` <br> `end` | `1` `2` — newline-separated body |

## 1xx — Invalid conditions and forms

| ID | Program | Expected |
|---|---|---|
| T-IF-101 | `if 1 then print "y" end` | runtime error — a non-boolean condition; no truthiness rule ([Q34](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-IF-102 | `if 0 then print "y" end` | runtime error — `0` is not `false` |
| T-IF-103 | `if "s" then print "y" end` | runtime error |
| T-IF-104 | `var n` <br> `if n then print "y" end` | runtime error — `none` cannot act as an operand |
| T-IF-105 | `if true print "y" end` | syntax error — `then` is required |
| T-IF-106 | `if true then print "y"` | syntax error — `end` is required |
| T-IF-107 | `if then print "y" end` | syntax error — a condition is required |
| T-IF-108 | `if true then else print "n" end` | syntax error — a `Body` requires at least one `Statement` ([Q35](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-IF-109 | `if true then print 1 else end` | syntax error — same for the else branch |
| T-IF-110 | `if true then print 1 else print 2 else print 3 end` | syntax error — at most one `else` |
| T-IF-111 | `if [true] then print "y" end` | runtime error — an array is not a boolean |
| T-IF-112 | `if false then print 1 / 0 end` | no output — the branch not taken is not evaluated |

## 2xx — Short form `if =>`

The short form has **no `else` and no `end`**.

| ID | Program | Expected |
|---|---|---|
| T-IF-201 | `var i := 100` <br> `if i = 100 => print "y"` | `y` |
| T-IF-202 | `if false => print "y"` | no output |
| T-IF-203 | `var i := 0` <br> `loop i := i + 1; if i = 100 => exit end` <br> `print i` | `100` — the spec's own example |
| T-IF-204 | `if true => print "y" else print "n"` | syntax error — the short form has no `else` |
| T-IF-205 | `if true => print "y" end` | syntax error — the short form has no `end` |
| T-IF-206 | `if true => var a := 1; print a` | `1` — the short-form body runs to the end of the line, semicolons separate its statements ([Q36](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-IF-207 | `if true => print 1` <br> `print 2` | `1` `2` — a newline ends the short-form body |
| T-IF-208 | `if false => print 1; print 2` | no output — **both** statements belong to the body |
| T-IF-209 | `if false => print 1; print 2` <br> `print 3` | `3` — only the next line is outside the body |
| T-IF-210 | `if true then if true => print 1 end` <br> `print 2` | `1` `2` — `end` on the same line closes the outer `if`, not the short body |
| T-IF-211 | `if true then if false => print 1 else print 2 end` | `2` — `else` on the same line ends the short body and belongs to the outer `if` |
| T-IF-212 | `if true => if true => print 1` | `1` — nested short forms on one line |

## 3xx — Nesting and structure

| ID | Program | Expected |
|---|---|---|
| T-IF-301 | `if true then if true then print "y" end end` | `y` — nested full form |
| T-IF-302 | `if true then if false then print "y" else print "n" end end` | `n` — the inner `else` binds to the inner `if` |
| T-IF-303 | `if false then print 1 else if true then print 2 end end` | `2` — else-if by nesting |
| T-IF-304 | `if true then if true => print "y" end` | `y` — short form nested inside the full form |
| T-IF-305 | `if true then var x := 1 end` <br> `print x` | runtime error — a branch body is a scope, see [T-DECL-204](02-declarations-and-scope.md#2xx--scope-shadowing-dynamic-type) |
| T-IF-306 | `var x := 1` <br> `if true then x := 2 end` <br> `print x` | `2` — assignment reaches the enclosing variable |
| T-IF-307 | `for i in [1,2] loop if i = 2 => exit end` <br> `print "done"` | `done` — `exit` inside a branch inside a loop |
| T-IF-308 | `if true then` <br> `  print 1;` <br> `else` <br> `  print 2` <br> `end` | `1` — a trailing separator before `else` |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-IF-901 | [Q34](../Language%20D.md#11-spec-ambiguities-and-decisions) — is a non-boolean condition a runtime error, or is there truthiness? | Runtime error. | `T-IF-101`–`T-IF-104`, `T-IF-111` |
| T-IF-902 | [Q35](../Language%20D.md#11-spec-ambiguities-and-decisions) — may a `Body` be empty? | No; syntax error. | `T-IF-108`, `T-IF-109` |
| T-IF-903 | [Q36](../Language%20D.md#11-spec-ambiguities-and-decisions) — the extent of an `IfShort` body. | To the end of the line, or the first `end`/`else` on it. | `T-IF-206`–`T-IF-212` |
