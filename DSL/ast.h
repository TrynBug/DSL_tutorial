#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>

/*
AST는 Abstract Syntax Tree (추상 구문 트리)를 의미한다.
파싱한 결과를 구조화한 트리 형태의 데이터 구조이다.
*/

namespace dsl
{
    enum class EASTType
    {
        Base,

        Name,
        NameList,
        Numeral,
        Boolean,
        LiteralString,

        AST,
        Block,

        Assignment,
        Expression,
        ExpressionList,
        PrimaryExpression,
        BinaryExpression,
        UnaryExpression,
        FunctionName,
        FunctionDefinition,
        FunctionParameter,
        FunctionCall,
        FunctionArgument,

        Statement,
        Return,
        Break,
        While,
        If,
        For
    };

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
    struct FunctionParameter;
    struct FunctionCall;
    struct FunctionArgument;

    struct Statement;
    struct Return;
    struct Break;
    struct While;
    struct If;
    struct For;

    using BasePtr = std::shared_ptr<Base>;
    using BaseCPtr = std::shared_ptr<const Base>;

    using NamePtr = std::shared_ptr<Name>;
    using NameCPtr = std::shared_ptr<const Name>;
    using NameListPtr = std::shared_ptr<NameList>;
    using NameListCPtr = std::shared_ptr<const NameList>;
    using NumeralPtr = std::shared_ptr<Numeral>;
    using NumeralCPtr = std::shared_ptr<const Numeral>;
    using BooleanPtr = std::shared_ptr<Boolean>;
    using BooleanCPtr = std::shared_ptr<const Boolean>;
    using LiteralStringPtr = std::shared_ptr<LiteralString>;
    using LiteralStringCPtr = std::shared_ptr<const LiteralString>;

    using ASTPtr = std::shared_ptr<AST>;
    using ASTCPtr = std::shared_ptr<const AST>;
    using BlockPtr = std::shared_ptr<Block>;
    using BlockCPtr = std::shared_ptr<const Block>;

    using AssignmentPtr = std::shared_ptr<Assignment>;
    using AssignmentCPtr = std::shared_ptr<const Assignment>;
    using ExpressionPtr = std::shared_ptr<Expression>;
    using ExpressionCPtr = std::shared_ptr<const Expression>;
    using ExpressionListPtr = std::shared_ptr<ExpressionList>;
    using ExpressionListCPtr = std::shared_ptr<const ExpressionList>;
    using PrimaryExpressionPtr = std::shared_ptr<PrimaryExpression>;
    using PrimaryExpressionCPtr = std::shared_ptr<const PrimaryExpression>;
    using BinaryExpressionPtr = std::shared_ptr<BinaryExpression>;
    using BinaryExpressionCPtr = std::shared_ptr<const BinaryExpression>;
    using UnaryExpressionPtr = std::shared_ptr<UnaryExpression>;
    using UnaryExpressionCPtr = std::shared_ptr<const UnaryExpression>;
    using FunctionNamePtr = std::shared_ptr<FunctionName>;
    using FunctionNameCPtr = std::shared_ptr<const FunctionName>;
    using FunctionDefinitionPtr = std::shared_ptr<FunctionDefinition>;
    using FunctionDefinitionCPtr = std::shared_ptr<const FunctionDefinition>;
    using FunctionParameterPtr = std::shared_ptr<FunctionParameter>;
    using FunctionParameterCPtr = std::shared_ptr<const FunctionParameter>;
    using FunctionCallPtr = std::shared_ptr<FunctionCall>;
    using FunctionCallCPtr = std::shared_ptr<const FunctionCall>;
    using FunctionArgumentPtr = std::shared_ptr<FunctionArgument>;
    using FunctionArgumentCPtr = std::shared_ptr<const FunctionArgument>;

    using StatementPtr = std::shared_ptr<Statement>;
    using StatementCPtr = std::shared_ptr<const Statement>;
    using ReturnPtr = std::shared_ptr<Return>;
    using ReturnCPtr = std::shared_ptr<const Return>;
    using BreakPtr = std::shared_ptr<Break>;
    using BreakCPtr = std::shared_ptr<const Break>;
    using WhilePtr = std::shared_ptr<While>;
    using WhileCPtr = std::shared_ptr<const While>;
    using IfPtr = std::shared_ptr<If>;
    using IfCPtr = std::shared_ptr<const If>;
    using ForPtr = std::shared_ptr<For>;
    using ForCPtr = std::shared_ptr<const For>;


    using FuncASTIterateCallback = std::function<void(const BaseCPtr& spBase)>;


    struct Base
    {
        virtual EASTType GetType() const { return EASTType::Base; }
        virtual void Print(const int indent = 0) const = 0;
        virtual void Iterate(const FuncASTIterateCallback& callback) const = 0;
    };



    struct Name : public Base
    {
        std::wstring name;

        Name() {}
        Name(const std::wstring& val) : name(val) {}

        virtual EASTType GetType() const override { return EASTType::Name; }
        void Print(const int indent = 0) const override;
    };

    struct NameList : public Base
    {
        std::vector<BasePtr> names;

        NameList() {}
        NameList(const std::vector<BasePtr>& val) : names(val) {}

        virtual EASTType GetType() const override { return EASTType::NameList; }
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

        virtual EASTType GetType() const override { return EASTType::Numeral; }
        void Print(const int indent = 0) const override;
    };

    struct Boolean : public Base
    {
        bool value = false;

        Boolean() {}
        Boolean(bool val) : value(val) {}

        virtual EASTType GetType() const override { return EASTType::Boolean; }
        void Print(const int indent = 0) const override;
    };

    struct LiteralString : public Base
    {
        std::wstring value;

        LiteralString() {}
        LiteralString(const std::wstring& val) : value(val) {}

        virtual EASTType GetType() const override { return EASTType::LiteralString; }
        void Print(const int indent = 0) const override;
    };

    struct AST : public Base
    {
        BasePtr block;

        AST() {}
        AST(const BasePtr& val) : block(val) {}

        virtual EASTType GetType() const override { return EASTType::AST; }
        void Print(const int indent = 0) const override;
    };

    struct Block : public Base
    {
       std::vector<BasePtr> statements;

       Block() {}
       Block(const std::vector<BasePtr>& val) : statements(val) {}
       
       virtual EASTType GetType() const override { return EASTType::Block; }
       void Print(const int indent = 0) const override;
       virtual void Iterate(const FuncASTIterateCallback& callback) const override;
    };

    struct Assignment : public Base
    {
        BasePtr name;
        BasePtr expression;

        Assignment() {}
        Assignment(const BasePtr& _name, const BasePtr& _expression) : name(_name), expression(_expression) {}

        virtual EASTType GetType() const override { return EASTType::Assignment; }
        void Print(const int indent = 0) const override;
    };

    struct Expression : public Base
    {
        BasePtr expression;

        Expression() {}
        Expression(const BasePtr& val) : expression(val) {}

        virtual EASTType GetType() const override { return EASTType::Expression; }
        void Print(const int indent = 0) const override;
    };

    struct ExpressionList : public Base
    {
        std::vector<BasePtr> expressions;

        ExpressionList() {}
        ExpressionList(const std::vector<BasePtr>& val) : expressions(val) {}

        virtual EASTType GetType() const override { return EASTType::ExpressionList; }
        void Print(const int indent = 0) const override;
    };

    struct PrimaryExpression : public Base
    {
        BasePtr primaryExpression;

        virtual EASTType GetType() const override { return EASTType::PrimaryExpression; }
        void Print(const int indent = 0) const override;
    };

    struct BinaryExpression : public Base
    {
        BasePtr primaryExpression1;
        std::wstring binaryOperator;
        BasePtr primaryExpression2;

        BinaryExpression() {}
        BinaryExpression(const BasePtr& ex1, const std::wstring& op, const BasePtr& ex2) : primaryExpression1(ex1), binaryOperator(op), primaryExpression2(ex2) {}

        virtual EASTType GetType() const override { return EASTType::BinaryExpression; }
        void Print(const int indent = 0) const override;
    };

    struct UnaryExpression : public Base
    {
        std::wstring unaryOperator;
        BasePtr primaryExpression;

        UnaryExpression() {}
        UnaryExpression(const std::wstring& op, const BasePtr& ex) : unaryOperator(op), primaryExpression(ex) {}

        virtual EASTType GetType() const override { return EASTType::UnaryExpression; }
        void Print(const int indent = 0) const override;
    };

    struct FunctionName : public Base
    {
        Name name;

        virtual EASTType GetType() const override { return EASTType::FunctionName; }
        void Print(const int indent = 0) const override;
    };

    struct FunctionDefinition : public Base
    {
        BasePtr name;
        BasePtr functionParameter;
        BasePtr block;

        FunctionDefinition() {}
        FunctionDefinition(const BasePtr& _name, const BasePtr& _functionParameter, const BasePtr& _block) : name(_name), functionParameter(_functionParameter), block(_block) {}

        virtual EASTType GetType() const override { return EASTType::FunctionDefinition; }
        void Print(const int indent = 0) const override;
    };

    struct FunctionParameter : public Base
    {
        BasePtr nameList;

        FunctionParameter() {}
        FunctionParameter(const BasePtr& _nameList) : nameList(_nameList) {}

        virtual EASTType GetType() const override { return EASTType::FunctionParameter; }
        void Print(const int indent = 0) const override;
    };

    struct FunctionArgument : public Base
    {
        BasePtr expressionList;

        FunctionArgument() {}
        FunctionArgument(const BasePtr& _expressionList) : expressionList(_expressionList) {}

        virtual EASTType GetType() const override { return EASTType::FunctionArgument; }
        void Print(const int indent = 0) const override;
    };

    struct FunctionCall : public Base
    {
        BasePtr name;
        BasePtr functionArgument;
  
        FunctionCall() {}
        FunctionCall(const BasePtr& _name, const BasePtr& _functionArgument) : name(_name), functionArgument(_functionArgument) {}

        virtual EASTType GetType() const override { return EASTType::FunctionCall; }
        void Print(const int indent = 0) const override;
    };

    struct Statement : public Base
    {
        BasePtr statement;

        Statement() {}
        Statement(const BasePtr& _statement) : statement(_statement) {}

        virtual EASTType GetType() const override { return EASTType::Statement; }
        void Print(const int indent = 0) const override;
    };

    struct Return : public Base
    {
        std::vector<BasePtr> expressions;

        virtual EASTType GetType() const override { return EASTType::Return; }
        void Print(const int indent = 0) const override;
    };

    struct Break : public Base
    {
        std::wstring value;

        virtual EASTType GetType() const override { return EASTType::Break; }
        void Print(const int indent = 0) const override;
    };

    struct While : public Base
    {
        BasePtr expression;
        BasePtr statDo;

        virtual EASTType GetType() const override { return EASTType::While; }
        void Print(const int indent = 0) const override;
    };

    struct If : public Base
    {
        BasePtr expression;
        BasePtr block;
        BasePtr statIf;

        If() {}
        If(const BasePtr& _expression, const BasePtr& _block, const BasePtr& _statIf) : expression(_expression), block(_block), statIf(_statIf) {}

        virtual EASTType GetType() const override { return EASTType::If; }
        void Print(const int indent = 0) const override;
    };

    struct For : public Base
    {
        BasePtr name;
        BasePtr expression1;
        BasePtr expression2;
        BasePtr expression3;

        virtual EASTType GetType() const override { return EASTType::For; }
        void Print(const int indent = 0) const override;
    };





}
