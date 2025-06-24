#include <iostream>
#include <variant>
#include <string>
#include <type_traits>

#include "ast.h"


namespace lua
{
    template <typename T>
    struct is_smart_ptr : std::false_type {};

    template <typename T>
    struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;


    void Name::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Name: " << name << std::endl;
    }

    void NameList::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "NameList:" << std::endl;
        for (const Name& name : names)
            name.Print(indent + 2);
    }

    void Numeral::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Numeral: " << value << std::endl;
    }

    void Boolean::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Boolean: " << value << std::endl;
    }

    void LiteralString::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "LiteralString: " << value << std::endl;
    }

    void UnaryOperator::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "UnaryOperator: " << value << std::endl;
    }

    void BinaryOperator::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "BinaryOperator: " << value << std::endl;
    }

    void Chunk::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Chunk:" << std::endl;
        if(block)
            block->Print(indent + 2);
            
    }

    //void Block::Print(const int indent /*= 0*/) const
    //{
    //    std::cout << std::string(indent, ' ') << "Block:" << std::endl;
    //    for (const StatementPtr& statement : statements)
    //    {
    //        if (statement)
    //            statement->Print(indent + 2);
    //    }
    //}
    void Block::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Block:" << std::endl;
        std::visit([indent](const auto& val)
            {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, StatementPtr>) {
                    if (val)
                        val->Print(indent + 2);
                }
                else if constexpr (std::is_same_v<T, std::vector<StatementPtr>>)
                {
                    for (const StatementPtr& statement : val)
                    {
                        if (statement)
                            statement->Print(indent + 2);
                    }
                }
            }, block);
    }

    void Assignment::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Assignment: " << std::endl;
        name.Print(indent + 2);
        if (expression)
            expression->Print(indent + 2);
    }

    void Expression::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Expression: " << std::endl;
        std::visit([indent](const auto& val)
            {
                using T = std::decay_t<decltype(val)>;
                if constexpr (is_smart_ptr_v<T>) {
                    if (val)
                        val->Print(indent + 2);
                }
                else {
                    val.Print(indent + 2);
                }
            }, expression);
    }

    void ExpressionList::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "ExpressionList: " << std::endl;
        for (const ExpressionPtr& expression : expressions)
        {
            if (expression)
                expression->Print(indent + 2);
        }
    }

    void PrimaryExpression::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "PrimaryExpression: " << std::endl;
        std::visit([indent](const auto& val)
            {
                using T = std::decay_t<decltype(val)>;
                if constexpr (is_smart_ptr_v<T>) {
                    if (val)
                        val->Print(indent + 2);
                }
                else {
                    val.Print(indent + 2);
                }
            }, primaryExpression);
    }

    void BinaryExpression::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "BinaryExpression: " << std::endl;
        if (primaryExpression1) 
            primaryExpression1->Print(indent + 2);
        binaryOperator.Print(indent + 2);
        if (primaryExpression2)
            primaryExpression2->Print(indent + 2);
    }

    void UnaryExpression::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "UnaryExpression: " << std::endl;
        unaryOperator.Print(indent + 2);
        if (primaryExpression)
            primaryExpression->Print(indent + 2);
    }

    void FunctionName::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionName: " << std::endl;
    }

    void FunctionBody::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionBody: " << std::endl;
    }

    void FunctionDefinition::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionDefinition: " << std::endl;
    }

    void FunctionArguments::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionArguments: " << std::endl;
        if (expressionList)
            expressionList->Print(indent + 2);
    }

    void FunctionCall::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionCall: " << std::endl;
        name.Print(indent + 2);
        if (functionArguments)
            functionArguments->Print(indent + 2);
    }

    void Statement::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Statement: " << std::endl;
        std::visit([indent](const auto& val) 
            {
                using T = std::decay_t<decltype(val)>;
                if constexpr (is_smart_ptr_v<T>) {
                    if (val)
                        val->Print(indent + 2);
                }
                else {
                    val.Print(indent + 2);
                }
            }, statement);
    }

    void StatReturn::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatReturn: " << std::endl;
    }

    void StatBreak::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatBreak: " << std::endl;
    }

    void StatGoto::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatGoto: " << std::endl;
    }

    void StatDo::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatDo: " << std::endl;
    }

    void StatWhile::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatWhile: " << std::endl;
    }

    void StatRepeat::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatRepeat: " << std::endl;
    }

    void StatIf::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatIf: " << std::endl;
        if (expression)
            expression->Print(indent + 2);
        if(block)
            block->Print(indent + 2);
        if (statIf)
            statIf->Print(indent + 2);
    }

    void StatFor::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatFor: " << std::endl;
    }

    void StatFunction::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "StatFunction: " << std::endl;
    }
}
