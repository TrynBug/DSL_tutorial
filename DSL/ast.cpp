
#include "ast.h"


namespace dsl
{
    void Name::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Name: " << name << std::endl;
    }

    void NameList::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"NameList:" << std::endl;
        for (const BasePtr& name : names)
        {
            if (name)
                name->Print(indent + 2);
        }
    }

    void Numeral::Print(const int indent /*= 0*/) const
    {
        if (isInteger)
            std::wcout << std::wstring(indent, ' ') << L"Numeral (int): " << intValue << std::endl;
        else
            std::wcout << std::wstring(indent, ' ') << L"Numeral (float): " << floatValue << std::endl;
    }

    void Boolean::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Boolean: " << value << std::endl;
    }

    void LiteralString::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"LiteralString: " << value << std::endl;
    }

    void AST::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"AST:" << std::endl;
        if(block)
            block->Print(indent + 2);
            
    }

    void Block::Print(const int indent /*= 0*/) const
    {
       std::wcout << std::wstring(indent, ' ') << L"Block:" << std::endl;
       for (const BasePtr& statement : statements)
       {
           if (statement)
               statement->Print(indent + 2);
       }
    }

    void Assignment::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Assignment: " << std::endl;
        if (name)
            name->Print(indent + 2);
        if (expression)
            expression->Print(indent + 2);
    }

    void Expression::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Expression: " << std::endl;
        if (expression)
            expression->Print(indent + 2);
    }

    void ExpressionList::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"ExpressionList: " << std::endl;
        for (const BasePtr& expression : expressions)
        {
            if (expression)
                expression->Print(indent + 2);
        }
    }

    void PrimaryExpression::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"PrimaryExpression: " << std::endl;

    }

    void BinaryExpression::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"BinaryExpression: " << std::endl;
        if (primaryExpression1) 
            primaryExpression1->Print(indent + 2);
        std::wcout << std::wstring(indent + 2, ' ') << L"binaryOperator: " << binaryOperator << std::endl;
        if (primaryExpression2)
            primaryExpression2->Print(indent + 2);
    }

    void UnaryExpression::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"UnaryExpression: " << std::endl;
        std::wcout << std::wstring(indent + 2, ' ') << L"unaryOperator: " << unaryOperator << std::endl;
        if (primaryExpression)
            primaryExpression->Print(indent + 2);
    }

    void FunctionName::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"FunctionName: " << std::endl;
    }

    void FunctionDefinition::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"FunctionDefinition: " << std::endl;

        if (name)
            name->Print(indent + 2);
        if (functionParameter)
            functionParameter->Print(indent + 2);
        if (block)
            block->Print(indent + 2);
    }

    void FunctionParameter::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"FunctionParameter: " << std::endl;

        if (nameList)
            nameList->Print(indent + 2);
    }

    void FunctionCall::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"FunctionCall: " << std::endl;
        if (name)
            name->Print(indent + 2);
        if (functionArgument)
            functionArgument->Print(indent + 2);
    }

    void FunctionArgument::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"FunctionArgument: " << std::endl;
        if(expressionList)
            expressionList->Print(indent + 2);
    }

    void Statement::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Statement: " << std::endl;
        if (statement)
            statement->Print(indent + 2);
    }

    void Return::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Return: " << std::endl;
    }

    void Break::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Break: " << std::endl;
    }

    void While::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"While: " << std::endl;
    }

    void If::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"If: " << std::endl;
        if (expression)
            expression->Print(indent + 2);
        if(block)
            block->Print(indent + 2);
        if (statIf)
            statIf->Print(indent + 2);
    }

    void For::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"For: " << std::endl;
    }
}
