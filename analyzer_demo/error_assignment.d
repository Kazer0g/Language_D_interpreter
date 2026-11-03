// A call may appear inside a target, but cannot be its final accessor.
var f := func => [1]
f()[1] := 5
f() := 5
