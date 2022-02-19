#pragma once

#include <memory>
#include <string>
#include <vector>
#include "error.h"
#include "tokenizer.h"
#include <map>
#include <iostream>
#include <sstream>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual std::shared_ptr<Object> Eval() = 0;
    virtual ~Object() = default;
};

extern std::map<std::string, std::shared_ptr<Object>> m;

std::string StringFromCell(std::shared_ptr<Object> obj);
std::shared_ptr<Object> MakeSymbol(std::shared_ptr<Object> obj);
std::shared_ptr<Object> MakeCell(std::shared_ptr<Object> first, std::shared_ptr<Object> second);
void CallOnEmpty(std::shared_ptr<Object> obj);
int TreeLength(std::shared_ptr<Object> obj);
std::shared_ptr<Object> LastInTree(std::shared_ptr<Object> obj);
std::shared_ptr<Object> LastInTreeNonNull(std::shared_ptr<Object> obj);
std::shared_ptr<Object> PosInTree(std::shared_ptr<Object> obj, int pos);
std::shared_ptr<Object> AfterPosInTree(std::shared_ptr<Object> obj, int pos);
std::shared_ptr<Object> UnbindFunc(std::shared_ptr<Object> obj);
void UnbindList(std::vector<std::shared_ptr<Object>>& objects, std::shared_ptr<Object> obj);
void CompareSzEq(std::size_t true_sz, std::size_t given_sz);
void CompareSzNeq(std::size_t true_sz, std::size_t given_sz);

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return dynamic_cast<T*>(obj.get()) != nullptr;
}

template <class T>
void IsType(const std::shared_ptr<Object>& obj) {
    if (!Is<T>(obj)) {
        throw RuntimeError("RuntimeError");
    }
}

template <class T>
void IsTypeSyntax(const std::shared_ptr<Object>& obj) {
    if (!Is<T>(obj)) {
        throw SyntaxError("SyntaxError");
    }
}

template <class T>
void IsNoType(const std::shared_ptr<Object>& obj) {
    if (Is<T>(obj)) {
        throw RuntimeError("RuntimeError");
    }
}

template <class T>
void AreTypesCorrect(std::vector<std::shared_ptr<Object>>& objects) {
    for (std::size_t i = 0; i < objects.size(); ++i) {
        if (objects[i] == nullptr || (!Is<T>(objects[i]) && !Is<T>(objects[i]->Eval()))) {
            throw RuntimeError("RuntimeError");
        }
        if (Is<T>(objects[i]->Eval())) {
            objects[i] = objects[i]->Eval();
        }
    }
}

class Symbol : public Object {
private:
    std::string name_;
public:
    std::shared_ptr<Object> Eval() override {
        if (m.find(name_) == m.end()) {
            return std::shared_ptr<Symbol>(new Symbol(name_));
        }
        return m[name_];
    }
    Symbol(std::string str) : name_(str) {
    }
    const std::string& GetName() const {
        return name_;
    }
};

class Number : public Object {
    int value_;
public:
    std::shared_ptr<Object> Eval() override {
        return std::shared_ptr<Symbol>(new Symbol(std::to_string(value_)));
    }
    Number(int num) : value_(num) {
    }
    int GetValue() const {
        return value_;
    }
};

class Boolean : public Object {
    std::string name_;
public:
    std::shared_ptr<Object> Eval() override {
        return std::shared_ptr<Symbol>(new Symbol(name_));
    }
    Boolean(std::string str) : name_(str) {
    }
    std::string GetValue() const {
        return name_;
    }
};

class Cell : public Object {
private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
public:
    std::shared_ptr<Object> Eval() override {
        std::string ans = "(";
        ans += StringFromCell(std::shared_ptr<Cell>(new Cell(first_, second_)));
        ans[ans.size() - 1] = ')';
        return std::shared_ptr<Symbol>(new Symbol(ans));
    }
    Cell() : first_(nullptr), second_(nullptr) {
    }
    Cell(std::shared_ptr<Object> first, std::shared_ptr<Object> second) : first_(first), second_(second) {
    }
    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }
    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }
};

class Function : public Object {
public:
    std::shared_ptr<Object> Eval() final { throw RuntimeError("RuntimeError"); }
    virtual std::shared_ptr<Object> Apply(std::shared_ptr<Object>) = 0;
};

class Quote : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        return obj;
    }
};

class CheckForNumber : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        if (!Is<Number>(objects[0])) {
            return std::shared_ptr<Boolean>(new Boolean("#f"));
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class Equal : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        AreTypesCorrect<Number>(objects);
        for (size_t i = 1; i < objects.size(); ++i) {
            if (As<Number>(objects[i])->GetValue() != As<Number>(objects[i - 1])->GetValue()) {
                return std::shared_ptr<Boolean>(new Boolean("#f"));
            }
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class Greater : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        AreTypesCorrect<Number>(objects);
        for (size_t i = 1; i < objects.size(); ++i) {
            if (As<Number>(objects[i])->GetValue() >= As<Number>(objects[i - 1])->GetValue()) {
                return std::shared_ptr<Boolean>(new Boolean("#f"));
            }
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class Less : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        AreTypesCorrect<Number>(objects);
        for (size_t i = 1; i < objects.size(); ++i) {
            if (As<Number>(objects[i])->GetValue() <= As<Number>(objects[i - 1])->GetValue()) {
                return std::shared_ptr<Boolean>(new Boolean("#f"));
            }
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class GreaterOrEqual : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        AreTypesCorrect<Number>(objects);
        for (size_t i = 1; i < objects.size(); ++i) {
            if (As<Number>(objects[i])->GetValue() > As<Number>(objects[i - 1])->GetValue()) {
                return std::shared_ptr<Boolean>(new Boolean("#f"));
            }
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class LessOrEqual : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        AreTypesCorrect<Number>(objects);
        for (size_t i = 1; i < objects.size(); ++i) {
            if (As<Number>(objects[i])->GetValue() < As<Number>(objects[i - 1])->GetValue()) {
                return std::shared_ptr<Boolean>(new Boolean("#f"));
            }
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class Sum : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        AreTypesCorrect<Number>(objects);
        int64_t res = 0;
        for (auto object : objects) {
            res += As<Number>(object)->GetValue();
        }
        return std::shared_ptr<Number>(new Number(res));
    }
};

class Multiplication : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        AreTypesCorrect<Number>(objects);
        int64_t res = 1;
        for (auto object : objects) {
            res *= As<Number>(object)->GetValue();
        }
        return std::shared_ptr<Number>(new Number(res));
    }
};

class Subtraction : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzNeq(0, objects.size());
        AreTypesCorrect<Number>(objects);
        int64_t res = As<Number>(objects[0])->GetValue();
        for (size_t i = 1; i < objects.size(); ++i) {
            res -= As<Number>(objects[i])->GetValue();
        }
        return std::shared_ptr<Number>(new Number(res));
    }
};

class Division : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzNeq(0, objects.size());
        AreTypesCorrect<Number>(objects);
        int64_t res = As<Number>(objects[0])->GetValue();
        for (size_t i = 1; i < objects.size(); ++i) {
            res /= As<Number>(objects[i])->GetValue();
        }
        return std::shared_ptr<Number>(new Number(res));
    }
};

class Max : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzNeq(0, objects.size());
        AreTypesCorrect<Number>(objects);
        int res = As<Number>(objects[0])->GetValue();
        for (size_t i = 1; i < objects.size(); ++i) {
            res = std::max(res, As<Number>(objects[i])->GetValue());
        }
        return std::shared_ptr<Number>(new Number(res));
    }
};

class Min : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzNeq(0, objects.size());
        AreTypesCorrect<Number>(objects);
        int res = As<Number>(objects[0])->GetValue();
        for (size_t i = 1; i < objects.size(); ++i) {
            res = std::min(res, As<Number>(objects[i])->GetValue());
        }
        return std::shared_ptr<Number>(new Number(res));
    }
};

class Abs : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        AreTypesCorrect<Number>(objects);
        return std::shared_ptr<Number>(new Number(abs(As<Number>(objects[0])->GetValue())));
    }
};

class CheckForBoolean : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        if (Is<Boolean>(objects[0])) {
            return std::shared_ptr<Boolean>(new Boolean("#t"));
        } else {
            return std::shared_ptr<Boolean>(new Boolean("#f"));
        }
    }
};

class Not : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        if (Is<Boolean>(objects[0]) && As<Boolean>(objects[0])->GetValue() == "#f") {
            return std::shared_ptr<Boolean>(new Boolean("#t"));
        } else {
            return std::shared_ptr<Boolean>(new Boolean("#f"));
        }
    }
};

std::shared_ptr<Object> UnbindForBoolean(std::shared_ptr<Object> obj, std::shared_ptr<Boolean> value);

class And : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        if (obj == nullptr) {
            return std::shared_ptr<Boolean>(new Boolean("#t"));
        }
        return UnbindForBoolean(obj, std::shared_ptr<Boolean>(new Boolean("#f")));
    }
};

class Or : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        if (obj == nullptr) {
            return std::shared_ptr<Boolean>(new Boolean("#f"));
        }
        return UnbindForBoolean(obj, std::shared_ptr<Boolean>(new Boolean("#t")));
    }
};

class Pair : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        std::cout << objects.size() << std::endl;
        CompareSzEq(1, objects.size());
        if (TreeLength(objects[0]) != 2) {
            return std::shared_ptr<Boolean>(new Boolean("#f"));
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class Null : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        if (objects[0] != nullptr) {
            return std::shared_ptr<Boolean>(new Boolean("#f"));
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class List : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        if (LastInTree(objects[0]) != nullptr) {
            return std::shared_ptr<Boolean>(new Boolean("#f"));
        }
        return std::shared_ptr<Boolean>(new Boolean("#t"));
    }
};

class Car : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        CallOnEmpty(objects[0]);
        IsType<Cell>(objects[0]);
        return As<Cell>(objects[0])->GetFirst();
    }
};

class Cdr : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        CallOnEmpty(objects[0]);
        IsType<Cell>(objects[0]);
        return As<Cell>(objects[0])->GetSecond();
    }
};

class Cons : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(2, objects.size());
        return std::shared_ptr<Cell>(new Cell(objects[0], objects[1]));
    }
};

class ListRef : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(2, objects.size());
        IsType<Number>(objects[1]);
        return PosInTree(objects[0], As<Number>(objects[1])->GetValue());
    }
};

class ListTail : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(2, objects.size());
        IsType<Number>(objects[1]);
        return AfterPosInTree(objects[0], As<Number>(objects[1])->GetValue());
    }
};

class If : public Function {
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        if (TreeLength(obj) < 1 || TreeLength(obj) > 3) {
            throw SyntaxError("SynyaxError");
        }
        auto st = Is<Cell>(As<Cell>(obj)->GetFirst()) ? UnbindFunc(As<Cell>(obj)->GetFirst()) : As<Cell>(obj)->GetFirst();
        if (Is<Boolean>(st) && As<Boolean>(st)->GetValue() == "#f") {
            if (TreeLength(obj) == 2) {
                return UnbindFunc(nullptr);
            }
            return UnbindFunc(LastInTreeNonNull(obj));
        }
        if (TreeLength(obj) == 1) {
            return UnbindFunc(nullptr);
        }
        return UnbindFunc(As<Cell>(As<Cell>(obj)->GetSecond())->GetFirst());
    }
};

void AddValuesToParams(std::map<std::string, std::shared_ptr<Object>>& vars, std::vector<std::string>& names,
                          std::vector<std::shared_ptr<Object>>& objects);
std::shared_ptr<Object> SearchInAncestors(std::string s, std::shared_ptr<Object> env);

void SetParams(std::shared_ptr<Object> env, std::shared_ptr<Object> obj);

class Lambda : public Function {
public:
    std::shared_ptr<Object> parent;
    std::map<std::string, std::shared_ptr<Object>> names;
    std::vector<std::shared_ptr<Object>> params;
    std::shared_ptr<Object> body;

    Lambda(std::vector<std::shared_ptr<Object>>& new_params, std::shared_ptr<Object> new_body) :
          body(new_body) {
        params = new_params;
        for (auto param : new_params) {
            names[As<Symbol>(param)->GetName()] = nullptr;
        }
    };

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        return nullptr;
    }
};

class MakeLambda : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        IsType<Cell>(obj);
        IsType<Cell>(As<Cell>(obj)->GetFirst());
        IsType<Cell>(As<Cell>(obj)->GetSecond());
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, As<Cell>(obj)->GetFirst());
        AreTypesCorrect<Symbol>(objects);
        auto new_lambda = std::shared_ptr<Lambda>(new Lambda(objects, As<Cell>(obj)->GetSecond()));
        As<Lambda>(new_lambda)->parent = nullptr;
        return new_lambda;
    }
};

class Define : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        IsTypeSyntax<Cell>(obj);
        auto first = As<Cell>(obj)->GetFirst();
        auto second = As<Cell>(obj)->GetSecond();
        IsTypeSyntax<Cell>(second);
        if (As<Cell>(second)->GetSecond() != nullptr) {
            throw SyntaxError("SyntaxError");
        }
        if (Is<Symbol>(first)) {
            if (!Is<Cell>(As<Cell>(second)->GetFirst())) {
                if (Is<Number>(As<Cell>(second)->GetFirst())) {
                    m[As<Symbol>(first)->GetName()] = As<Cell>(second)->GetFirst();
                }
                if (Is<Symbol>(As<Cell>(second)->GetFirst())) {
                    if (m.find(As<Symbol>(As<Cell>(second)->GetFirst())->GetName()) == m.end()) {
                        throw NameError("NameError");
                    }
                    m[As<Symbol>(first)->GetName()] = std::shared_ptr<Number>(new Number(
                        As<Number>(m[As<Symbol>(As<Cell>(second)->GetFirst())->GetName()])->GetValue()));
                }
            } else {
                auto s_f = As<Cell>(second)->GetFirst();
                auto func = As<Cell>(s_f)->GetFirst()->Eval();
                IsTypeSyntax<Function>(func);
                if (!Is<Lambda>(func) && !Is<Define>(func)) {
                    m[As<Symbol>(first)->GetName()] = As<Function>(func)->Apply(As<Cell>(s_f)->GetSecond());
                }
            }
        }
        return nullptr;
    }
};

class Set : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        IsTypeSyntax<Cell>(obj);
        auto first = As<Cell>(obj)->GetFirst();
        auto second = As<Cell>(obj)->GetSecond();
        IsTypeSyntax<Symbol>(first);
        IsTypeSyntax<Cell>(second);
        if (As<Cell>(second)->GetSecond() != nullptr) {
            throw SyntaxError("SyntaxError");
        }
        if (m.find(As<Symbol>(first)->GetName()) == m.end()) {
            throw NameError("NameError");
        }
        if (!Is<Cell>(As<Cell>(second)->GetFirst())) {
            m[As<Symbol>(first)->GetName()] = As<Cell>(second)->GetFirst();
        } else {
            auto s_f = As<Cell>(second)->GetFirst();
            auto func = As<Cell>(s_f)->GetFirst()->Eval();
            IsTypeSyntax<Function>(func);
            if (!Is<Lambda>(func) && !Is<Define>(func)) {
                m[As<Symbol>(first)->GetName()] = As<Function>(func)->Apply(As<Cell>(s_f)->GetSecond());
            }
        }
        return nullptr;
    }
};

class IsSymbol : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        IsType<Cell>(obj);
        std::vector<std::shared_ptr<Object>> objects;
        UnbindList(objects, obj);
        CompareSzEq(1, objects.size());
        if (Is<Symbol>(objects[0])) {
            return std::shared_ptr<Boolean>(new Boolean("#t"));
        }
        return std::shared_ptr<Boolean>(new Boolean("#f"));
    }
};

class SetCar : public Function {
public:
    std::shared_ptr<Object> Apply(std::shared_ptr<Object> obj) override {
        std::cout << 10000 << std::endl;
        IsType<Cell>(obj);
        auto first = As<Cell>(obj)->GetFirst();
        auto second = As<Cell>(obj)->GetSecond();
        std::cout << 1000 << std::endl;
        IsType<Cell>(second);
        std::cout << 2000 << std::endl;
        IsType<Symbol>(first);
        std::cout << 3000 << std::endl;
        auto pair = m[As<Symbol>(first)->GetName()];
        std::cout << m[As<Symbol>(first)->GetName()] << std::endl;
        auto ispair = As<Function>(m["pair?"])->Apply(std::shared_ptr<Cell>(new Cell(std::shared_ptr<Cell>(
            new Cell(std::shared_ptr<Symbol>(new Symbol("quote")), pair)), nullptr)));
        std::cout << 4000 << std::endl;
        if (As<Boolean>(ispair)->GetValue() == "#f") {
            throw RuntimeError("RuntimeError");
        }
        m[As<Symbol>(first)->GetName()] = std::shared_ptr<Cell>(new Cell(As<Cell>(second)->GetFirst(), As<Cell>(pair)->GetSecond()));
        std::swap(As<Cell>(pair)->GetFirst(), As<Cell>(second)->GetFirst());
        std::cout << As<Number>(As<Cell>(second)->GetFirst())->GetValue() << std::endl;
        std::cout << As<Number>(As<Cell>(pair)->GetFirst())->GetValue() << std::endl;
        return nullptr;
    }
};