// A function cannot exit a loop that surrounds its literal.
loop
    var f := func is
        exit
    end
    exit
end
