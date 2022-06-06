#ifndef PARSE_CONTEXT_H
#define PARSE_CONTEXT_H


#include <string>
#include <unordered_map>
#include <memory>

namespace commercial {

class RoleParsingContext {
public:
    /// Название профессии
    std::string role_name;
    /// Значение зарплаты
    double salary_value = 0.0;
    /// Позиция роли в калькуляции
    int role_position_on_calculation = 0;

    RoleParsingContext(const std::string &_role_name, double _salary_value, int _role_position_on_calculation)
    : role_name(_role_name), salary_value(_salary_value), role_position_on_calculation(_role_position_on_calculation) {}
    RoleParsingContext() {}
};


class CathegoryPosition {
public:
    double dcathegory = 0.0;
    int position = -1;

    CathegoryPosition(double _dcathegory, int _position)
      : dcathegory(_dcathegory), position(_position) {}
};




}

#endif
