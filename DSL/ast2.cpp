#include <iostream>
#include <variant>
#include <string>
#include <type_traits>

#include "ast2.h"


namespace lua2
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
        for (const BasePtr& name : names)
        {
            if (name)
                name->Print(indent + 2);
        }
    }

    void Numeral::Print(const int indent /*= 0*/) const
    {
        if (isInteger)
            std::cout << std::string(indent, ' ') << "Numeral (int): " << intValue << std::endl;
        else
            std::cout << std::string(indent, ' ') << "Numeral (float): " << floatValue << std::endl;
    }

    void Boolean::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Boolean: " << value << std::endl;
    }

    void LiteralString::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "LiteralString: " << value << std::endl;
    }

    void Chunk::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Chunk:" << std::endl;
        if(block)
            block->Print(indent + 2);
            
    }

    void Block::Print(const int indent /*= 0*/) const
    {
       std::cout << std::string(indent, ' ') << "Block:" << std::endl;
       for (const BasePtr& statement : statements)
       {
           if (statement)
               statement->Print(indent + 2);
       }
    }

    void Assignment::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Assignment: " << std::endl;
        if (name)
            name->Print(indent + 2);
        if (expression)
            expression->Print(indent + 2);
    }

    void Expression::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Expression: " << std::endl;
        if (expression)
            expression->Print(indent + 2);
    }

    void ExpressionList::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "ExpressionList: " << std::endl;
        for (const BasePtr& expression : expressions)
        {
            if (expression)
                expression->Print(indent + 2);
        }
    }

    void PrimaryExpression::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "PrimaryExpression: " << std::endl;

    }

    void BinaryExpression::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "BinaryExpression: " << std::endl;
        if (primaryExpression1) 
            primaryExpression1->Print(indent + 2);
        std::cout << std::string(indent + 2, ' ') << "binaryOperator: " << binaryOperator << std::endl;
        if (primaryExpression2)
            primaryExpression2->Print(indent + 2);
    }

    void UnaryExpression::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "UnaryExpression: " << std::endl;
        std::cout << std::string(indent + 2, ' ') << "unaryOperator: " << unaryOperator << std::endl;
        if (primaryExpression)
            primaryExpression->Print(indent + 2);
    }

    void FunctionName::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionName: " << std::endl;
    }

    void FunctionDefinition::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionDefinition: " << std::endl;

        if (name)
            name->Print(indent + 2);
        if (functionParameter)
            functionParameter->Print(indent + 2);
        if (block)
            block->Print(indent + 2);
    }

    void FunctionParameter::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionParameter: " << std::endl;

        if (nameList)
            nameList->Print(indent + 2);
    }

    void FunctionCall::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionCall: " << std::endl;
        if (name)
            name->Print(indent + 2);
        if (functionArgument)
            functionArgument->Print(indent + 2);
    }

    void FunctionArgument::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "FunctionArgument: " << std::endl;
        if(expressionList)
            expressionList->Print(indent + 2);
    }

    void Statement::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Statement: " << std::endl;
        if (statement)
            statement->Print(indent + 2);
    }

    void Return::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Return: " << std::endl;
    }

    void Break::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "Break: " << std::endl;
    }

    void While::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "While: " << std::endl;
    }

    void If::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "If: " << std::endl;
        if (expression)
            expression->Print(indent + 2);
        if(block)
            block->Print(indent + 2);
        if (statIf)
            statIf->Print(indent + 2);
    }

    void For::Print(const int indent /*= 0*/) const
    {
        std::cout << std::string(indent, ' ') << "For: " << std::endl;
    }
}
