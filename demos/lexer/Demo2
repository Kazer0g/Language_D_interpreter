// Demo 2 — lookahead: range vs real literal
// same starting characters '1', '.', but different token result

print 1..3        // T_INT, T_RANGE, T_INT — lookahead: peekNext() is '.'
print 1.3         // T_REAL 1.3           — lookahead: peekNext() is a digit

for i in 1..3 loop
    print i
end