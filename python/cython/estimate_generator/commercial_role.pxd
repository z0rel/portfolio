

from libcpp.string cimport string
from libcpp.unordered_map cimport unordered_map
from libcpp.memory cimport shared_ptr

# Declare the class with cdef
cdef extern from "commercial_role.h" namespace "commercial":
    cdef cppclass Role:
        Role()
        Role(const Role&)
        double trudoemkost_summary
        double employees_cnt
        double salary_day
        double summary_salary
        string role_name
        unsigned int rows_cnt
        int is_ip
        unsigned int position
        unsigned int role_id


cdef extern from "parse_context.h" namespace "commercial":
    cdef cppclass RoleParsingContext:
        RoleParsingContext()
        RoleParsingContext(const RoleParsingContext&)
        RoleParsingContext(const string &_role_name, double _salary_value, int _role_position_on_calculation)

        string role_name # Название профессии
        double salary_value # Значение зарплаты
        int role_position_on_calculation  # Позиция роли в калькуляции


cdef extern from "parse_context.h" namespace "commercial":
    cdef cppclass CathegoryPosition:
        CathegoryPosition()
        CathegoryPosition(const RoleParsingContext&)
        CathegoryPosition(double _dcathegory, int _position)

        string scathegory
        double dcathegory
        int position


cdef extern from "cmath":
    double round(double x)
