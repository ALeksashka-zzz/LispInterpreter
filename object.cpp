#include "object.h"

std::string StringFromCell(std::shared_ptr<Object> obj) {
    if (obj == nullptr) {
        return "( ";
    }
    std::string s;
    if (As<Cell>(obj)->GetFirst() != nullptr) {
        if (!Is<Cell>(As<Cell>(obj)->GetFirst())) {
            s += As<Symbol>(MakeSymbol(As<Cell>(obj)->GetFirst()))->GetName();
        } else {
            s += "(";
            s += StringFromCell(As<Cell>(obj)->GetFirst());
            s[s.size() - 1] = ')';
        }
        s += " ";
    } else {
        s += "() ";
    }
    if (As<Cell>(obj)->GetSecond() != nullptr) {
        if (!Is<Cell>(As<Cell>(obj)->GetSecond())) {
            s += ". ";
            s += As<Symbol>(MakeSymbol(As<Cell>(obj)->GetSecond()))->GetName();
            s += " ";
        } else {
            s += StringFromCell(As<Cell>(obj)->GetSecond());
        }

    }
    return s;
}

std::shared_ptr<Object> MakeSymbol(std::shared_ptr<Object> obj) {
    if (Is<Number>(obj)) {
        return std::shared_ptr<Symbol>(new Symbol(std::to_string(As<Number>(obj)->GetValue())));
    }
    if (Is<Boolean>(obj)) {
        return std::shared_ptr<Symbol>(
            new Symbol(As<Boolean>(obj)->GetValue()));
    }
    return obj;
}

std::shared_ptr<Object> MakeCell(std::shared_ptr<Object> first, std::shared_ptr<Object> second) {
    return std::shared_ptr<Cell>(new Cell(first, second));
}

void CallOnEmpty(std::shared_ptr<Object> obj) {
    if (obj == nullptr) {
        throw RuntimeError("RuntimeError");
    }
}

int TreeLength(std::shared_ptr<Object> obj) {
    if (!Is<Cell>(obj) || obj == nullptr) {
        return (obj == nullptr) ? 0 : 1;
    }
    return 1 + TreeLength(As<Cell>(obj)->GetSecond());
}

std::shared_ptr<Object> LastInTree(std::shared_ptr<Object> obj) {
    if (!Is<Cell>(obj) || obj == nullptr) {
        return obj;
    }
    return LastInTree(As<Cell>(obj)->GetSecond());
}

std::shared_ptr<Object> LastInTreeNonNull(std::shared_ptr<Object> obj) {
    if (Is<Cell>(obj) && As<Cell>(obj)->GetSecond() == nullptr) {
        return As<Cell>(obj)->GetFirst();
    }
    return LastInTreeNonNull(As<Cell>(obj)->GetSecond());
}

std::shared_ptr<Object> PosInTree(std::shared_ptr<Object> obj, int pos) {
    if (pos >= 0 && obj == nullptr) {
        throw RuntimeError("RuntimeError");
    }
    if (pos == 0) {
        return As<Cell>(obj)->GetFirst();
    }
    return PosInTree(As<Cell>(obj)->GetSecond(), pos - 1);
}

std::shared_ptr<Object> AfterPosInTree(std::shared_ptr<Object> obj, int pos) {
    if (pos == 0) {
        return obj;
    }
    if (pos > 0 && obj == nullptr) {
        throw RuntimeError("RuntimeError");
    }
    return AfterPosInTree(As<Cell>(obj)->GetSecond(), pos - 1);
}

std::shared_ptr<Object> UnbindFunc(std::shared_ptr<Object> obj) {
    if (Is<Symbol>(obj)) {
        if (m.find(As<Symbol>(obj)->GetName()) != m.end()) {
            return m[As<Symbol>(obj)->GetName()];
        }
        throw NameError("NameError");
    }
    if (!Is<Cell>(obj)) {
        return obj;
    }
    IsType<Cell>(obj);
    CallOnEmpty(As<Cell>(obj)->GetFirst());
    auto func = As<Cell>(obj)->GetFirst()->Eval();
    IsType<Function>(func);
    return As<Function>(func)->Apply(As<Cell>(obj)->GetSecond());
}

void UnbindList(std::vector<std::shared_ptr<Object>>& objects,
                std::shared_ptr<Object> obj) {
    if (obj == nullptr) {
        return;
    }
    if (!Is<Cell>(obj)) {
        objects.push_back(obj);
        return;
    }
    if (!Is<Cell>(As<Cell>(obj)->GetFirst())) {
        std::cout << 3000 << std::endl;
        objects.push_back(As<Cell>(obj)->GetFirst());
    } else {
        std::cout << 2000 << std::endl;
        auto func = As<Cell>(As<Cell>(obj)->GetFirst())->GetFirst()->Eval();
        if (Is<Function>(func)) {
            std::cout << 1000 << std::endl;
            objects.push_back(UnbindFunc(As<Cell>(obj)->GetFirst()));
        } else {
            UnbindList(objects, As<Cell>(obj)->GetFirst());
        }
    }
    UnbindList(objects, As<Cell>(obj)->GetSecond());
}

std::shared_ptr<Object> UnbindForBoolean(std::shared_ptr<Object> obj, std::shared_ptr<Boolean> value) {
    auto first = (Is<Cell>(As<Cell>(obj)->GetFirst())) ? UnbindFunc(As<Cell>(obj)->GetFirst()) : As<Cell>(obj)->GetFirst();
    if (Is<Boolean>(first) && As<Boolean>(first)->GetValue() == value->GetValue()) {
        return std::shared_ptr<Boolean>(new Boolean(value->GetValue()));
    }
    if (As<Cell>(obj)->GetSecond() == nullptr) {
        return first;
    }
    if (!Is<Cell>(As<Cell>(obj)->GetSecond())) {
        return As<Cell>(obj)->GetSecond();
    }
    if (Is<Function>(As<Cell>(As<Cell>(obj)->GetSecond())->GetFirst())) {
        return UnbindFunc(As<Cell>(obj));
    }
    return UnbindForBoolean(As<Cell>(obj)->GetSecond(), value);
}

void CompareSzEq(std::size_t true_sz, std::size_t given_sz) {
    if (true_sz != given_sz) {
        throw RuntimeError("RuntimeError");
    }
}

void CompareSzNeq(std::size_t true_sz, std::size_t given_sz) {
    if (true_sz == given_sz) {
        throw RuntimeError("RuntimeError");
    }
}
/*
void AddValuesToParams(std::map<std::string, std::shared_ptr<Object>>& vars, std::vector<std::string>& names,
                       std::vector<std::shared_ptr<Object>>& objects) {
    CompareSzEq(names.size(), objects.size());
    AreTypesCorrect<Number>(objects);
    for (size_t i = 0; i < objects.size(); ++i) {
        vars[names[i]] = std::shared_ptr<Number>(new Number(As<Number>(objects[i])->GetValue()));
    }
}

std::shared_ptr<Object> SearchInAncestors(std::string s, std::shared_ptr<Object> env) {

}

void SetParams(std::shared_ptr<Object> env, std::shared_ptr<Object> obj) {
    if (obj == nullptr) {
        return;
    }
    IsType<Cell>(obj);
    auto cur_param = As<Cell>(obj)->GetFirst();
    if (Is<Cell>(cur_param)) {
        IsType<Symbol>(As<Cell>(cur_param)->GetFirst());
        auto func = SearchInAncestors(As<Symbol>(obj)->GetName(), As<Lambda>(obj)->parent);
        IsType<Function>(func);
        As<Function>(func)->Apply()
    }
};*/

