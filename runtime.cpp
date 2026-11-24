// Runtime values and aggregate storage for Language D.
#include <charconv>
#include <deque>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_set>
#include <cmath>

struct Expr;
struct FunctionValue;
struct ArrayValue;
struct TupleValue;

enum ValueKind { V_NONE, V_INT, V_REAL, V_BOOL, V_STRING, V_ARRAY, V_TUPLE, V_FUNC };

struct Value {
    ValueKind kind = V_NONE;
    long long integer = 0;
    double real = 0.0;
    bool boolean = false;
    string text;
    ArrayValue* array = nullptr;
    TupleValue* tuple = nullptr;
    FunctionValue* function = nullptr;
};

struct ArrayValue {
    map<long long, Value> elements;
};

struct TupleValue {
    vector<string> names;
    vector<Value> elements;
};

Value intValue(long long value) {
    Value result;
    result.kind = V_INT;
    result.integer = value;
    return result;
}

Value realValue(double value) {
    Value result;
    result.kind = V_REAL;
    result.real = value;
    return result;
}

Value boolValue(bool value) {
    Value result;
    result.kind = V_BOOL;
    result.boolean = value;
    return result;
}

Value stringValue(const string& value) {
    Value result;
    result.kind = V_STRING;
    result.text = value;
    return result;
}

string typeName(const Value& value) {
    switch (value.kind) {
        case V_NONE: return "none";
        case V_INT: return "int";
        case V_REAL: return "real";
        case V_BOOL: return "bool";
        case V_STRING: return "string";
        case V_ARRAY: return "[]";
        case V_TUPLE: return "{}";
        case V_FUNC: return "func";
    }
    return "none";
}

// Expand scientific notation while preserving the shortest round-trip digits.
static string decimalWithDot(double value) {
    if (value == 0.0) return signbit(value) ? "-0.0" : "0.0";

    char buffer[128];
    auto converted = to_chars(buffer, buffer + sizeof(buffer), value,
                              chars_format::general);
    if (converted.ec != errc()) {
        throw runtime_error("could not format real value");
    }
    string raw(buffer, converted.ptr);
    size_t exponentAt = raw.find_first_of("eE");
    string mantissa = raw.substr(0, exponentAt);
    int exponent = 0;
    if (exponentAt != string::npos) {
        exponent = stoi(raw.substr(exponentAt + 1));
    }

    bool negative = !mantissa.empty() && mantissa[0] == '-';
    size_t start = negative ? 1 : 0;
    size_t dotAt = mantissa.find('.', start);
    int originalPoint = static_cast<int>(dotAt == string::npos ? mantissa.size() : dotAt) -
                        static_cast<int>(start);
    string digits;
    for (size_t i = start; i < mantissa.size(); ++i) {
        if (mantissa[i] != '.') digits.push_back(mantissa[i]);
    }
    int point = originalPoint + exponent;

    string expanded;
    if (negative) expanded.push_back('-');
    if (point <= 0) {
        expanded += "0.";
        expanded.append(static_cast<size_t>(-point), '0');
        expanded += digits;
    } else if (point >= static_cast<int>(digits.size())) {
        expanded += digits;
        expanded.append(static_cast<size_t>(point - static_cast<int>(digits.size())), '0');
        expanded += ".0";
    } else {
        expanded.append(digits, 0, static_cast<size_t>(point));
        expanded.push_back('.');
        expanded.append(digits, static_cast<size_t>(point), string::npos);
    }
    return expanded;
}

struct Runtime {
    deque<ArrayValue> arrays;
    deque<TupleValue> tuples;

    Value newArray() {
        arrays.emplace_back();
        Value result;
        result.kind = V_ARRAY;
        result.array = &arrays.back();
        return result;
    }

    Value newTuple() {
        tuples.emplace_back();
        Value result;
        result.kind = V_TUPLE;
        result.tuple = &tuples.back();
        return result;
    }

    [[noreturn]] void fail(const Expr* expression, const string& message) const {
        int line = expression ? expression->line : 0;
        int column = expression ? expression->column : 0;
        throw runtime_error("Runtime error at " + to_string(line) + ":" +
                            to_string(column) + ": " + message);
    }
    Value unary(const Expr* expression, const Value& operand);
    Value binary(const Expr* expression, const Value& left, const Value& right);

    string render(const Value& value) const {
        unordered_set<const void*> active;
        return renderValue(value, active);
    }

private:
    string renderValue(const Value& value, unordered_set<const void*>& active) const {
        switch (value.kind) {
            case V_NONE: return "none";
            case V_INT: return to_string(value.integer);
            case V_REAL: return decimalWithDot(value.real);
            case V_BOOL: return value.boolean ? "true" : "false";
            case V_STRING: return value.text;
            case V_FUNC: return "func";
            case V_ARRAY: {
                if (!value.array) return "[]";
                if (!active.insert(value.array).second) return "[...]";
                string result = "[";
                bool first = true;
                for (const auto& item : value.array->elements) {
                    if (!first) result += ", ";
                    result += renderValue(item.second, active);
                    first = false;
                }
                result += "]";
                active.erase(value.array);
                return result;
            }
            case V_TUPLE: {
                if (!value.tuple) return "{}";
                if (!active.insert(value.tuple).second) return "{...}";
                string result = "{";
                for (size_t i = 0; i < value.tuple->elements.size(); ++i) {
                    if (i != 0) result += ", ";
                    if (i < value.tuple->names.size() && !value.tuple->names[i].empty()) {
                        result += value.tuple->names[i] + " := ";
                    }
                    result += renderValue(value.tuple->elements[i], active);
                }
                result += "}";
                active.erase(value.tuple);
                return result;
            }
        }
        return "none";
    }
};

#include "operators.cpp"
