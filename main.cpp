// Single-translation-unit build order: lexer -> parser -> analyzer -> runtime -> interpreter.
// Default invocation executes the validated program; --ast and --before-after inspect trees.
#include "lexer.cpp"
#include "parser.cpp"
#include "analyzer.cpp"
#include "runtime.cpp"
#include "interpreter.cpp"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

string readFile(const string& path) {
    ifstream file(path);
    if (!file) throw runtime_error("cannot open file: " + path);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    bool onlyTokens = false;
    bool noOptimize = false;
    bool beforeAfter = false;
    bool dumpAst = false;
    string path;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--tokens") onlyTokens = true;
        else if (arg == "--no-optimize") noOptimize = true;
        else if (arg == "--before-after") beforeAfter = true;
        else if (arg == "--ast") dumpAst = true;
        else if (arg.rfind("--", 0) == 0 || !path.empty()) {
            cerr << "unknown option or extra argument: " << arg << "\n";
            return 1;
        } else path = arg;
    }

    if (path.empty() || (onlyTokens && (noOptimize || beforeAfter || dumpAst)) ||
        (noOptimize && beforeAfter) || (beforeAfter && dumpAst)) {
        cerr << "usage: main [--tokens | --before-after | --ast [--no-optimize] | --no-optimize] program.d\n";
        return 1;
    }

    vector<Stmt*> program;
    try {
        Lexer lexer(readFile(path));
        vector<Token> tokens = lexer.run();
        if (onlyTokens) {
            printTokens(tokens);
            return 0;
        }

        Parser parser(tokens);
        program = parser.parseProgram();
        Analyzer analyzer;
        analyzer.checkProgram(program);

        if (beforeAfter) {
            printTree({"Program before optimization", stmtTrees(program)});
            analyzer.optimizeProgram(program);
            printTree({"Program after optimization", stmtTrees(program)});
            cout << "Optimizations: " << analyzer.foldedExpressions << " constant expressions folded, "
                 << analyzer.removedStatements << " unreachable statements removed\n";
        } else {
            if (!noOptimize) analyzer.optimizeProgram(program);
            if (dumpAst) {
                printTree({noOptimize ? "Program before optimization" : "Program", stmtTrees(program)});
            } else {
                Interpreter interpreter(cout);
                interpreter.run(program);
            }
        }
        destroyProgram(program);
    } catch (const exception& e) {
        destroyProgram(program);
        cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
