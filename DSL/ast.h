#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

/*
AST는 Abstract Syntax Tree (추상 구문 트리)를 의미한다.
파싱한 결과를 구조화한 트리 형태의 데이터 구조이다.
*/

namespace dsl
{
    struct Base;

    struct Name;
    struct NameList;
    struct Numeral;
    struct Boolean;
    struct LiteralString;

    struct AST;
    struct Block;
    
    struct Assignment;
    struct Expression;
    struct ExpressionList;
    struct PrimaryExpression;
    struct BinaryExpression;
    struct UnaryExpression;
    struct FunctionName;
    struct FunctionDefinition;
    struct FunctionCall;
    struct FunctionArgument;

    struct Statement;
    struct Return;
    struct Break;
    struct While;
    struct If;
    struct For;

    using BasePtr = std::shared_ptr<Base>;

    using NamePtr = std::shared_ptr<Name>;
    using NameListPtr = std::shared_ptr<NameList>;
    using NumeralPtr = std::shared_ptr<Numeral>;
    using BooleanPtr = std::shared_ptr<Boolean>;
    using LiteralStringPtr = std::shared_ptr<LiteralString>;

    using ASTPtr = std::shared_ptr<AST>;
    using BlockPtr = std::shared_ptr<Block>;

    using AssignmentPtr = std::shared_ptr<Assignment>;
    using ExpressionPtr = std::shared_ptr<Expression>;
    using ExpressionListPtr = std::shared_ptr<ExpressionList>;
    using PrimaryExpressionPtr = std::shared_ptr<PrimaryExpression>;
    using BinaryExpressionPtr = std::shared_ptr<BinaryExpression>;
    using UnaryExpressionPtr = std::shared_ptr<UnaryExpression>;
    using FunctionNamePtr = std::shared_ptr<FunctionName>;
    using FunctionDefinitionPtr = std::shared_ptr<FunctionDefinition>;
    using FunctionCallPtr = std::shared_ptr<FunctionCall>;
    using FunctionArgumentPtr = std::shared_ptr<FunctionArgument>;

    using StatementPtr = std::shared_ptr<Statement>;
    using ReturnPtr = std::shared_ptr<Return>;
    using BreakPtr = std::shared_ptr<Break>;
    using WhilePtr = std::shared_ptr<While>;
    using IfPtr = std::shared_ptr<If>;
    using ForPtr = std::shared_ptr<For>;


    struct Base
    {
        virtual void Print(const int indent = 0) const = 0;
    };



    struct Name : public Base
    {
        std::wstring name;

        Name() {}
        Name(const std::wstring& val) : name(val) {}

        void Print(const int indent = 0) const override;
    };

    struct NameList : public Base
    {
        std::vector<BasePtr> names;

        NameList() {}
        NameList(const std::vector<BasePtr>& val) : names(val) {}

        void Print(const int indent = 0) const override;
    };

    struct Numeral : public Base
    {
        bool isInteger = true;
        __int64 intValue = 0;
        double floatValue = 0.0;

        Numeral() {};
        Numeral(__int64 val) : isInteger(true), intValue(val), floatValue(0.0) {}
        Numeral(double val) : isInteger(false), intValue(0), floatValue(val) {}

        void Print(const int indent = 0) const override;
    };

    struct Boolean : public Base
    {
        bool value = false;

        Boolean() {}
        Boolean(bool val) : value(val) {}

        void Print(const int indent = 0) const override;
    };

    struct LiteralString : public Base
    {
        std::wstring value;

        LiteralString() {}
        LiteralString(const std::wstring& val) : value(val) {}

        void Print(const int indent = 0) const override;
    };

    struct AST : public Base
    {
        BasePtr block;

        AST() {}
        AST(const BasePtr& val) : block(val) {}

        void Print(const int indent = 0) const override;
    };

    struct Block : public Base
    {
       std::vector<BasePtr> statements;

       Block() {}
       Block(const std::vector<BasePtr>& val) : statements(val) {}
       
       void Print(const int indent = 0) const override;
    };

    struct Assignment : public Base
    {
        BasePtr name;
        BasePtr expression;

        Assignment() {}
        Assignment(const BasePtr& _name, const BasePtr& _expression) : name(_name), expression(_expression) {}

        void Print(const int indent = 0) const override;
    };

    struct Expression : public Base
    {
        BasePtr expression;

        Expression() {}
        Expression(const BasePtr& val) : expression(val) {}

        void Print(const int indent = 0) const override;
    };

    struct ExpressionList : public Base
    {
        std::vector<BasePtr> expressions;

        ExpressionList() {}
        ExpressionList(const std::vector<BasePtr>& val) : expressions(val) {}

        void Print(const int indent = 0) const override;
    };

    struct PrimaryExpression : public Base
    {
        BasePtr primaryExpression;

        void Print(const int indent = 0) const override;
    };

    struct BinaryExpression : public Base
    {
        BasePtr primaryExpression1;
        std::wstring binaryOperator;
        BasePtr primaryExpression2;

        BinaryExpression() {}
        BinaryExpression(const BasePtr& ex1, const std::wstring& op, const BasePtr& ex2) : primaryExpression1(ex1), binaryOperator(op), primaryExpression2(ex2) {}

        void Print(const int indent = 0) const override;
    };

    struct UnaryExpression : public Base
    {
        std::wstring unaryOperator;
        BasePtr primaryExpression;

        UnaryExpression() {}
        UnaryExpression(const std::wstring& op, const BasePtr& ex) : unaryOperator(op), primaryExpression(ex) {}

        void Print(const int indent = 0) const override;
    };

    struct FunctionName : public Base
    {
        Name name;

        void Print(const int indent = 0) const override;
    };

    struct FunctionDefinition : public Base
    {
        BasePtr name;
        BasePtr functionParameter;
        BasePtr block;

        FunctionDefinition() {}
        FunctionDefinition(const BasePtr& _name, const BasePtr& _functionParameter, const BasePtr& _block) : name(_name), functionParameter(_functionParameter), block(_block) {}

        void Print(const int indent = 0) const override;
    };

    struct FunctionParameter : public Base
    {
        BasePtr nameList;

        FunctionParameter() {}
        FunctionParameter(const BasePtr& _nameList) : nameList(_nameList) {}

        void Print(const int indent = 0) const override;
    };

    struct FunctionArgument : public Base
    {
        BasePtr expressionList;

        FunctionArgument() {}
        FunctionArgument(const BasePtr& _expressionList) : expressionList(_expressionList) {}

        void Print(const int indent = 0) const override;
    };

    struct FunctionCall : public Base
    {
        BasePtr name;
        BasePtr functionArgument;
  
        FunctionCall() {}
        FunctionCall(const BasePtr& _name, const BasePtr& _functionArgument) : name(_name), functionArgument(_functionArgument) {}

        void Print(const int indent = 0) const override;
    };

    struct Statement : public Base
    {
        BasePtr statement;

        Statement() {}
        Statement(const BasePtr& _statement) : statement(_statement) {}

        void Print(const int indent = 0) const override;
    };

    struct Return : public Base
    {
        std::vector<BasePtr> expressions;

        void Print(const int indent = 0) const override;
    };

    struct Break : public Base
    {
        std::wstring value;

        void Print(const int indent = 0) const override;
    };

    struct While : public Base
    {
        BasePtr expression;
        BasePtr statDo;

        void Print(const int indent = 0) const override;
    };

    struct If : public Base
    {
        BasePtr expression;
        BasePtr block;
        BasePtr statIf;

        If() {}
        If(const BasePtr& _expression, const BasePtr& _block, const BasePtr& _statIf) : expression(_expression), block(_block), statIf(_statIf) {}
        void Print(const int indent = 0) const override;
    };

    struct For : public Base
    {
        BasePtr name;
        BasePtr expression1;
        BasePtr expression2;
        BasePtr expression3;

        void Print(const int indent = 0) const override;
    };





}
