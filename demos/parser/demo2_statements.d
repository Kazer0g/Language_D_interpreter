// Parser demo 2 — statements, loops, short if
var sum := 0
for i in 1..10 loop
    if i = 5 => exit
    sum := sum + i
end
print sum
