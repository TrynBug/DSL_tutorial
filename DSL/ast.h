#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>

namespace ast {

    struct identifier {
        std::string name;
    };

    struct number {
        int value;
    };

    struct string {
        std::string value;
    };

    struct var_assign {
        identifier lhs;
        number rhs;
    };

    struct print_stmt {
        string value;
    };

    struct if_stmt;

    using statement = std::variant<
        var_assign,
        print_stmt,
        std::shared_ptr<if_stmt>  // 재귀 구조 → 포인터 필요
    >;

    struct if_stmt {
        identifier cond_var;
        number cond_value;
        std::vector<statement> body;
    };

    struct program {
        std::vector<statement> statements;
    };


    struct employee
    {
        int age;
        std::string surname;
        std::string forename;
        double salary;
    };
}


