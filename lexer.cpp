#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// === ТОКЕНЫ ===

enum TokenKind {
    // литералы и имена
    T_IDENT, T_INT, T_REAL, T_STRING,

    // ключевые слова
    T_VAR, T_IF, T_THEN, T_ELSE, T_END,
    T_WHILE, T_FOR, T_IN, T_LOOP,
    T_FUNC, T_IS, T_RETURN, T_PRINT, T_EXIT,
    T_AND, T_OR, T_XOR, T_NOT,
    T_TRUE, T_FALSE, T_NONE,
    T_TYPE_INT, T_TYPE_REAL, T_TYPE_BOOL, T_TYPE_STRING,

    // операторы
    T_ASSIGN,     // :=
    T_ARROW,      // =>
    T_PLUS, T_MINUS, T_STAR, T_SLASH,
    T_LESS, T_LESS_EQ, T_GREATER, T_GREATER_EQ,
    T_EQUAL,      // =
    T_NOT_EQUAL,  // /=
    T_RANGE,      // ..
    T_DOT, T_COMMA,

    // разделители
    T_SEMICOLON, T_NEWLINE,
    T_LPAREN, T_RPAREN,
    T_LBRACKET, T_RBRACKET,
    T_LBRACE, T_RBRACE,

    T_EOF
};

struct Token {
    TokenKind kind = T_EOF;
    string text;          // имя идентификатора / раскодированное значение строки
    long long intValue = 0;
    double realValue = 0;
    int line = 1;
    int column = 1;
};

// Имя токена для печати
string tokenName(TokenKind k) {
    switch (k) {
        case T_IDENT:  return "Identifier";
        case T_INT:    return "Integer";
        case T_REAL:   return "Real";
        case T_STRING: return "String";

        case T_VAR: return "var";       case T_IF: return "if";
        case T_THEN: return "then";     case T_ELSE: return "else";
        case T_END: return "end";       case T_WHILE: return "while";
        case T_FOR: return "for";       case T_IN: return "in";
        case T_LOOP: return "loop";     case T_FUNC: return "func";
        case T_IS: return "is";         case T_RETURN: return "return";
        case T_PRINT: return "print";   case T_EXIT: return "exit";
        case T_AND: return "and";       case T_OR: return "or";
        case T_XOR: return "xor";       case T_NOT: return "not";
        case T_TRUE: return "true";     case T_FALSE: return "false";
        case T_NONE: return "none";

        case T_TYPE_INT: return "int";       case T_TYPE_REAL: return "real";
        case T_TYPE_BOOL: return "bool";     case T_TYPE_STRING: return "string";

        case T_ASSIGN: return ":=";     case T_ARROW: return "=>";
        case T_PLUS: return "+";        case T_MINUS: return "-";
        case T_STAR: return "*";        case T_SLASH: return "/";
        case T_LESS: return "<";        case T_LESS_EQ: return "<=";
        case T_GREATER: return ">";     case T_GREATER_EQ: return ">=";
        case T_EQUAL: return "=";       case T_NOT_EQUAL: return "/=";
        case T_RANGE: return "..";      case T_DOT: return ".";
        case T_COMMA: return ",";

        case T_SEMICOLON: return ";";   case T_NEWLINE: return "Newline";
        case T_LPAREN: return "(";      case T_RPAREN: return ")";
        case T_LBRACKET: return "[";    case T_RBRACKET: return "]";
        case T_LBRACE: return "{";      case T_RBRACE: return "}";

        case T_EOF: return "EOF";
    }
    return "?";
}

// Ключевые слова языка D
unordered_map<string, TokenKind> KEYWORDS = {
    {"var", T_VAR},       {"if", T_IF},         {"then", T_THEN},
    {"else", T_ELSE},     {"end", T_END},       {"while", T_WHILE},
    {"for", T_FOR},       {"in", T_IN},         {"loop", T_LOOP},
    {"func", T_FUNC},     {"is", T_IS},         {"return", T_RETURN},
    {"print", T_PRINT},   {"exit", T_EXIT},     {"and", T_AND},
    {"or", T_OR},         {"xor", T_XOR},       {"not", T_NOT},
    {"true", T_TRUE},     {"false", T_FALSE},   {"none", T_NONE},
    {"int", T_TYPE_INT},  {"real", T_TYPE_REAL},
    {"bool", T_TYPE_BOOL},{"string", T_TYPE_STRING},
};

// === ЛЕКСЕР ===

struct Lexer {
    string src;
    size_t pos = 0;
    int line = 1, column = 1;
    int tokLine = 1, tokCol = 1;   // начало текущей лексемы
    vector<Token> tokens;

    Lexer(string source) : src(source) {}

    // ---- работа с символами ----
    bool atEnd()    { return pos >= src.size(); }
    char peek()     { return atEnd() ? '\0' : src[pos]; }
    char peekNext() { return pos + 1 >= src.size() ? '\0' : src[pos + 1]; }

    char advance() {
        char c = src[pos++];
        if (c == '\n') { line++; column = 1; } else { column++; }
        return c;
    }

    bool match(char expected) {
        if (peek() != expected) return false;
        advance();
        return true;
    }

    void add(TokenKind kind, string text) {
        Token t;
        t.kind = kind;
        t.text = text;
        t.line = tokLine;
        t.column = tokCol;
        tokens.push_back(t);
    }

    void fail(string message) {
        throw runtime_error("Lexical error at " + to_string(tokLine) + ":" +
                            to_string(tokCol) + ": " + message);
    }

    // ---- главный цикл ----
    vector<Token> run() {
        while (true) {
            skipSpaces();
            if (atEnd()) break;
            scanToken();
        }
        if (!tokens.empty() && tokens.back().kind == T_NEWLINE) tokens.pop_back();
        tokLine = line; tokCol = column;
        add(T_EOF, "");
        return tokens;
    }

    // Пробелы, табы и комментарии
    void skipSpaces() {
        while (!atEnd()) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r') {
                advance();
            } else if (c == '/' && peekNext() == '/') {
                while (!atEnd() && peek() != '\n') advance();
            } else {
                return;
            }
        }
    }

    void scanToken() {
        tokLine = line;
        tokCol = column;
        char c = peek();

        if (c == '\n') {
            advance();
            bool prevIsNewline = !tokens.empty() && tokens.back().kind == T_NEWLINE;
            if (!prevIsNewline && !tokens.empty()) add(T_NEWLINE, "\\n");
            return;
        }
        
        if (isdigit((unsigned char)c))                { scanNumber(); return; }
        if (isalpha((unsigned char)c) || c == '_')    { scanWord();   return; }
        if (c == '"' || c == '\'')                    { scanString(); return; }
        scanOperator();
    }

    void scanNumber() {
        string text;
        while (isdigit((unsigned char)peek())) text += advance();
        
        bool isReal = false;
        if (peek() == '.') {
            if (isdigit((unsigned char)peekNext())) { 
                isReal = true; 
                text += advance();
                while (isdigit((unsigned char)peek())) text += advance();
            }
            else if (peekNext() == '.') {  }
            else { fail("unexpected character after number"); }
        }

        if (isalpha((unsigned char)peek()) || peek() == '_')
            fail("unexpected character after a number");

        Token t;
        t.kind = isReal ? T_REAL : T_INT;
        t.text = text;
        t.line = tokLine;
        t.column = tokCol;
        try {
            if (isReal) t.realValue = stod(text);
            else        t.intValue  = stoll(text);
        } catch (out_of_range&) {
            fail("number is out of range: " + text);
        }
        tokens.push_back(t);
    }

    // Идентификатор или ключевое слово
    void scanWord() {
        string text;
        while (isalnum((unsigned char)peek()) || peek() == '_') text += advance();

        auto it = KEYWORDS.find(text);
        add(it != KEYWORDS.end() ? it->second : T_IDENT, text);
    }

    // Строка в одинарных или двойных кавычках
    void scanString() {
        char quote = advance();
        string text;

        while (true) {
            if (atEnd()) fail("not closed string");

            char c = peek();
            if (c == quote) { advance(); break; }

            if (c == '\\') {
                advance();
                if (atEnd()) fail("unterminated escape sequence");
                char e = advance();
                if      (e == 'n')  text += '\n';
                else if (e == 't')  text += '\t';
                else if (e == 'r')  text += '\r';
                else if (e == '\\') text += '\\';
                else if (e == '"')  text += '"';
                else if (e == '\'') text += '\'';
                else fail(string("unknown escape sequence: \\") + e);
                continue;
            }
            text += advance();
        }
        add(T_STRING, text);
    }

    void scanOperator() {
        char c = advance();
        switch (c) {
            case ':':
                if (match('=')) { add(T_ASSIGN, ":="); return; }
                fail("assignment is ':=', not ':'");
            case '=':
                if (match('>')) { add(T_ARROW, "=>"); return; }
                add(T_EQUAL, "="); return;
            case '/':
                if (match('=')) { add(T_NOT_EQUAL, "/="); return; }
                add(T_SLASH, "/"); return;
            case '<':
                if (match('=')) { add(T_LESS_EQ, "<="); return; }
                add(T_LESS, "<"); return;
            case '>':
                if (match('=')) { add(T_GREATER_EQ, ">="); return; }
                add(T_GREATER, ">"); return;
            case '.':
                if (match('.')) { add(T_RANGE, ".."); return; }
                add(T_DOT, "."); return;

            case '+': add(T_PLUS, "+");       return;
            case '-': add(T_MINUS, "-");      return;
            case '*': add(T_STAR, "*");       return;
            case ',': add(T_COMMA, ",");      return;
            case ';': add(T_SEMICOLON, ";");  return;
            case '(': add(T_LPAREN, "(");     return;
            case ')': add(T_RPAREN, ")");     return;
            case '[': add(T_LBRACKET, "[");   return;
            case ']': add(T_RBRACKET, "]");   return;
            case '{': add(T_LBRACE, "{");     return;
            case '}': add(T_RBRACE, "}");     return;

            default:
                fail(string("unexpected character: '") + c + "'");
        }
    }
};

// === MAIN ===

int main(int argc, char** argv) {
    string source;

    if (argc > 1) {
        ifstream file(argv[1]);
        if (!file) { cerr << "cannot open file: " << argv[1] << "\n"; return 1; }
        stringstream buffer;
        buffer << file.rdbuf();
        source = buffer.str();
    } else {
        stringstream buffer;
        buffer << cin.rdbuf();
        source = buffer.str();
    }

    try {
        Lexer lexer(source);
        vector<Token> tokens = lexer.run();

        for (Token t : tokens) {
            cout << t.line << ":" << t.column << "\t" << tokenName(t.kind);
            if      (t.kind == T_INT)    cout << "\t" << t.intValue;
            else if (t.kind == T_REAL)   cout << "\t" << t.realValue;
            else if (t.kind == T_IDENT)  cout << "\t" << t.text;
            else if (t.kind == T_STRING) cout << "\t\"" << t.text << "\"";
            cout << "\n";
        }
    } catch (exception& e) {
        cerr << e.what() << "\n";
        return 1;
    }

    return 0;
}