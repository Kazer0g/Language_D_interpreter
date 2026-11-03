// Standalone regression tests; build this file, not main.cpp, as the entry point.
#include <functional>
#include "../lexer.cpp"
#include "../parser.cpp"
#include "../analyzer.cpp"

struct Program {
    vector<Stmt*> body;

    explicit Program(const string& source) {
        Lexer lexer(source);
        Parser parser(lexer.run());
        body = parser.parseProgram();
    }
    ~Program() { destroyProgram(body); }
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
};

void require(bool condition, const string& message) {
    if (!condition) throw runtime_error(message);
}

string tree(const Program& program) {
    ostringstream out;
    streambuf* previous = cout.rdbuf(out.rdbuf());
    printTree({"Program", stmtTrees(program.body)});
    cout.rdbuf(previous);
    return out.str();
}

void semanticError(const string& source, const string& message) {
    Program program(source); // The parser must accept these contextual violations.
    string before = tree(program);
    Analyzer analyzer;
    try {
        analyzer.run(program.body);
    } catch (const runtime_error& error) {
        string diagnostic = error.what();
        require(diagnostic.find("Semantic error at ") == 0, diagnostic);
        require(diagnostic.find(message) != string::npos, diagnostic);
        require(tree(program) == before, "a failed validation changed the AST");
        return;
    }
    throw runtime_error("expected semantic error: " + source);
}

void integerResult(const string& expression, long long expected) {
    Program program("print " + expression);
    Analyzer analyzer;
    analyzer.run(program.body);
    const Expr* result = program.body[0]->values[0];
    require(result->kind == E_INT && result->intValue == expected, expression);
}

void staysBinary(const string& expression) {
    Program program("print " + expression);
    Analyzer analyzer;
    analyzer.run(program.body);
    require(program.body[0]->values[0]->kind == E_BINARY, "must retain: " + expression);
}

int main() {
    size_t passed = 0;
    auto test = [&](const string& name, const function<void()>& run) {
        try {
            run();
            ++passed;
            cout << "PASS " << name << "\n";
        } catch (const exception& error) {
            cerr << "FAIL " << name << ": " << error.what() << "\n";
            throw;
        }
    };

    try {
        test("validation does not change nodes or counters", [] {
            Program p("var f := func(x) is loop exit end; return x + 1 end\nprint f(2 + 3)");
            string before = tree(p);
            Stmt* statement = p.body[0];
            Expr* expression = p.body[1]->values[0]->items[0];
            Analyzer a;
            a.checkProgram(p.body);
            require(tree(p) == before && statement == p.body[0] &&
                    expression == p.body[1]->values[0]->items[0], "validation mutated AST");
            require(a.foldedExpressions == 0 && a.removedStatements == 0, "validation optimized");
            a.run(p.body, false);
            require(tree(p) == before, "disabled optimization changed AST");
        });

        test("keyword context and source positions", [] {
            semanticError("exit", "1:1: 'exit'");
            semanticError("print 1\nreturn 2", "2:1: 'return'");
            semanticError("if true then exit end", "'exit'");
            semanticError("loop return 1 end", "'return'");
            semanticError("loop var f := func is exit end; exit end", "'exit'");
            semanticError("var a := [func is exit end]", "'exit'");
            semanticError("print g(func is exit end)", "'exit'");
            semanticError("while func is exit end loop exit end", "'exit'");
            semanticError("for func is exit end loop exit end", "'exit'");
            semanticError("a[func is exit end] := 1", "'exit'");
        });

        test("nested functions have independent loop context", [] {
            Program p("loop\nvar f := func is loop exit end; return 1 end\nexit\nend\n"
                      "var g := func is while true loop return 2 end; return 3 end");
            Analyzer a;
            a.run(p.body);
            semanticError("var f := func is loop var g := func is exit end; exit end end", "'exit'");
        });

        test("assignment checks the final accessor", [] {
            semanticError("f() := 1", "assignment target");
            semanticError("a[1].f() := 1", "assignment target");
            Program p("x := 1\na[1] := 2\nt.a := 3\nt.1 := 4\nf()[1] := 5\nf().a := 6");
            Analyzer a;
            a.run(p.body);
        });

        test("all checks run before unreachable-code elimination", [] {
            semanticError("var f := func is return 1; exit end", "'exit'");
            semanticError("var f := func is return 1; g() := 2 end", "assignment target");
        });

        test("integer arithmetic, precedence and floor division", [] {
            integerResult("2 + 3 * 4", 14);
            integerResult("(2 + 3) * 4", 20);
            integerResult("-7 / 2", -4);
            integerResult("7 / -2", -4);
            integerResult("-7 / -2", 3);
            integerResult("-8 / 2", -4);
            integerResult("1 / -9223372036854775807", -1);
            integerResult("0 * -9223372036854775807", 0);
            integerResult("-3 * -4", 12);
            integerResult("(-9223372036854775807 - 1) / 1", numeric_limits<long long>::min());
            integerResult("9223372036854775806 + 1", numeric_limits<long long>::max());
            integerResult("-9223372036854775807 - 1", numeric_limits<long long>::min());
        });

        test("overflow and division by zero remain runtime operations", [] {
            staysBinary("9223372036854775807 + 1");
            staysBinary("(-9223372036854775807 - 1) - 1");
            staysBinary("9223372036854775807 * 2");
            staysBinary("-9223372036854775807 * 2");
            staysBinary("-9223372036854775807 * -2");
            staysBinary("(-9223372036854775807 - 1) * -1");
            staysBinary("(-9223372036854775807 - 1) / -1");
            staysBinary("1 / 0");
            staysBinary("1.0 / 0.0");
            Program p("print -(-9223372036854775807 - 1)");
            Analyzer a;
            a.run(p.body);
            require(p.body[0]->values[0]->kind == E_UNARY, "negation overflow was folded");
        });

        test("mixed real arithmetic and exact integer comparisons", [] {
            Program p("print 1 + 2.5, 7 / 2.0, -2.5, 9007199254740993 = 9007199254740992, 3 < 5");
            Analyzer a;
            a.run(p.body);
            const auto& values = p.body[0]->values;
            require(values[0]->kind == E_REAL && values[0]->realValue == 3.5, "mixed addition");
            require(values[1]->kind == E_REAL && values[1]->realValue == 3.5, "real division");
            require(values[2]->kind == E_REAL && values[2]->realValue == -2.5, "real negation");
            require(values[3]->kind == E_BOOL && !values[3]->boolValue, "integer precision lost");
            require(values[4]->kind == E_BOOL && values[4]->boolValue, "comparison");
            require(stod(values[0]->text) == values[0]->realValue, "real AST text");
        });

        test("non-finite real results and signed-zero division stay at runtime", [] {
            string large = "1" + string(308, '0') + ".0";
            staysBinary(large + " * 10.0");
            staysBinary(large + " + " + large);
            staysBinary("1.0 / -0.0");
            Program p("print 1.0 / 3.0, -0.0");
            Analyzer a;
            a.run(p.body);
            const Expr* third = p.body[0]->values[0];
            require(third->kind == E_REAL && stod(third->text) == third->realValue,
                    "fractional real text lost precision");
            require(std::signbit(p.body[0]->values[1]->realValue), "lost negative zero");
        });

        test("strings and boolean constants", [] {
            Program p("print 'ab' + 'cd', 'a' = 'a', 'a' /= 'b', not false, "
                      "true and false, false or true, true xor true");
            Analyzer a;
            a.run(p.body);
            const auto& v = p.body[0]->values;
            require(v[0]->kind == E_STRING && v[0]->text == "abcd", "concatenation");
            const bool expected[] = {true, true, true, false, true, false};
            for (size_t i = 1; i < v.size(); ++i)
                require(v[i]->kind == E_BOOL && v[i]->boolValue == expected[i - 1], "boolean value");
        });

        test("side effects, dynamic names and type errors are retained", [] {
            for (const string& expression : {"true or f()", "false and f()", "0 * f()",
                    "x + 1", "1 + '2'", "none + 1", "true + false", "'a' < 'b'",
                    "[1] + [2]", "{a := 1} + {a := 2}"})
                staysBinary(expression);
            Program p("var f := func => y\nvar y := 3\nprint f()\n"
                      "var y := 4\nvar g := func(x, x) => x");
            Analyzer a;
            a.run(p.body);
            require(p.body.size() == 5, "dynamic declarations were rejected or removed");
        });

        test("folding reaches every expression position and preserves locations", [] {
            Program p("var a := [2 + 3, {x := 3 * 4}], f := func(x) => x + (1 + 2)\n"
                      "a[1 + 1] := g(2 * 3)\nif 3 < 5 then print 1 + 1 end\n"
                      "for i in 1 + 1 .. 2 * 3 loop print i end\nwhile 2 < 1 loop exit end");
            Analyzer a;
            a.run(p.body);
            require(p.body[0]->values[0]->items[0]->intValue == 5, "array item");
            require(p.body[0]->values[0]->items[1]->items[0]->intValue == 12, "tuple item");
            require(p.body[0]->values[1]->bodyExpr->right->intValue == 3, "arrow function");
            require(p.body[1]->target->index->intValue == 2, "assignment index");
            require(p.body[1]->value->items[0]->intValue == 6, "call argument");
            require(p.body[2]->cond->kind == E_BOOL && p.body[2]->cond->boolValue, "if condition");
            require(p.body[3]->from->intValue == 2 && p.body[3]->to->intValue == 6, "for bounds");
            require(p.body[4]->cond->kind == E_BOOL && !p.body[4]->cond->boolValue, "while condition");
            const Expr* folded = p.body[0]->values[0]->items[0];
            require(folded->line == 1 && folded->column == 13, "folded source location");
        });

        test("unreachable tails are removed in their own bodies", [] {
            Program p("var f := func is\nif true then return 1; print 111 end\n"
                      "print 222\nreturn 2\nprint 333\nend\n"
                      "loop exit; print 444 end\nprint 555");
            Analyzer a;
            a.run(p.body);
            const auto& body = p.body[0]->values[0]->body;
            require(body.size() == 3 && body[1]->kind == S_PRINT, "conditional return removed live code");
            require(body[0]->thenBody.size() == 1, "branch tail");
            require(p.body[1]->body.size() == 1 && p.body[2]->kind == S_PRINT, "loop tail");
            require(a.removedStatements == 3, "removed count");
            string once = tree(p);
            a.run(p.body);
            require(tree(p) == once && a.foldedExpressions == 0 && a.removedStatements == 0,
                    "optimization is not idempotent");
        });
    } catch (const exception&) {
        return 1;
    }
    cout << passed << " test groups passed\n";
    return 0;
}
