// parser.cpp — синтаксический анализатор (парсер) языка D.
//
// Вход:  vector<Token> из лексера (lexer.cpp).
// Выход: дерево программы (AST) — vector<Stmt*>.
//
// Метод: рекурсивный спуск (Лекция 3, слайды 19–24): на каждое правило
// грамматики — своя функция parseXxx(). Устройство — "The First Option:
// Separation" из лекции Case Study (слайд 4): узлы дерева (Expr, Stmt) только
// хранят данные, а вся логика разбора — в struct Parser.
//
// Грамматика — "as decided" из README.md, §10 (спека Project D.pdf + исправления
// Q3, Q4, Q20 и оператор-вызов Q41).
//
// Этот файл не собирается сам по себе: его подключает main.cpp
// ПОСЛЕ lexer.cpp (парсеру нужны Token и TokenKind оттуда).

// ============================================================================
// === УЗЛЫ ДЕРЕВА (AST) ===
// ============================================================================
//
// Два вида узлов: выражение (Expr — что-то, что даёт значение) и оператор
// (Stmt — что-то, что выполняется). У каждого есть kind — "что это за узел";
// в зависимости от kind используются разные поля, остальные пустые.

struct Stmt;  // объявлен ниже, а Expr уже на него ссылается (тело функции)

enum ExprKind {
    // литералы
    E_INT,       // 42                     intValue
    E_REAL,      // 1.5                    realValue, text (как в исходнике)
    E_STRING,    // "abc"                  text
    E_BOOL,      // true / false           boolValue
    E_NONE,      // none
    E_ARRAY,     // [1, 2, 3]              items
    E_TUPLE,     // {a := 1, 2}            items + names ("" у безымянных)
    E_FUNC,      // func(x) => x + 1       names (параметры) + bodyExpr
                 // func(x) is ... end     names (параметры) + body

    // Reference — то, что начинается с имени
    E_NAME,      // x                      text
    E_INDEX,     // a[i]                   target, index
    E_CALL,      // f(x, y)                target, items (аргументы)
    E_FIELD,     // t.name                 target, text
    E_POSITION,  // t.2                    target, intValue

    // операции
    E_UNARY,     // -x, +x, not x          op, operand
    E_BINARY,    // a + b, a and b, a < b  op, left, right
    E_IS         // x is int               operand, text (имя типа)
};

struct Expr {
    ExprKind kind;
    int line = 0, column = 0;

    long long intValue = 0;
    double realValue = 0;
    bool boolValue = false;
    string text;

    TokenKind op = T_EOF;               // знак операции (T_PLUS, T_AND, ...)
    Expr* operand = nullptr;            // E_UNARY, E_IS
    Expr* left = nullptr;               // E_BINARY
    Expr* right = nullptr;              // E_BINARY

    Expr* target = nullptr;             // E_INDEX, E_CALL, E_FIELD, E_POSITION: к чему применяем
    Expr* index = nullptr;              // E_INDEX

    vector<Expr*> items;                // элементы массива / кортежа, аргументы вызова
    vector<string> names;               // имена элементов кортежа, параметры функции

    vector<Stmt*> body;                 // E_FUNC в форме  is ... end
    Expr* bodyExpr = nullptr;           // E_FUNC в форме  => выражение
};

enum StmtKind {
    S_VAR,       // var a := 1, b             names + values (nullptr, если без :=)
    S_ASSIGN,    // a[1] := 5                 target, value
    S_IF,        // if c then .. else .. end  cond, thenBody, elseBody; isShort для if c => ..
    S_WHILE,     // while c loop .. end       cond, body
    S_FOR,       // for i in 1..10 loop .. end    loopVar ("" если нет), from, to, body
                 // for x in arr loop .. end      loopVar, from (сам массив/кортеж), to = nullptr
    S_LOOP,      // loop .. end (бесконечный) body
    S_EXIT,      // exit
    S_RETURN,    // return [выражение]        value (может быть nullptr)
    S_PRINT,     // print a, b                values
    S_CALL       // f(x) отдельной строкой    value (узел E_CALL)
};

struct Stmt {
    StmtKind kind;
    int line = 0, column = 0;

    vector<string> names;               // S_VAR
    vector<Expr*> values;               // S_VAR, S_PRINT

    Expr* target = nullptr;             // S_ASSIGN
    Expr* value = nullptr;              // S_ASSIGN, S_RETURN, S_CALL

    Expr* cond = nullptr;               // S_IF, S_WHILE
    vector<Stmt*> thenBody, elseBody;   // S_IF
    bool isShort = false;               // S_IF: короткая форма  if c => ...

    string loopVar;                     // S_FOR
    Expr* from = nullptr;               // S_FOR
    Expr* to = nullptr;                 // S_FOR
    vector<Stmt*> body;                 // S_WHILE, S_FOR, S_LOOP
};

// ============================================================================
// === ПАРСЕР ===
// ============================================================================

struct Parser {
    vector<Token> tokens;
    size_t pos = 0;

    // Для проверок из Q24: exit только внутри цикла, return только внутри функции.
    int loopDepth = 0;
    int funcDepth = 0;

    Parser(vector<Token> t) : tokens(t) {}

    // ----

    Token& peek(int ahead = 0) {
        size_t i = pos + ahead;
        if (i >= tokens.size()) return tokens.back();   // последний токен всегда T_EOF
        return tokens[i];
    }

    bool check(TokenKind k) { return peek().kind == k; }

    Token advance() {
        Token t = peek();
        if (t.kind != T_EOF) pos++;
        return t;
    }

    bool match(TokenKind k) {
        if (!check(k)) return false;
        advance();
        return true;
    }

    // Текущий токен обязан быть k, иначе — синтаксическая ошибка.
    Token expect(TokenKind k, string what) {
        if (!check(k)) fail("expected " + what + ", got " + describe(peek()));
        return advance();
    }

    // Как показать токен в сообщении об ошибке.
    string describe(Token t) {
        if (t.kind == T_IDENT)  return "identifier '" + t.text + "'";
        if (t.kind == T_INT || t.kind == T_REAL) return "number " + t.text;
        if (t.kind == T_STRING) return "string \"" + t.text + "\"";
        if (t.kind == T_NEWLINE) return "end of line";
        if (t.kind == T_EOF)     return "end of file";
        return "'" + tokenName(t.kind) + "'";
    }

    void fail(string message) { failAt(peek(), message); }

    void failAt(Token t, string message) {
        throw runtime_error("Syntax error at " + to_string(t.line) + ":" +
                            to_string(t.column) + ": " + message);
    }

    // Внутри скобок ( ) [ ] { } перевод строки ничего не значит
    void skipNewlines() {
        while (check(T_NEWLINE)) advance();
    }

    Expr* newExpr(ExprKind kind, Token t) {
        Expr* e = new Expr();
        e->kind = kind;
        e->line = t.line;
        e->column = t.column;
        return e;
    }

    Stmt* newStmt(StmtKind kind, Token at) {
        Stmt* s = new Stmt();
        s->kind = kind;
        s->line = at.line;
        s->column = at.column;
        return s;
    }

    // ========================================================================
    // Программа и тела
    // ========================================================================

    // Основной цикл для парсинга
    // Program : { Statement [ ; ] }
    vector<Stmt*> parseProgram() {
        vector<Stmt*> program;
        while (true) {
            skipNewlines();
            if (check(T_EOF)) break;
            program.push_back(parseStatement());
            endOfStatement();
        }
        return program;
    }

    // После оператора: необязательная ';'
    void endOfStatement() {
        match(T_SEMICOLON);
    }

    // Body : Statement { [ ; ] Statement }
    // Тело заканчивается на 'end' или 'else'
    // Хотя бы один оператор обязателен (Q35).
    vector<Stmt*> parseBody(string where) {
        vector<Stmt*> body;
        while (true) {
            skipNewlines();
            if (check(T_END) || check(T_ELSE) || check(T_EOF)) break;
            body.push_back(parseStatement());
            endOfStatement();
        }
        if (body.empty()) fail("empty " + where + ": a body needs at least one statement");
        return body;
    }

    // Тело короткого if:  if c => Body
    // Идёт до конца строки или до первого end / else на этой строке (Q36).
    vector<Stmt*> parseShortBody() {
        vector<Stmt*> body;
        while (true) {
            if (check(T_NEWLINE) || check(T_EOF) || check(T_END) || check(T_ELSE)) break;
            body.push_back(parseStatement());
            endOfStatement();
        }
        if (body.empty()) fail("empty body after '=>'");
        return body;
    }

    // ========================================================================
    // Операторы (Statement)
    // ========================================================================

    // Statement : Declaration | Assignment | If | IfShort | Loop
    //           | Exit | Return | Print | Call
    Stmt* parseStatement() {
        switch (peek().kind) {
            case T_VAR:    return parseDeclaration();
            case T_IF:     return parseIf();
            case T_WHILE:  return parseWhile();
            case T_FOR:    return parseFor();
            case T_LOOP:   return parseInfiniteLoop();
            case T_EXIT:   return parseExit();
            case T_RETURN: return parseReturn();
            case T_PRINT:  return parsePrint();
            case T_IDENT:  return parseAssignmentOrCall();
            default:
                fail("expected a statement, got " + describe(peek()));
        }
        return nullptr;
    }

    // Declaration        : var VariableDefinition { , VariableDefinition }
    // VariableDefinition : IDENT [ := Expression ]
    Stmt* parseDeclaration() {
        Stmt* s = newStmt(S_VAR, expect(T_VAR, "'var'"));
        do {
            Token name = expect(T_IDENT, "a variable name");
            Expr* init = nullptr;
            if (match(T_ASSIGN)) init = parseExpression();
            s->names.push_back(name.text);
            s->values.push_back(init);
        } while (match(T_COMMA));
        return s;
    }

    // Assignment : Reference := Expression
    // Call       : Reference ( Expression { , Expression } )    — Q41
    // Оба начинаются с Reference, поэтому сначала читаем его, а потом смотрим,
    // что дальше: ':=' — присваивание, иначе это должен быть вызов.
    Stmt* parseAssignmentOrCall() {
        Token start = peek();
        Expr* ref = parseReference();

        if (check(T_ASSIGN)) {
            Token assign = advance();
            if (ref->kind == E_CALL)
                failAt(assign, "cannot assign to a function call");     // f() := 1 (Q24)
            Stmt* s = newStmt(S_ASSIGN, start);
            s->target = ref;
            s->value = parseExpression();
            return s;
        }

        if (ref->kind == E_CALL) {
            Stmt* s = newStmt(S_CALL, start);
            s->value = ref;
            return s;
        }

        fail("expected ':=' or a call after '" + start.text +
             "' (an expression alone is not a statement), got " + describe(peek()));
        return nullptr;
    }

    // If      : if Expression then Body [ else Body ] end
    // IfShort : if Expression => Body
    // Начало одинаковое; после условия смотрим: 'then' или '=>'.
    Stmt* parseIf() {
        Stmt* s = newStmt(S_IF, expect(T_IF, "'if'"));
        s->cond = parseExpression();

        if (match(T_ARROW)) {
            s->isShort = true;
            s->thenBody = parseShortBody();
            return s;
        }

        expect(T_THEN, "'then' or '=>' after the condition");
        s->thenBody = parseBody("'then' branch");
        if (match(T_ELSE)) s->elseBody = parseBody("'else' branch");
        expect(T_END, "'end' to close 'if'");
        return s;
    }

    // WhileLoop : while Expression LoopBody
    Stmt* parseWhile() {
        Stmt* s = newStmt(S_WHILE, expect(T_WHILE, "'while'"));
        s->cond = parseExpression();
        s->body = parseLoopBody();
        return s;
    }

    // ForLoop   : [ ForHeader ] LoopBody
    // ForHeader : for [ IDENT in ] Expression [ .. Expression ]
    Stmt* parseFor() {
        Stmt* s = newStmt(S_FOR, expect(T_FOR, "'for'"));

        // "i in" есть, только если сразу за именем идёт 'in' — смотрим на 2 токена вперёд.
        if (check(T_IDENT) && peek(1).kind == T_IN) {
            s->loopVar = advance().text;
            advance();   // in
        }

        s->from = parseExpression();
        if (match(T_RANGE)) s->to = parseExpression();

        s->body = parseLoopBody();
        return s;
    }

    // ForLoop без заголовка:  loop Body end  — бесконечный цикл.
    Stmt* parseInfiniteLoop() {
        Stmt* s = newStmt(S_LOOP, peek());
        s->body = parseLoopBody();
        return s;
    }

    // LoopBody : loop Body end
    vector<Stmt*> parseLoopBody() {
        expect(T_LOOP, "'loop'");
        loopDepth++;
        vector<Stmt*> body = parseBody("loop body");
        loopDepth--;
        expect(T_END, "'end' to close the loop");
        return body;
    }

    // Exit : exit    — только внутри цикла (Q24).
    Stmt* parseExit() {
        Token t = expect(T_EXIT, "'exit'");
        if (loopDepth == 0) failAt(t, "'exit' outside of a loop");
        return newStmt(S_EXIT, t);
    }

    // Return : return [ Expression ]    — только внутри функции (Q14, Q24).
    Stmt* parseReturn() {
        Token t = expect(T_RETURN, "'return'");
        if (funcDepth == 0) failAt(t, "'return' outside of a function");
        Stmt* s = newStmt(S_RETURN, t);
        // Выражения нет, если сразу за return кончается оператор.
        if (!check(T_NEWLINE) && !check(T_SEMICOLON) && !check(T_END) &&
            !check(T_ELSE) && !check(T_EOF))
            s->value = parseExpression();
        return s;
    }

    // Print : print Expression { , Expression }
    Stmt* parsePrint() {
        Stmt* s = newStmt(S_PRINT, expect(T_PRINT, "'print'"));
        do {
            s->values.push_back(parseExpression());
        } while (match(T_COMMA));
        return s;
    }

    // ========================================================================
    // Выражения (Expression) — от самого слабого приоритета к самому сильному
    // ========================================================================

    // Expression : Relation { ( or | and | xor ) Relation }
    // Все три на одном уровне, слева направо (Q30).
    Expr* parseExpression() {
        Expr* left = parseRelation();
        while (check(T_OR) || check(T_AND) || check(T_XOR)) {
            Token op = advance();
            left = makeBinary(op, left, parseRelation());
        }
        return left;
    }

    // Relation : Factor [ ( < | <= | > | >= | = | /= ) Factor ]
    // Не больше одного сравнения: 1 < 2 < 3 — ошибка.
    Expr* parseRelation() {
        Expr* left = parseFactor();
        if (check(T_LESS) || check(T_LESS_EQ) || check(T_GREATER) ||
            check(T_GREATER_EQ) || check(T_EQUAL) || check(T_NOT_EQUAL)) {
            Token op = advance();
            left = makeBinary(op, left, parseFactor());
        }
        return left;
    }

    // Factor : Term { ( + | - ) Term }        (Q3: знак обязателен)
    Expr* parseFactor() {
        Expr* left = parseTerm();
        while (check(T_PLUS) || check(T_MINUS)) {
            Token op = advance();
            left = makeBinary(op, left, parseTerm());
        }
        return left;
    }

    // Term : Unary { ( * | / ) Unary }
    Expr* parseTerm() {
        Expr* left = parseUnary();
        while (check(T_STAR) || check(T_SLASH)) {
            Token op = advance();
            left = makeBinary(op, left, parseUnary());
        }
        return left;
    }

    Expr* makeBinary(Token op, Expr* left, Expr* right) {
        Expr* e = newExpr(E_BINARY, op);
        e->op = op.kind;
        e->left = left;
        e->right = right;
        return e;
    }

    // Unary : [ + | - | not ] Unary              (Q4: знаки можно ставить подряд)
    //       | Primary [ is TypeIndicator ]       (Q20: is на любом Primary)
    Expr* parseUnary() {
        if (check(T_PLUS) || check(T_MINUS) || check(T_NOT)) {
            Token op = advance();
            Expr* e = newExpr(E_UNARY, op);
            e->op = op.kind;
            e->operand = parseUnary();
            return e;
        }

        Expr* primary = parsePrimary();
        if (check(T_IS)) {
            Token isTok = advance();
            Expr* e = newExpr(E_IS, isTok);
            e->operand = primary;
            e->text = parseTypeIndicator();
            return e;
        }
        return primary;
    }

    // TypeIndicator : int | real | bool | string | none | [ ] | { } | func
    string parseTypeIndicator() {
        if (match(T_TYPE_INT))    return "int";
        if (match(T_TYPE_REAL))   return "real";
        if (match(T_TYPE_BOOL))   return "bool";
        if (match(T_TYPE_STRING)) return "string";
        if (match(T_NONE))        return "none";
        if (match(T_FUNC))        return "func";
        if (match(T_LBRACKET)) { expect(T_RBRACKET, "']' (type [ ])"); return "[]"; }
        if (match(T_LBRACE))   { expect(T_RBRACE, "'}' (type { })");   return "{}"; }
        fail("expected a type after 'is' (int, real, bool, string, none, [ ], { }, func), got " +
             describe(peek()));
        return "";
    }

    // Primary : Reference | Literal | FunctionLiteral | ( Expression )
    // Literal : INTEGER | REAL | STRING | true | false | none | Tuple | Array
    Expr* parsePrimary() {
        Token t = peek();
        switch (t.kind) {
            case T_INT: {
                advance();
                Expr* e = newExpr(E_INT, t);
                e->intValue = t.intValue;
                return e;
            }
            case T_REAL: {
                advance();
                Expr* e = newExpr(E_REAL, t);
                e->realValue = t.realValue;
                e->text = t.text;
                return e;
            }
            case T_STRING: {
                advance();
                Expr* e = newExpr(E_STRING, t);
                e->text = t.text;
                return e;
            }
            case T_TRUE:
            case T_FALSE: {
                advance();
                Expr* e = newExpr(E_BOOL, t);
                e->boolValue = (t.kind == T_TRUE);
                return e;
            }
            case T_NONE:
                advance();
                return newExpr(E_NONE, t);

            case T_LBRACKET: return parseArray();
            case T_LBRACE:   return parseTuple();
            case T_FUNC:     return parseFunction();
            case T_IDENT:    return parseReference();

            case T_LPAREN: {
                advance();
                skipNewlines();
                Expr* inner = parseExpression();
                skipNewlines();
                expect(T_RPAREN, "')'");
                return inner;   // скобки нужны только для порядка — в дереве их нет
            }

            default:
                fail("expected an expression, got " + describe(t));
        }
        return nullptr;
    }

    // Array : [ [ Expression { , Expression } ] ]     — может быть пустым: []
    Expr* parseArray() {
        Expr* e = newExpr(E_ARRAY, expect(T_LBRACKET, "'['"));
        skipNewlines();
        if (match(T_RBRACKET)) return e;
        do {
            skipNewlines();
            e->items.push_back(parseExpression());
            skipNewlines();
        } while (match(T_COMMA));
        expect(T_RBRACKET, "',' or ']' in the array");
        return e;
    }

    // Tuple        : { TupleElement { , TupleElement } }    — пустого {} нет (Q33)
    // TupleElement : [ IDENT := ] Expression
    Expr* parseTuple() {
        Expr* e = newExpr(E_TUPLE, expect(T_LBRACE, "'{'"));
        skipNewlines();
        if (check(T_RBRACE)) fail("empty tuple '{}' is not allowed ({} is only a type after 'is')");
        do {
            skipNewlines();
            string name = "";
            // Именованный элемент, только если за именем сразу идёт ':='.
            if (check(T_IDENT) && peek(1).kind == T_ASSIGN) {
                name = advance().text;
                advance();   // :=
            }
            e->names.push_back(name);
            e->items.push_back(parseExpression());
            skipNewlines();
        } while (match(T_COMMA));
        expect(T_RBRACE, "',' or '}' in the tuple");
        return e;
    }

    // FunctionLiteral : func [ ( IDENT { , IDENT } ) ] FunBody
    // FunBody         : is Body end
    //                 | => Expression
    Expr* parseFunction() {
        Expr* e = newExpr(E_FUNC, expect(T_FUNC, "'func'"));

        if (match(T_LPAREN)) {
            skipNewlines();
            if (!check(T_RPAREN)) {           // func() — то же, что func
                do {
                    skipNewlines();
                    e->names.push_back(expect(T_IDENT, "a parameter name").text);
                    skipNewlines();
                } while (match(T_COMMA));
            }
            expect(T_RPAREN, "',' or ')' after parameters");
        }

        // Тело функции — новый контекст: exit из внешнего цикла здесь нельзя,
        // а return — можно.
        int savedLoopDepth = loopDepth;
        loopDepth = 0;
        funcDepth++;

        if (match(T_ARROW)) {
            e->bodyExpr = parseExpression();
        } else if (match(T_IS)) {
            e->body = parseBody("function body");
            expect(T_END, "'end' to close the function");
        } else {
            fail("expected 'is' or '=>' in the function literal, got " + describe(peek()));
        }

        funcDepth--;
        loopDepth = savedLoopDepth;
        return e;
    }

    // Reference : IDENT
    //           | Reference [ Expression ]                    элемент массива
    //           | Reference ( Expression { , Expression } )   вызов
    //           | Reference . IDENT                           элемент кортежа по имени
    //           | Reference . IntegerLiteral                  элемент кортежа по номеру
    // Левая рекурсия -> цикл (как в Лекции 3, слайд 23).
    Expr* parseReference() {
        Token name = expect(T_IDENT, "name");
        Expr* ref = newExpr(E_NAME, name);
        ref->text = name.text;

        while (true) {
            if (check(T_LBRACKET)) {
                Expr* e = newExpr(E_INDEX, advance());
                e->target = ref;
                skipNewlines();
                e->index = parseExpression();
                skipNewlines();
                expect(T_RBRACKET, "']'");
                ref = e;
            } else if (check(T_LPAREN)) {
                Expr* e = newExpr(E_CALL, advance());
                e->target = ref;
                skipNewlines();
                if (!check(T_RPAREN)) {       // f() — вызов без аргументов
                    do {
                        skipNewlines();
                        e->items.push_back(parseExpression());
                        skipNewlines();
                    } while (match(T_COMMA));
                }
                expect(T_RPAREN, "',' or ')' in the call");
                ref = e;
            } else if (check(T_DOT)) {
                Token dot = advance();
                if (check(T_IDENT)) {
                    Expr* e = newExpr(E_FIELD, dot);
                    e->target = ref;
                    e->text = advance().text;
                    ref = e;
                } else if (check(T_INT)) {
                    Expr* e = newExpr(E_POSITION, dot);
                    e->target = ref;
                    e->intValue = advance().intValue;
                    ref = e;
                } else {
                    fail("expected a name or a number after '.', got " + describe(peek()));
                }
            } else {
                break;
            }
        }
        return ref;
    }
};

// ============================================================================
// === ПЕЧАТЬ ДЕРЕВА ===
// ============================================================================
//
// Два шага. Сначала AST переводится в простое дерево подписей (TreeNode:
// строка + дети). Потом printTree рисует его линиями, как команда tree:
//
//   Program
//   ├── Declaration
//   │   └── x :=
//   │       └── Int 1
//   └── Print
//       └── Name x

struct TreeNode {
    string label;
    vector<TreeNode> kids = {};   // у листа детей нет: {"Int 1"} — это лист
};

TreeNode stmtTree(Stmt* s);

vector<TreeNode> stmtTrees(const vector<Stmt*>& list) {
    vector<TreeNode> kids;
    for (Stmt* s : list) kids.push_back(stmtTree(s));
    return kids;
}

TreeNode exprTree(Expr* e) {
    switch (e->kind) {
        case E_INT:    return {"Int " + to_string(e->intValue)};
        case E_REAL:   return {"Real " + e->text};
        case E_STRING: return {"String \"" + e->text + "\""};
        case E_BOOL:   return {e->boolValue ? "Bool true" : "Bool false"};
        case E_NONE:   return {"None"};

        case E_ARRAY: {
            TreeNode n = {"Array (" + to_string(e->items.size()) + " elements)"};
            for (Expr* item : e->items) n.kids.push_back(exprTree(item));
            return n;
        }

        case E_TUPLE: {
            TreeNode n = {"Tuple"};
            for (size_t i = 0; i < e->items.size(); i++) {
                if (e->names[i] != "") n.kids.push_back({e->names[i] + " :=", {exprTree(e->items[i])}});
                else                   n.kids.push_back(exprTree(e->items[i]));
            }
            return n;
        }

        case E_FUNC: {
            string params;
            for (size_t i = 0; i < e->names.size(); i++)
                params += (i ? ", " : "") + e->names[i];
            TreeNode n = {"Function (" + params + ")"};
            if (e->bodyExpr) n.kids.push_back({"=>", {exprTree(e->bodyExpr)}});
            else             n.kids.push_back({"is", stmtTrees(e->body)});
            return n;
        }

        case E_NAME: return {"Name " + e->text};

        case E_INDEX:
            return {"Index", {exprTree(e->target), exprTree(e->index)}};

        case E_CALL: {
            TreeNode n = {"Call (" + to_string(e->items.size()) + " args)", {exprTree(e->target)}};
            for (Expr* arg : e->items) n.kids.push_back(exprTree(arg));
            return n;
        }

        case E_FIELD:    return {"Field ." + e->text, {exprTree(e->target)}};
        case E_POSITION: return {"Position ." + to_string(e->intValue), {exprTree(e->target)}};
        case E_UNARY:    return {"Unary " + tokenName(e->op), {exprTree(e->operand)}};
        case E_BINARY:   return {"Binary " + tokenName(e->op), {exprTree(e->left), exprTree(e->right)}};
        case E_IS:       return {"Is " + e->text, {exprTree(e->operand)}};
    }
    return {"?"};
}

TreeNode stmtTree(Stmt* s) {
    switch (s->kind) {
        case S_VAR: {
            TreeNode n = {"Declaration"};
            for (size_t i = 0; i < s->names.size(); i++) {
                if (s->values[i]) n.kids.push_back({s->names[i] + " :=", {exprTree(s->values[i])}});
                else              n.kids.push_back({s->names[i] + " (none)"});
            }
            return n;
        }

        case S_ASSIGN: {
            TreeNode n = {"Assignment", {exprTree(s->target)}};
            n.kids.push_back({":=", {exprTree(s->value)}});
            return n;
        }

        case S_IF: {
            TreeNode n = {s->isShort ? "IfShort" : "If"};
            n.kids.push_back({"condition", {exprTree(s->cond)}});
            n.kids.push_back({"then", stmtTrees(s->thenBody)});
            if (!s->elseBody.empty()) n.kids.push_back({"else", stmtTrees(s->elseBody)});
            return n;
        }

        case S_WHILE: {
            TreeNode n = {"While"};
            n.kids.push_back({"condition", {exprTree(s->cond)}});
            n.kids.push_back({"body", stmtTrees(s->body)});
            return n;
        }

        case S_FOR: {
            TreeNode n = {"For" + (s->loopVar != "" ? " " + s->loopVar : "")};
            if (s->to) {
                n.kids.push_back({"from", {exprTree(s->from)}});
                n.kids.push_back({"to", {exprTree(s->to)}});
            } else {
                n.kids.push_back({"over", {exprTree(s->from)}});
            }
            n.kids.push_back({"body", stmtTrees(s->body)});
            return n;
        }

        case S_LOOP: return {"Loop (infinite)", stmtTrees(s->body)};
        case S_EXIT: return {"Exit"};

        case S_RETURN: {
            TreeNode n = {"Return"};
            if (s->value) n.kids.push_back(exprTree(s->value));
            return n;
        }

        case S_PRINT: {
            TreeNode n = {"Print"};
            for (Expr* v : s->values) n.kids.push_back(exprTree(v));
            return n;
        }

        case S_CALL: return {"CallStatement", {exprTree(s->value)}};
    }
    return {"?"};
}

// prefix — линии от предков: "│   " под предком, у которого ниже ещё есть
// братья, и "    " под предком, который был последним.
void printKids(const TreeNode& n, string prefix) {
    for (size_t i = 0; i < n.kids.size(); i++) {
        bool last = (i + 1 == n.kids.size());
        cout << prefix << (last ? "└── " : "├── ") << n.kids[i].label << "\n";
        printKids(n.kids[i], prefix + (last ? "    " : "│   "));
    }
}

void printTree(const TreeNode& root) {
    cout << root.label << "\n";
    printKids(root, "");
}
