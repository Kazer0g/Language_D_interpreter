// Runtime operators. Included after the Runtime and Value declarations.
#include <cmath>
#include <limits>
#include <set>

namespace {
bool checkedAdd(long long left, long long right, long long& result) {
    const long long low = numeric_limits<long long>::min();
    const long long high = numeric_limits<long long>::max();
    if ((right > 0 && left > high - right) ||
        (right < 0 && left < low - right)) return false;
    result = left + right;
    return true;
}

bool checkedSubtract(long long left, long long right, long long& result) {
    const long long low = numeric_limits<long long>::min();
    const long long high = numeric_limits<long long>::max();
    if ((right > 0 && left < low + right) ||
        (right < 0 && left > high + right)) return false;
    result = left - right;
    return true;
}

bool checkedMultiply(long long left, long long right, long long& result) {
    const long long low = numeric_limits<long long>::min();
    const long long high = numeric_limits<long long>::max();
    if (left == 0 || right == 0) {
        result = 0;
        return true;
    }
    if (left == -1) {
        if (right == low) return false;
        result = -right;
        return true;
    }
    if (right == -1) {
        if (left == low) return false;
        result = -left;
        return true;
    }
    if (left > 0) {
        if ((right > 0 && left > high / right) ||
            (right < 0 && right < low / left)) return false;
    } else if ((right > 0 && left < low / right) ||
               (right < 0 && left < high / right)) return false;
    result = left * right;
    return true;
}

double asReal(const Value& value) {
    return value.kind == V_INT ? static_cast<double>(value.integer) : value.real;
}

bool isNumeric(const Value& value) {
    return value.kind == V_INT || value.kind == V_REAL;
}

bool isComparison(TokenKind operation) {
    return operation == T_LESS || operation == T_LESS_EQ ||
           operation == T_GREATER || operation == T_GREATER_EQ ||
           operation == T_EQUAL || operation == T_NOT_EQUAL;
}

bool compare(TokenKind operation, double left, double right) {
    switch (operation) {
    case T_LESS: return left < right;
    case T_LESS_EQ: return left <= right;
    case T_GREATER: return left > right;
    case T_GREATER_EQ: return left >= right;
    case T_EQUAL: return left == right;
    case T_NOT_EQUAL: return left != right;
    default: return false;
    }
}

bool compareIntegers(TokenKind operation, long long left, long long right) {
    switch (operation) {
    case T_LESS: return left < right;
    case T_LESS_EQ: return left <= right;
    case T_GREATER: return left > right;
    case T_GREATER_EQ: return left >= right;
    case T_EQUAL: return left == right;
    case T_NOT_EQUAL: return left != right;
    default: return false;
    }
}

bool compare(TokenKind operation, const string& left, const string& right) {
    if (operation == T_EQUAL) return left == right;
    if (operation == T_NOT_EQUAL) return left != right;
    return false;
}
}

Value Runtime::unary(const Expr* expression, const Value& operand) {
    if (expression->op == T_NOT) {
        if (operand.kind != V_BOOL)
            fail(expression, "operator 'not' requires a boolean operand");
        return boolValue(!operand.boolean);
    }
    if (expression->op == T_PLUS || expression->op == T_MINUS) {
        if (operand.kind == V_INT) {
            if (expression->op == T_PLUS) return operand;
            if (operand.integer == numeric_limits<long long>::min())
                fail(expression, "integer overflow");
            return intValue(-operand.integer);
        }
        if (operand.kind == V_REAL) {
            double result = expression->op == T_MINUS ? -operand.real : operand.real;
            if (!isfinite(result)) fail(expression, "real result is not finite");
            return realValue(result);
        }
    }
    fail(expression, "invalid operand type for unary operator");
}

Value Runtime::binary(const Expr* expression, const Value& left,
                      const Value& right) {
    const TokenKind operation = expression->op;
    if (isNumeric(left) && isNumeric(right)) {
        if (left.kind == V_INT && right.kind == V_INT) {
            if (isComparison(operation))
                return boolValue(compareIntegers(operation, left.integer,
                                                 right.integer));
            long long result = 0;
            bool valid = false;
            switch (operation) {
            case T_PLUS:
                valid = checkedAdd(left.integer, right.integer, result);
                break;
            case T_MINUS:
                valid = checkedSubtract(left.integer, right.integer, result);
                break;
            case T_STAR:
                valid = checkedMultiply(left.integer, right.integer, result);
                break;
            case T_SLASH:
                if (right.integer == 0) fail(expression, "division by zero");
                if (left.integer == numeric_limits<long long>::min() &&
                    right.integer == -1) fail(expression, "integer overflow");
                result = left.integer / right.integer;
                if (left.integer % right.integer != 0 &&
                    ((left.integer < 0) != (right.integer < 0))) --result;
                valid = true;
                break;
            default:
                fail(expression, "invalid numeric operator");
            }
            if (!valid) fail(expression, "integer overflow");
            return intValue(result);
        }
        double leftReal = asReal(left);
        double rightReal = asReal(right);
        if (isComparison(operation))
            return boolValue(compare(operation, leftReal, rightReal));
        double result = 0.0;
        switch (operation) {
        case T_PLUS: result = leftReal + rightReal; break;
        case T_MINUS: result = leftReal - rightReal; break;
        case T_STAR: result = leftReal * rightReal; break;
        case T_SLASH:
            if (rightReal == 0.0) fail(expression, "division by zero");
            result = leftReal / rightReal;
            break;
        default:
            fail(expression, "invalid numeric operator");
        }
        if (!isfinite(result)) fail(expression, "real result is not finite");
        return realValue(result);
    }

    if (left.kind == V_STRING && right.kind == V_STRING) {
        if (operation == T_PLUS) return stringValue(left.text + right.text);
        if (operation == T_EQUAL || operation == T_NOT_EQUAL)
            return boolValue(compare(operation, left.text, right.text));
    }

    if (left.kind == V_BOOL && right.kind == V_BOOL) {
        switch (operation) {
        case T_AND: return boolValue(left.boolean && right.boolean);
        case T_OR: return boolValue(left.boolean || right.boolean);
        case T_XOR: return boolValue(left.boolean != right.boolean);
        case T_EQUAL: return boolValue(left.boolean == right.boolean);
        case T_NOT_EQUAL: return boolValue(left.boolean != right.boolean);
        default: break;
        }
    }

    if (operation == T_PLUS && left.kind == V_ARRAY && right.kind == V_ARRAY) {
        Value result = newArray();
        long long key = 1;
        for (const auto& element : left.array->elements)
            result.array->elements[key++] = element.second;
        for (const auto& element : right.array->elements)
            result.array->elements[key++] = element.second;
        return result;
    }

    if (operation == T_PLUS && left.kind == V_TUPLE && right.kind == V_TUPLE) {
        Value result = newTuple();
        set<string> names;
        auto append = [&](const TupleValue* tuple) {
            for (size_t index = 0; index < tuple->elements.size(); ++index) {
                const string& name = tuple->names[index];
                if (!name.empty() && !names.insert(name).second)
                    fail(expression, "duplicate tuple field '" + name + "'");
                result.tuple->names.push_back(name);
                result.tuple->elements.push_back(tuple->elements[index]);
            }
        };
        append(left.tuple);
        append(right.tuple);
        return result;
    }

    fail(expression, "invalid operand types for operator '" +
                     tokenName(operation) + "'");
}
