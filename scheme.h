#pragma once

#include <string>
#include <sstream>
#include "object.h"

std::shared_ptr<Object> UnbindFunc(std::shared_ptr<Object> obj);

void UnbindList(std::vector<std::shared_ptr<Object>>& objects,
                                                std::shared_ptr<Object> cell);

class Interpreter {
public:
    Interpreter();
    std::string Run(std::string str);
};