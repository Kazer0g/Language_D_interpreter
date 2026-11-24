// AST interpreter. Included after runtime.cpp.
#include <climits>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>

struct Environment;

struct FunctionValue {
    const Expr* expression = nullptr;
    std::shared_ptr<Environment> closure;
};

struct Environment {
    std::shared_ptr<Environment> parent;
    map<string, Value> values;
};

struct Interpreter {
    explicit Interpreter(ostream& destination)
        : output(destination), global(make_shared<Environment>()) {}

    void run(const vector<Stmt*>& program) {
        execute(program, global);
    }

private:
    struct Flow {
        enum Kind { NORMAL, BREAK, RETURN } kind = NORMAL;
        Value value;
    };

    struct Location {
        Value* slot = nullptr;
        map<long long, Value>* elements = nullptr;
        long long key = 0;
    };

    ostream& output;
    Runtime runtime;
    shared_ptr<Environment> global;
    vector<unique_ptr<FunctionValue>> functions;

    [[noreturn]] void fail(int line, int column, const string& message) const {
        throw runtime_error("Runtime error at " + to_string(line) + ":" +
                            to_string(column) + ": " + message);
    }

    Value* findVariable(const shared_ptr<Environment>& environment,
                        const string& name) const {
        for (auto scope = environment; scope; scope = scope->parent) {
            auto found = scope->values.find(name);
            if (found != scope->values.end()) return &found->second;
        }
        return nullptr;
    }

    Value readVariable(const shared_ptr<Environment>& environment,
                       const string& name, const Expr* expression) const {
        Value* value = findVariable(environment, name);
        if (!value) fail(expression->line, expression->column,
                         "undeclared name '" + name + "'");
        return *value;
    }

    bool requireBoolean(const Value& value, int line, int column) const {
        if (value.kind != V_BOOL)
            fail(line, column, "condition must be boolean");
        return value.boolean;
    }

    Value evaluate(const Expr* expression,
                   const shared_ptr<Environment>& environment) {
        switch (expression->kind) {
        case E_INT:
            return intValue(expression->intValue);
        case E_REAL:
            return realValue(expression->realValue);
        case E_STRING:
            return stringValue(expression->text);
        case E_BOOL:
            return boolValue(expression->boolValue);
        case E_NONE:
            return Value{};
        case E_NAME:
            return readVariable(environment, expression->text, expression);
        case E_ARRAY:
            return evaluateArray(expression, environment);
        case E_TUPLE:
            return evaluateTuple(expression, environment);
        case E_FUNC:
            return evaluateFunction(expression, environment);
        case E_INDEX:
            return readArrayElement(expression, environment);
        case E_FIELD:
        case E_POSITION:
            return *locate(expression, environment).slot;
        case E_CALL:
            return evaluateCall(expression, environment);
        case E_UNARY: {
            Value operand = evaluate(expression->operand, environment);
            return runtime.unary(expression, operand);
        }
        case E_BINARY: {
            // Evaluate both operands, including and/or, from left to right.
            Value left = evaluate(expression->left, environment);
            Value right = evaluate(expression->right, environment);
            return runtime.binary(expression, left, right);
        }
        case E_IS: {
            Value operand = evaluate(expression->operand, environment);
            return boolValue(typeName(operand) == expression->text);
        }
        }
        fail(expression->line, expression->column, "unknown expression");
    }

    Value evaluateArray(const Expr* expression,
                        const shared_ptr<Environment>& environment) {
        Value array = runtime.newArray();
        long long key = 1;
        for (const Expr* item : expression->items)
            array.array->elements[key++] = evaluate(item, environment);
        return array;
    }

    Value evaluateTuple(const Expr* expression,
                        const shared_ptr<Environment>& environment) {
        Value tuple = runtime.newTuple();
        set<string> names;
        for (size_t index = 0; index < expression->items.size(); ++index) {
            const string name = index < expression->names.size()
                                    ? expression->names[index] : "";
            if (!name.empty() && !names.insert(name).second)
                fail(expression->line, expression->column,
                     "duplicate tuple field '" + name + "'");
            Value item = evaluate(expression->items[index], environment);
            tuple.tuple->names.push_back(name);
            tuple.tuple->elements.push_back(item);
        }
        return tuple;
    }

    Value evaluateFunction(const Expr* expression,
                           const shared_ptr<Environment>& environment) {
        set<string> names;
        for (const string& name : expression->names) {
            if (!names.insert(name).second)
                fail(expression->line, expression->column,
                     "duplicate function parameter '" + name + "'");
        }
        auto function = make_unique<FunctionValue>();
        function->expression = expression;
        function->closure = environment;
        Value value;
        value.kind = V_FUNC;
        value.function = function.get();
        functions.push_back(move(function));
        return value;
    }

    Location locate(const Expr* expression,
                    const shared_ptr<Environment>& environment) {
        if (expression->kind == E_NAME) {
            Value* slot = findVariable(environment, expression->text);
            if (!slot) fail(expression->line, expression->column,
                             "undeclared name '" + expression->text + "'");
            return {slot, nullptr, 0};
        }
        if (expression->kind == E_INDEX) {
            Value base = evaluate(expression->target, environment);
            Value index = evaluate(expression->index, environment);
            if (base.kind != V_ARRAY)
                fail(expression->line, expression->column,
                     "index target must be array");
            if (index.kind != V_INT)
                fail(expression->index->line, expression->index->column,
                     "array index must be integer");
            return {nullptr, &base.array->elements, index.integer};
        }
        if (expression->kind == E_FIELD || expression->kind == E_POSITION)
            return locateTupleElement(expression, environment);
        fail(expression->line, expression->column, "invalid assignment target");
    }

    Location locateTupleElement(const Expr* expression,
                                const shared_ptr<Environment>& environment) {
        Value tuple = evaluate(expression->target, environment);
        if (tuple.kind != V_TUPLE)
            fail(expression->line, expression->column,
                 "field target must be tuple");
        size_t position = tuple.tuple->elements.size();
        if (expression->kind == E_FIELD) {
            auto found = std::find(tuple.tuple->names.begin(),
                                   tuple.tuple->names.end(), expression->text);
            if (found == tuple.tuple->names.end())
                fail(expression->line, expression->column,
                     "tuple has no field '" + expression->text + "'");
            position = static_cast<size_t>(found - tuple.tuple->names.begin());
        } else {
            if (expression->intValue < 1 ||
                static_cast<unsigned long long>(expression->intValue) > position)
                fail(expression->line, expression->column,
                     "tuple position out of range");
            position = static_cast<size_t>(expression->intValue - 1);
        }
        return {&tuple.tuple->elements[position], nullptr, 0};
    }

    Value readArrayElement(const Expr* expression,
                           const shared_ptr<Environment>& environment) {
        Location location = locate(expression, environment);
        auto found = location.elements->find(location.key);
        return found == location.elements->end() ? Value{} : found->second;
    }

    Value evaluateCall(const Expr* expression,
                       const shared_ptr<Environment>& environment) {
        Value callee = evaluate(expression->target, environment);
        if (callee.kind != V_FUNC)
            fail(expression->line, expression->column, "value is not callable");
        vector<Value> arguments;
        for (const Expr* argument : expression->items)
            arguments.push_back(evaluate(argument, environment));
        return call(callee, arguments, expression);
    }

    Value call(const Value& callee, const vector<Value>& arguments,
               const Expr* callExpression) {
        const Expr* function = callee.function->expression;
        if (arguments.size() != function->names.size())
            fail(callExpression->line, callExpression->column,
                 "function argument count mismatch");
        auto callScope = make_shared<Environment>();
        callScope->parent = callee.function->closure;
        for (size_t index = 0; index < arguments.size(); ++index)
            callScope->values[function->names[index]] = arguments[index];
        if (function->bodyExpr)
            return evaluate(function->bodyExpr, callScope);
        Flow result = execute(function->body, callScope);
        return result.kind == Flow::RETURN ? result.value : Value{};
    }

    Flow execute(const vector<Stmt*>& statements,
                 const shared_ptr<Environment>& environment) {
        for (const Stmt* statement : statements) {
            Flow result = executeStatement(statement, environment);
            if (result.kind != Flow::NORMAL) return result;
        }
        return {};
    }

    Flow executeStatement(const Stmt* statement,
                          const shared_ptr<Environment>& environment) {
        switch (statement->kind) {
        case S_VAR:
            declareVariables(statement, environment);
            return {};
        case S_ASSIGN:
            assign(statement, environment);
            return {};
        case S_IF:
            return executeIf(statement, environment);
        case S_WHILE:
            return executeWhile(statement, environment);
        case S_FOR:
            return executeFor(statement, environment);
        case S_LOOP:
            return executeLoop(statement, environment);
        case S_EXIT:
            return {Flow::BREAK, Value{}};
        case S_RETURN:
            return {Flow::RETURN, statement->value
                                      ? evaluate(statement->value, environment)
                                      : Value{}};
        case S_PRINT:
            print(statement, environment);
            return {};
        case S_CALL:
            evaluate(statement->value, environment);
            return {};
        }
        return {};
    }

    void declareVariables(const Stmt* statement,
                          const shared_ptr<Environment>& environment) {
        for (size_t index = 0; index < statement->names.size(); ++index) {
            const string& name = statement->names[index];
            if (environment->values.count(name))
                fail(statement->line, statement->column,
                     "redeclaration of '" + name + "'");
            Value value = index < statement->values.size() && statement->values[index]
                              ? evaluate(statement->values[index], environment)
                              : Value{};
            environment->values[name] = value;
        }
    }

    void assign(const Stmt* statement,
                const shared_ptr<Environment>& environment) {
        Location destination = locate(statement->target, environment);
        Value value = evaluate(statement->value, environment);
        if (destination.slot) *destination.slot = value;
        else (*destination.elements)[destination.key] = value;
    }

    Flow executeIf(const Stmt* statement,
                   const shared_ptr<Environment>& environment) {
        Value condition = evaluate(statement->cond, environment);
        bool takeThen = requireBoolean(condition, statement->cond->line,
                                       statement->cond->column);
        auto branchScope = make_shared<Environment>();
        branchScope->parent = environment;
        return execute(takeThen ? statement->thenBody : statement->elseBody,
                       branchScope);
    }

    Flow executeWhile(const Stmt* statement,
                      const shared_ptr<Environment>& environment) {
        while (true) {
            Value condition = evaluate(statement->cond, environment);
            if (!requireBoolean(condition, statement->cond->line,
                                statement->cond->column)) break;
            Flow result = executeOneIteration(statement, environment, nullptr);
            if (result.kind == Flow::RETURN) return result;
            if (result.kind == Flow::BREAK) break;
        }
        return {};
    }

    Flow executeLoop(const Stmt* statement,
                     const shared_ptr<Environment>& environment) {
        while (true) {
            Flow result = executeOneIteration(statement, environment, nullptr);
            if (result.kind == Flow::RETURN) return result;
            if (result.kind == Flow::BREAK) break;
        }
        return {};
    }

    Flow executeFor(const Stmt* statement,
                    const shared_ptr<Environment>& environment) {
        Value first = evaluate(statement->from, environment);
        if (statement->to)
            return executeRange(statement, environment, first);
        return executeAggregate(statement, environment, first);
    }

    Flow executeRange(const Stmt* statement,
                      const shared_ptr<Environment>& environment,
                      const Value& first) {
        Value last = evaluate(statement->to, environment);
        if (first.kind != V_INT || last.kind != V_INT)
            fail(statement->line, statement->column,
                 "range bounds must be integers");
        for (long long value = first.integer; value <= last.integer; ++value) {
            Value loopValue = intValue(value);
            Flow result = executeOneIteration(statement, environment, &loopValue);
            if (result.kind == Flow::RETURN) return result;
            if (result.kind == Flow::BREAK) break;
            if (value == LLONG_MAX) break;
        }
        return {};
    }

    Flow executeAggregate(const Stmt* statement,
                         const shared_ptr<Environment>& environment,
                         const Value& aggregate) {
        if (aggregate.kind == V_ARRAY) {
            vector<long long> keys;
            for (const auto& element : aggregate.array->elements)
                keys.push_back(element.first);
            for (long long key : keys) {
                auto found = aggregate.array->elements.find(key);
                Value loopValue = found->second;
                Flow result = executeOneIteration(statement, environment, &loopValue);
                if (result.kind == Flow::RETURN) return result;
                if (result.kind == Flow::BREAK) break;
            }
            return {};
        }
        if (aggregate.kind == V_TUPLE) {
            const size_t count = aggregate.tuple->elements.size();
            for (size_t index = 0; index < count; ++index) {
                Value loopValue = aggregate.tuple->elements[index];
                Flow result = executeOneIteration(statement, environment, &loopValue);
                if (result.kind == Flow::RETURN) return result;
                if (result.kind == Flow::BREAK) break;
            }
            return {};
        }
        fail(statement->line, statement->column,
             "for value must be array, tuple, or integer range");
    }

    Flow executeOneIteration(const Stmt* statement,
                             const shared_ptr<Environment>& parent,
                             const Value* loopValue) {
        auto bodyScope = make_shared<Environment>();
        bodyScope->parent = parent;
        if (loopValue && !statement->loopVar.empty())
            bodyScope->values[statement->loopVar] = *loopValue;
        return execute(statement->body, bodyScope);
    }

    void print(const Stmt* statement,
               const shared_ptr<Environment>& environment) {
        vector<Value> values;
        for (const Expr* expression : statement->values) {
            values.push_back(evaluate(expression, environment));
        }
        vector<string> rendered;
        for (const Value& value : values) rendered.push_back(runtime.render(value));
        for (size_t index = 0; index < rendered.size(); ++index) {
            if (index) output << ' ';
            output << rendered[index];
        }
        output << '\n';
    }
};
