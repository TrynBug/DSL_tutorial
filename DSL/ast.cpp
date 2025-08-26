#include "pch.h"

#include "ast.h"


namespace dsl
{
    void Name::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Name: " << name << std::endl;
    }

    void Name::Iterate(const FuncASTIterateCallback& callback) const
    {
        return;
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

    void NameList::Iterate(const FuncASTIterateCallback& callback) const
    {
        for (const BaseCPtr& name : names)
        {
            callback(name);

            if (name)
                name->Iterate(callback);
        }
    }

    void Numeral::Print(const int indent /*= 0*/) const
    {
        if (isInteger)
            std::wcout << std::wstring(indent, ' ') << L"Numeral (int): " << intValue << std::endl;
        else
            std::wcout << std::wstring(indent, ' ') << L"Numeral (float): " << floatValue << std::endl;
    }

    void Numeral::Iterate(const FuncASTIterateCallback& callback) const
    {
        return;
    }

    void Boolean::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Boolean: " << value << std::endl;
    }

    void Boolean::Iterate(const FuncASTIterateCallback& callback) const
    {
        return;
    }

    void LiteralString::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"LiteralString: " << value << std::endl;
    }

    void LiteralString::Iterate(const FuncASTIterateCallback& callback) const
    {
        return;
    }

    void AST::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"AST:" << std::endl;
        if(block)
            block->Print(indent + 2);
    }

    void AST::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(block);

        if (block)
            block->Iterate(callback);
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

    void Block::Iterate(const FuncASTIterateCallback& callback) const
    {
        for (const BaseCPtr& statement : statements)
        {
            callback(statement);

            if (statement)
                statement->Iterate(callback);
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

    void Assignment::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(name);
        callback(expression);

        if (name)
            name->Iterate(callback);
        if (expression)
            expression->Iterate(callback);
    }

    void Expression::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Expression: " << std::endl;
        if (expression)
            expression->Print(indent + 2);
    }

    void Expression::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(expression);

        if (expression)
            expression->Iterate(callback);
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

    void ExpressionList::Iterate(const FuncASTIterateCallback& callback) const
    {
        for (const BaseCPtr& expression : expressions)
        {
            callback(expression);

            if (expression)
                expression->Iterate(callback);
        }
    }

    void PrimaryExpression::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"PrimaryExpression: " << std::endl;

    }

    void PrimaryExpression::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(primaryExpression);

        if (primaryExpression)
            primaryExpression->Iterate(callback);
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

    void BinaryExpression::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(primaryExpression1);

        if (primaryExpression1)
            primaryExpression1->Iterate(callback);

        callback(primaryExpression2);

        if (primaryExpression2)
            primaryExpression2->Iterate(callback);
    }

    void UnaryExpression::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"UnaryExpression: " << std::endl;
        std::wcout << std::wstring(indent + 2, ' ') << L"unaryOperator: " << unaryOperator << std::endl;
        if (primaryExpression)
            primaryExpression->Print(indent + 2);
    }

    void UnaryExpression::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(primaryExpression);

        if (primaryExpression)
            primaryExpression->Iterate(callback);
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

    void FunctionDefinition::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(name);
        if (name)
            name->Iterate(callback);

        callback(functionParameter);
        if (functionParameter)
            functionParameter->Iterate(callback);

        callback(block);
        if (block)
            block->Iterate(callback);
    }

    void FunctionParameter::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"FunctionParameter: " << std::endl;

        if (nameList)
            nameList->Print(indent + 2);
    }

    void FunctionParameter::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(nameList);
        if (nameList)
            nameList->Iterate(callback);
    }

    void FunctionArgument::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"FunctionArgument: " << std::endl;
        if (expressionList)
            expressionList->Print(indent + 2);
    }

    void FunctionArgument::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(expressionList);
        if (expressionList)
            expressionList->Iterate(callback);
    }

    void FunctionCall::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"FunctionCall: " << std::endl;
        if (name)
            name->Print(indent + 2);
        if (functionArgument)
            functionArgument->Print(indent + 2);
    }

    void FunctionCall::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(name);
        if (name)
            name->Iterate(callback);

        callback(functionArgument);
        if (functionArgument)
            functionArgument->Iterate(callback);
    }

    void Statement::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Statement: " << std::endl;
        if (statement)
            statement->Print(indent + 2);
    }

    void Statement::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(statement);
        if (statement)
            statement->Iterate(callback);
    }

    void Return::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Return: " << std::endl;
    }

    void Return::Iterate(const FuncASTIterateCallback& callback) const
    {
        for (const BaseCPtr& expression : expressions)
        {
            callback(expression);

            if (expression)
                expression->Iterate(callback);
        }
    }

    void Break::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"Break: " << std::endl;
    }

    void Break::Iterate(const FuncASTIterateCallback& callback) const
    {
        return;
    }

    void While::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"While: " << std::endl;
    }

    void While::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(expression);
        if (expression)
            expression->Iterate(callback);

        callback(statDo);
        if (statDo)
            statDo->Iterate(callback);
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

    void If::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(expression);
        if (expression)
            expression->Iterate(callback);

        callback(block);
        if (block)
            block->Iterate(callback);

        callback(statIf);
        if (statIf)
            statIf->Iterate(callback);
    }

    void For::Print(const int indent /*= 0*/) const
    {
        std::wcout << std::wstring(indent, ' ') << L"For: " << std::endl;
    }

    void For::Iterate(const FuncASTIterateCallback& callback) const
    {
        callback(name);
        if (name)
            name->Iterate(callback);

        callback(expression1);
        if (expression1)
            expression1->Iterate(callback);

        callback(expression2);
        if (expression2)
            expression2->Iterate(callback);

        callback(expression3);
        if (expression3)
            expression3->Iterate(callback);
    }
}
