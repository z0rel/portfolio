#ifndef COMMERCIAL_ROLE_H
#define COMMERCIAL_ROLE_H

#include <string>

namespace commercial {

class Role {
public:
    double trudoemkost_summary = 0.0;
    double employees_cnt = 0.0;
    double salary_day = 0.0;
    double summary_salary = 0.0;
    std::string role_name;
    unsigned int rows_cnt = 0;
    int is_ip = 0; /// является ли роль ИП
    unsigned int position = 0;
    unsigned int role_id = 0;
};

}

#endif // COMMERCIAL_ROLE_H
