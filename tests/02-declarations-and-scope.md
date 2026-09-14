# Block 02 — Declarations and Scope

Prefix `T-DECL` · Index: [Tests.md](Tests.md) · Conventions: [IDs](Tests.md#test-ids)
Spec: [§3.1 Variable Declaration](../Language%20D.md#31-variable-declaration), [§3.2 Scope and Visibility](../Language%20D.md#32-scope-and-visibility)

**Program**: [`02-declarations-and-scope`](02-declarations-and-scope) — every runnable test of this block composed into one program, negatives listed commented out at the end.

## Rule under test

```
Declaration
    : var VariableDefinition { , VariableDefinition }
VariableDefinition
    : IDENT [ := Expression ]
```

- A declaration may appear in **any position** within the program.
- Without an initial value the variable has the special value `none`.
- The type of a variable is **not fixed** during declaration.
- Within a scope, **only one** variable with a given name is allowed.
- A variable in a nested scope **hides** the same name from the enclosing scope.

## 0xx — Valid declarations

| ID | Program | Expected |
|---|---|---|
| T-DECL-001 | `var a := 1` <br> `print a` | `1` |
| T-DECL-002 | `var a` <br> `print a is none` | `true` — no initial value means `none` |
| T-DECL-003 | `var a := 1, b := 2` <br> `print a + b` | `3` — two definitions in one declaration |
| T-DECL-004 | `var a := 1, b, c := 3` <br> `print b is none` | `true` — mixed initialised and not |
| T-DECL-005 | `var a := 1 + 2 * 3` <br> `print a` | `7` — initial value is any expression |
| T-DECL-006 | `var a := 1` <br> `var b := a + 1` <br> `print b` | `2` — an earlier variable is visible |
| T-DECL-007 | `print 1` <br> `var a := 2` <br> `print a` | `1` `2` — declaration in any position |
| T-DECL-008 | `var f := func(x) => x + 1` <br> `print f(1)` | `2` — function value as initialiser |

## 1xx — Invalid declarations

| ID | Program | Expected |
|---|---|---|
| T-DECL-101 | `var a := 1` <br> `var a := 2` | runtime error — only one variable with the given name per scope, raised when the second declaration executes ([Q24](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-DECL-102 | `var a := 1, a := 2` | runtime error — duplicate name inside one declaration |
| T-DECL-103 | `print a` <br> `var a := 1` | runtime error — a variable is active only **from** its declaration point |
| T-DECL-104 | `var` | syntax error — at least one `VariableDefinition` is required |
| T-DECL-105 | `var a :=` | syntax error — `:=` requires an expression |
| T-DECL-106 | `var a := 1,` | syntax error — trailing comma |
| T-DECL-107 | `print b` | runtime error — undeclared identifier |
| T-DECL-108 | `var 1a := b` | syntax error — a declared name is an `IDENT` and cannot start with a digit ([Q22](../Language%20D.md#11-spec-ambiguities-and-decisions)); caught by the lexer before `b` is even looked at, so this is a syntax error, not a "`b` is undeclared" runtime error |

## 2xx — Scope, shadowing, dynamic type

| ID | Program | Expected |
|---|---|---|
| T-DECL-201 | `var x := 1` <br> `if true then var x := 2; print x end` <br> `print x` | `2` `1` — the nested `x` hides the outer one |
| T-DECL-202 | `var x := 1` <br> `if true then print x end` | `1` — an enclosing variable is visible in a nested scope |
| T-DECL-203 | `var x := 1` <br> `if true then x := 2 end` <br> `print x` | `2` — assignment (no `var`) reaches the enclosing variable |
| T-DECL-204 | `if true then var y := 1 end` <br> `print y` | runtime error — `y` is dead after its scope ends |
| T-DECL-205 | `var x := 1` <br> `for i in [1] loop var x := 9 end` <br> `print x` | `1` — a loop body is a scope |
| T-DECL-206 | `var x := 1` <br> `var f := func is var x := 9; return x end` <br> `print f(), x` | `9` `1` — a function body is a scope |
| T-DECL-207 | `for i in [1,2] loop var t := i end` <br> `print i` | runtime error — the loop variable's scope is the loop body |
| T-DECL-208 | `var a := 1` <br> `a := "s"` <br> `print a is string` | `true` — the type of a variable is not fixed |
| T-DECL-209 | `var a := 1` <br> `a := 1.5; a := true; a := [1]; a := {x := 1}; a := func => 0` <br> `print a is func` | `true` — a variable can take any type |
| T-DECL-210 | `var x := 1` <br> `if true then var x := x + 1; print x end` | `2` — the initialiser is evaluated before the new name exists, so it sees the outer `x` ([Q25](../Language%20D.md#11-spec-ambiguities-and-decisions)) |
| T-DECL-211 | `var a := [1], b := a` <br> `b[1] := 9` <br> `print a[1]` | `9` — declaring from an array aliases it ([Q26](../Language%20D.md#11-spec-ambiguities-and-decisions)) |

## 9xx — Decided

| ID | Question | Decision | Verified by |
|---|---|---|---|
| T-DECL-901 | [Q1](../Language%20D.md#11-spec-ambiguities-and-decisions) — the description calls the default value both `none` and "empty". | One value, `none`. | `T-DECL-002` |
| T-DECL-902 | [Q24](../Language%20D.md#11-spec-ambiguities-and-decisions) — is redeclaration a syntax error or a runtime error? | Runtime error, raised when the second declaration executes. The parser checks grammar and placement only. | `T-DECL-101`, `T-DECL-102` |
| T-DECL-903 | [Q25](../Language%20D.md#11-spec-ambiguities-and-decisions) — scope of a variable inside its own initialiser. | The initialiser sees the enclosing scope; the name is introduced afterwards. | `T-DECL-210` |
| T-DECL-904 | [Q25](../Language%20D.md#11-spec-ambiguities-and-decisions) — a self-referencing initialiser with no outer name. | `var y := y` is a runtime error: `y` is undeclared while its initialiser runs. | `var y := y` → runtime error |
| T-DECL-905 | [Q22](../Language%20D.md#11-spec-ambiguities-and-decisions) — a declared name that starts with a digit. | Syntax error, caught by the lexer before the statement runs at all — so it wins even over an initialiser that would independently fail (an undeclared `b`). | `T-DECL-108` |
