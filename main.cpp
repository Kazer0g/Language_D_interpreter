// main.cpp — точка входа интерпретатора языка D. Здесь весь пайплайн:
//
//     текст программы -> Lexer -> токены -> Parser -> дерево (AST) -> ...
//
// Сборка:  g++ -std=c++17 -o main main.cpp
// Запуск:  main program.d             печатает дерево
//          main --tokens program.d    печатает токены
//
// #include "файл.cpp" просто вставляет текст файла в это место.
// Порядок важен: парсер использует Token из лексера, поэтому лексер первым.

#include "lexer.cpp"
#include "parser.cpp"

string readFile(string path) {
    ifstream file(path);
    if (!file) throw runtime_error("cannot open file: " + path);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char** argv) {
    bool onlyTokens = false;
    string path = "";

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--tokens") onlyTokens = true;
        else path = arg;
    }

    if (path == "") {
        cerr << "usage: main [--tokens] program.d\n";
        return 1;
    }

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
        vector<Stmt*> program = parser.parseProgram();

        // 3. Интерпретатор: дерево -> выполнение (будет следующим шагом).
        //    Пока просто печатаем дерево.
        cout << "Program\n";
        printStmts(program, 1);
    } catch (exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}
