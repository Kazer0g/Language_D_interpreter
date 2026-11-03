// analyzer.cpp — семантический анализатор языка D.
// Подключается после parser.cpp. Узлы AST хранят данные, логика обхода — здесь.
//
// Первый проход только проверяет AST:
//   1. Контекст exit / return (функция не наследует циклы снаружи).
//   2. Допустимость цели присваивания.
// Второй проход изменяет AST:
//   1. Сворачивает безопасные выражения над простыми константами.
//   2. Удаляет хвост тела после безусловного return / exit.
// Типовые ошибки и поиск значений по именам остаются задачей интерпретатора.

#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <utility>

struct Analyzer {
    size_t foldedExpressions = 0;
    size_t removedStatements = 0;

    void run(vector<Stmt*>& program, bool optimize = true) {
        checkProgram(program);
        foldedExpressions = removedStatements = 0;
        if (optimize) optimizeProgram(program);
    }

    // const-обход: ни структура дерева, ни поля узлов не меняются.
    void checkProgram(const vector<Stmt*>& program) const {
        checkStmts(program, 0, false);
    }

    // даже недостижимый код сначала проверяется.
    void optimizeProgram(vector<Stmt*>& program) {
        foldedExpressions = removedStatements = 0;
        optimizeStmts(program);
    }

private:
    void fail(int line, int column, const string& message) const {
        throw runtime_error("Semantic error at " + to_string(line) + ":" +
                            to_string(column) + ": " + message);
    }

    void checkStmts(const vector<Stmt*>& body, int loopDepth, bool inFunction) const {
        for (const Stmt* s : body) {
            if (s->kind == S_EXIT && loopDepth == 0)
                fail(s->line, s->column, "'exit' outside of a loop");
            if (s->kind == S_RETURN && !inFunction)
                fail(s->line, s->column, "'return' outside of a function");

            if (s->kind == S_ASSIGN) {
                const Expr* target = s->target;
                if (!target || (target->kind != E_NAME && target->kind != E_INDEX &&
                                target->kind != E_FIELD && target->kind != E_POSITION))
                    fail(s->line, s->column, "invalid assignment target: cannot assign to a function call or value");
            }

            checkExpr(s->target);
            checkExpr(s->value);
            checkExpr(s->cond);
            checkExpr(s->from);
            checkExpr(s->to);
            for (const Expr* value : s->values) checkExpr(value);
            checkStmts(s->thenBody, loopDepth, inFunction);
            checkStmts(s->elseBody, loopDepth, inFunction);
            bool isLoop = s->kind == S_WHILE || s->kind == S_FOR || s->kind == S_LOOP;
            checkStmts(s->body, loopDepth + (isLoop ? 1 : 0), inFunction);
        }
    }

    void checkExpr(const Expr* e) const {
        if (!e) return;
        checkExpr(e->operand);
        checkExpr(e->left);
        checkExpr(e->right);
        checkExpr(e->target);
        checkExpr(e->index);
        for (const Expr* item : e->items) checkExpr(item);
        if (e->kind == E_FUNC) {
            // Новый контекст: return разрешён, exit требует собственного цикла.
            checkStmts(e->body, 0, true);
            checkExpr(e->bodyExpr);
        }
    }

    void optimizeStmts(vector<Stmt*>& body) {
        for (size_t i = 0; i < body.size(); ++i) {
            Stmt* s = body[i];
            optimizeExpr(s->target);
            optimizeExpr(s->value);
            optimizeExpr(s->cond);
            optimizeExpr(s->from);
            optimizeExpr(s->to);
            for (Expr* value : s->values) optimizeExpr(value);
            optimizeStmts(s->thenBody);
            optimizeStmts(s->elseBody);
            optimizeStmts(s->body);

            // Только непосредственный return / exit завершает это тело.
            // Return внутри условной ветки не удаляет операторы после самого if.
            if (s->kind == S_RETURN || s->kind == S_EXIT) {
                removedStatements += body.size() - i - 1;
                for (size_t j = i + 1; j < body.size(); ++j) destroyStmt(body[j]);
                body.resize(i + 1);
                break;
            }
        }
    }

    void optimizeExpr(Expr* e) {
        if (!e) return;
        optimizeExpr(e->operand);
        optimizeExpr(e->left);
        optimizeExpr(e->right);
        optimizeExpr(e->target);
        optimizeExpr(e->index);
        for (Expr* item : e->items) optimizeExpr(item);
        if (e->kind == E_FUNC) {
            optimizeExpr(e->bodyExpr);
            optimizeStmts(e->body);
        }

        // Снизу вверх: 2 + 3 * 4 -> 2 + 12 -> 14 за один обход.
        Expr literal{};
        bool folded = false;
        if (e->kind == E_UNARY) folded = foldUnary(e, literal);
        if (e->kind == E_BINARY) folded = foldBinary(e, literal);
        if (!folded) return;

        literal.line = e->line;
        literal.column = e->column;
        destroyExpr(e->operand);
        destroyExpr(e->left);
        destroyExpr(e->right);
        *e = std::move(literal);
        ++foldedExpressions;
    }

    bool foldUnary(const Expr* e, Expr& result) const {
        const Expr* value = e->operand;
        if (value->kind == E_BOOL && e->op == T_NOT) {
            result.kind = E_BOOL;
            result.boolValue = !value->boolValue;
            return true;
        }
        if (e->op != T_PLUS && e->op != T_MINUS) return false;
        if (value->kind == E_INT) {
            if (e->op == T_MINUS && value->intValue == numeric_limits<long long>::min())
                return false;
            result.kind = E_INT;
            result.intValue = e->op == T_MINUS ? -value->intValue : value->intValue;
            return true;
        }
        if (value->kind == E_REAL)
            return realLiteral(e->op == T_MINUS ? -value->realValue : value->realValue, result);
        return false;
    }

    template<class T>
    bool comparison(TokenKind op, T a, T b, Expr& result) const {
        bool value;
        switch (op) {
            case T_LESS:       value = a < b; break;
            case T_LESS_EQ:    value = a <= b; break;
            case T_GREATER:    value = a > b; break;
            case T_GREATER_EQ: value = a >= b; break;
            case T_EQUAL:      value = a == b; break;
            case T_NOT_EQUAL:  value = a != b; break;
            default: return false;
        }
        result.kind = E_BOOL;
        result.boolValue = value;
        return true;
    }

    bool foldIntegers(TokenKind op, long long a, long long b, Expr& result) const {
        if (comparison(op, a, b, result)) return true;
        const long long lo = numeric_limits<long long>::min();
        const long long hi = numeric_limits<long long>::max();
        long long value;
        switch (op) {
            case T_PLUS:
                if ((b > 0 && a > hi - b) || (b < 0 && a < lo - b)) return false;
                value = a + b;
                break;
            case T_MINUS:
                if ((b > 0 && a < lo + b) || (b < 0 && a > hi + b)) return false;
                value = a - b;
                break;
            case T_STAR:
                // Проверяем границы ДО умножения, чтобы не получить UB в C++.
                if (a > 0 && ((b > 0 && a > hi / b) || (b < 0 && b < lo / a)))
                    return false;
                if (a < 0 && ((b > 0 && a < lo / b) || (b < 0 && a < hi / b)))
                    return false;
                value = a * b;
                break;
            case T_SLASH:
                if (b == 0 || (a == lo && b == -1)) return false;
                value = a / b;
                // C++ делит к нулю, D — к минус бесконечности.
                if (a % b != 0 && ((a < 0) != (b < 0))) --value;
                break;
            default: return false;
        }
        result.kind = E_INT;
        result.intValue = value;
        return true;
    }

    bool realLiteral(double value, Expr& result) const {
        if (!std::isfinite(value)) return false;
        result.kind = E_REAL;
        result.realValue = value;
        // text используется принтером AST; сохраняем точность double.
        ostringstream out;
        out.imbue(locale::classic());
        out << setprecision(numeric_limits<double>::max_digits10) << value;
        result.text = out.str();
        if (result.text.find_first_of(".eE") == string::npos) result.text += ".0";
        return true;
    }

    bool foldBinary(const Expr* e, Expr& result) const {
        const Expr* a = e->left;
        const Expr* b = e->right;
        if (a->kind == E_INT && b->kind == E_INT)
            return foldIntegers(e->op, a->intValue, b->intValue, result);

        bool aNumeric = a->kind == E_INT || a->kind == E_REAL;
        bool bNumeric = b->kind == E_INT || b->kind == E_REAL;
        if (aNumeric && bNumeric) {
            double x = a->kind == E_INT ? static_cast<double>(a->intValue) : a->realValue;
            double y = b->kind == E_INT ? static_cast<double>(b->intValue) : b->realValue;
            if (comparison(e->op, x, y, result)) return true;
            switch (e->op) {
                case T_PLUS:  return realLiteral(x + y, result);
                case T_MINUS: return realLiteral(x - y, result);
                case T_STAR:  return realLiteral(x * y, result);
                case T_SLASH: return y != 0.0 && realLiteral(x / y, result);
                default: return false;
            }
        }
        if (a->kind == E_STRING && b->kind == E_STRING) {
            if (e->op == T_PLUS) {
                result.kind = E_STRING;
                result.text = a->text + b->text;
                return true;
            }
            if (e->op == T_EQUAL || e->op == T_NOT_EQUAL)
                return comparison(e->op, a->text, b->text, result);
        }
        if (a->kind == E_BOOL && b->kind == E_BOOL) {
            result.kind = E_BOOL;
            switch (e->op) {
                case T_AND: result.boolValue = a->boolValue && b->boolValue; return true;
                case T_OR:  result.boolValue = a->boolValue || b->boolValue; return true;
                case T_XOR: result.boolValue = a->boolValue != b->boolValue; return true;
                case T_EQUAL:
                case T_NOT_EQUAL: return comparison(e->op, a->boolValue, b->boolValue, result);
                default: return false;
            }
        }
        return false;
    }
};
