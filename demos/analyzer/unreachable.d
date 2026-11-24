// ./main --before-after demos/analyzer/unreachable.d
var f := func is
    if true then
        return 2 + 3
        print "unreachable branch tail"
    end
    print "keep: the preceding return is conditional"
    return 7
    print "unreachable function tail"
end

loop
    exit
    print "unreachable loop tail"
end
print "keep: execution continues after the loop"
