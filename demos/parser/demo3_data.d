// Parser demo 3 — arrays, tuples, functions, references
var people := [
    {name := "Ann", age := 23},
    {name := "Bob", age := 21}
]
var older := func(p, years) => p.age + years
print older(people[1], 5), people[2].name is string
