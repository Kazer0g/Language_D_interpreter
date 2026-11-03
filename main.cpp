// main.cpp — точка входа интерпретатора языка D. Здесь весь пайплайн:
//
//     текст -> Lexer -> токены -> Parser -> AST -> Analyzer -> AST -> ...
//
// Сборка:  g++ -std=c++17 -o main main.cpp
// Запуск:  main program.d                 проверяет и печатает оптимизированное AST
//          main --tokens program.d        печатает токены
//          main --no-optimize program.d   проверяет и печатает исходное AST
//          main --before-after program.d  показывает AST до и после оптимизаций
//
// #include "файл.cpp" просто вставляет текст файла в это место.
// Порядок важен: парсер использует Token из лексера, поэтому лексер первым.

#include "lexer.cpp"
#include "parser.cpp"
#include "analyzer.cpp"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

string readFile(string path) {
    ifstream file(path);
    if (!file) throw runtime_error("cannot open file: " + path);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char** argv) {
#ifdef _WIN32
    // Ветки дерева печатаются в UTF-8, в том числе в Windows PowerShell.
    SetConsoleOutputCP(CP_UTF8);
#endif
    bool onlyTokens = false;
    bool noOptimize = false;
    bool beforeAfter = false;
    string path = "";

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--tokens") onlyTokens = true;
        else if (arg == "--no-optimize") noOptimize = true;
        else if (arg == "--before-after") beforeAfter = true;
        else if (arg.rfind("--", 0) == 0 || !path.empty()) {
            cerr << "unknown option or extra argument: " << arg << "\n";
            return 1;
        } else path = arg;
    }

    if (path.empty() || (onlyTokens && (noOptimize || beforeAfter)) || (noOptimize && beforeAfter)) {
        cerr << "usage: main [--tokens | --no-optimize | --before-after] program.d\n";
        return 1;
    }

    vector<Stmt*> program;
    try {
        string source = readFile(path);

        // 1. Лексер: текст -> токены
        Lexer lexer(source);
        vector<Token> tokens = lexer.run();
        if (onlyTokens) {
            printTokens(tokens);
            return 0;
        }

        // 2. Парсер: токены -> дерево
        Parser parser(tokens);
        program = parser.parseProgram();

        // 3. Analyzer: сначала проверки всего дерева, затем оптимизации.
        Analyzer analyzer;
        analyzer.checkProgram(program);
        if (beforeAfter) {
            printTree({"Program before optimization", stmtTrees(program)});
        }
        if (!noOptimize) analyzer.optimizeProgram(program);
        printTree({beforeAfter ? "Program after optimization" : "Program", stmtTrees(program)});
        if (beforeAfter) {
            cout << "Optimizations: " << analyzer.foldedExpressions << " constant expressions folded, "
                 << analyzer.removedStatements << " unreachable statements removed\n";
        }
        // 4. Интерпретатор: дерево -> выполнение (будет следующим шагом).
        destroyProgram(program);
    } catch (exception& e) {
        destroyProgram(program);
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
