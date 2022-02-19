#include "parser.h"
#include "scheme.h"
#include <iostream>

std::map<std::string, std::shared_ptr<Object>> m;

Interpreter::Interpreter() {
    m.clear();
    m["quote"] = std::shared_ptr<Quote>(new Quote());
    m["number?"] = std::shared_ptr<CheckForNumber>(new CheckForNumber());
    m["="] = std::shared_ptr<Equal>(new Equal());
    m[">"] = std::shared_ptr<Greater>(new Greater());
    m["<"] = std::shared_ptr<Less>(new Less());
    m[">="] = std::shared_ptr<GreaterOrEqual>(new GreaterOrEqual());
    m["<="] = std::shared_ptr<LessOrEqual>(new LessOrEqual());
    m["+"] = std::shared_ptr<Sum>(new Sum());
    m["-"] = std::shared_ptr<Subtraction>(new Subtraction());
    m["*"] = std::shared_ptr<Multiplication>(new Multiplication());
    m["/"] = std::shared_ptr<Division>(new Division());
    m["max"] = std::shared_ptr<Max>(new Max());
    m["min"] = std::shared_ptr<Min>(new Min());
    m["abs"] = std::shared_ptr<Abs>(new Abs());
    m["boolean?"] = std::shared_ptr<CheckForBoolean>(new CheckForBoolean());
    m["not"] = std::shared_ptr<Not>(new Not());
    m["and"] = std::shared_ptr<And>(new And());
    m["or"] = std::shared_ptr<Or>(new Or());
    m["pair?"] = std::shared_ptr<Pair>(new Pair());
    m["null?"] = std::shared_ptr<Null>(new Null());
    m["list?"] = std::shared_ptr<List>(new List());
    m["cdr"] = std::shared_ptr<Cdr>(new Cdr());
    m["car"] = std::shared_ptr<Car>(new Car());
    m["cons"] = std::shared_ptr<Cons>(new Cons());
    m["list"] = std::shared_ptr<Quote>(new Quote());
    m["list-ref"] = std::shared_ptr<ListRef>(new ListRef());
    m["list-tail"] = std::shared_ptr<ListTail>(new ListTail());
    m["if"] = std::shared_ptr<If>(new If());
    m["define"] = std::shared_ptr<Define>(new Define());
    m["symbol?"] = std::shared_ptr<IsSymbol>(new IsSymbol());
    m["set!"] = std::shared_ptr<Set>(new Set());
    m["set-car!"] = std::shared_ptr<SetCar>(new SetCar());
}

std::string Interpreter::Run(std::string str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};
    auto obj = Read(&tokenizer);
    if (!tokenizer.IsEnd()) { throw SyntaxError("SyntaxError"); }
    if (obj == nullptr) {
        throw RuntimeError("RuntimeError");
    }

    auto cur = UnbindFunc(obj);
    if (cur == nullptr) {
        return "()";
    }
    return As<Symbol>(cur->Eval())->GetName();
}
