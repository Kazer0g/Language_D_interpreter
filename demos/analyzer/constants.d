// ./main --before-after demos/analyzer/constants.d
var x := 2 + 3 * 4
print x, 3 < 5, -7 / 2, 1 + 2.5, "hello " + "world", not false

// These expressions must stay in the AST: errors belong to runtime.
print 1 / 0, 9223372036854775807 + 1
// Both operands are evaluated in D, so the call must remain.
print true or f()
