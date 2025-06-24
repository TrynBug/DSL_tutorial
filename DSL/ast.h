#pragma once

#include <string>
#include <variant>
#include <optional>
#include <vector>
#include <memory>




namespace lua
{
    struct Name;
    struct NameList;
    struct Numeral;
    struct Boolean;
    struct LiteralString;
    struct UnaryOperator;
    struct BinaryOperator;

    struct Chunk;
    struct Block;
    
    struct Assignment;
    struct Expression;
    struct ExpressionList;
    struct PrimaryExpression;
    struct BinaryExpression;
    struct UnaryExpression;
    struct FunctionName;
    struct FunctionBody;
    struct FunctionDefinition;
    struct FunctionArguments;
    struct FunctionCall;

    struct Statement;
    struct StatReturn;
    struct StatBreak;
    struct StatGoto;
    struct StatDo;
    struct StatWhile;
    struct StatRepeat;
    struct StatIf;
    struct StatFor;
    struct StatFunction;

    using ChunkPtr = std::shared_ptr<Chunk>;
    using BlockPtr = std::shared_ptr<Block>;

    using AssignmentPtr = std::shared_ptr<Assignment>;
    using ExpressionPtr = std::shared_ptr<Expression>;
    using ExpressionListPtr = std::shared_ptr<ExpressionList>;
    using PrimaryExpressionPtr = std::shared_ptr<PrimaryExpression>;
    using BinaryExpressionPtr = std::shared_ptr<BinaryExpression>;
    using UnaryExpressionPtr = std::shared_ptr<UnaryExpression>;
    using FunctionNamePtr = std::shared_ptr<FunctionName>;
    using FunctionBodyPtr = std::shared_ptr<FunctionBody>;
    using FunctionDefinitionPtr = std::shared_ptr<FunctionDefinition>;
    using FunctionArgumentsPtr = std::shared_ptr<FunctionArguments>;
    using FunctionCallPtr = std::shared_ptr<FunctionCall>;

    using StatementPtr = std::shared_ptr<Statement>;
    using StatReturnPtr = std::shared_ptr<StatReturn>;
    using StatBreakPtr = std::shared_ptr<StatBreak>;
    using StatGotoPtr = std::shared_ptr<StatGoto>;
    using StatDoPtr = std::shared_ptr<StatDo>;
    using StatWhilePtr = std::shared_ptr<StatWhile>;
    using StatRepeatPtr = std::shared_ptr<StatRepeat>;
    using StatIfPtr = std::shared_ptr<StatIf>;
    using StatForPtr = std::shared_ptr<StatFor>;
    using StatFunctionPtr = std::shared_ptr<StatFunction>;




    struct Name
    {
        std::string name;

        void Print(const int indent = 0) const;
    };

    struct NameList
    {
        std::vector<Name> names;

        void Print(const int indent = 0) const;
    };

    struct Numeral
    {
        int value;

        void Print(const int indent = 0) const;
    };

    struct Boolean
    {
        bool value;

        void Print(const int indent = 0) const;
    };

    struct LiteralString
    {
        std::string value;

        void Print(const int indent = 0) const;
    };

    struct UnaryOperator
    {
        std::string value;

        void Print(const int indent = 0) const;
    };

    struct BinaryOperator
    {
        std::string value;

        void Print(const int indent = 0) const;
    };


    struct Chunk
    {
        BlockPtr block;

        void Print(const int indent = 0) const;
    };

    //struct Block
    //{
    //    std::vector<StatementPtr> statements;

    //    Block(const std::vector<StatementPtr>& in_statements)
    //    {
    //        statements = in_statements;
    //    }
    //    Block(const StatementPtr& in_statement)
    //    {
    //        statements.push_back(in_statement);
    //    }
    //    Block() 
    //    {
    //    }

    //    void push_back(const StatementPtr& st) {}
    //    void push_back(const std::vector<StatementPtr>& st) {}

    //    void Print(const int indent = 0) const;
    //};

    using TypeBlock = std::variant<
        StatementPtr,
        std::vector<StatementPtr>>;
    struct Block
    {
        TypeBlock block;

        void Print(const int indent = 0) const;
    };

    struct Assignment
    {
        Name name;
        ExpressionPtr expression;

        void Print(const int indent = 0) const;
    };

    using TypeExpression = std::variant<
        PrimaryExpressionPtr,
        FunctionDefinitionPtr,
        BinaryExpressionPtr,
        UnaryExpressionPtr>;
    struct Expression
    {
        TypeExpression expression;

        void Print(const int indent = 0) const;
    };

    struct ExpressionList
    {
        std::vector<ExpressionPtr> expressions;

        void Print(const int indent = 0) const;
    };

    using TypePrimaryExpression = std::variant<Name, Numeral, Boolean, LiteralString, ExpressionPtr>;
    struct PrimaryExpression
    {
        TypePrimaryExpression primaryExpression;

        void Print(const int indent = 0) const;
    };

    struct BinaryExpression
    {
        PrimaryExpressionPtr primaryExpression1;
        BinaryOperator binaryOperator;
        PrimaryExpressionPtr primaryExpression2;

        void Print(const int indent = 0) const;
    };

    struct UnaryExpression
    {
        UnaryOperator unaryOperator;
        PrimaryExpressionPtr primaryExpression;

        void Print(const int indent = 0) const;
    };

    struct FunctionName
    {
        Name name;

        void Print(const int indent = 0) const;
    };

    struct FunctionBody
    {
        NameList params;
        BlockPtr block;

        void Print(const int indent = 0) const;
    };

    struct FunctionDefinition
    {
        Name name;
        FunctionBodyPtr functionBody;

        void Print(const int indent = 0) const;
    };

    struct FunctionArguments
    {
        ExpressionListPtr expressionList;

        void Print(const int indent = 0) const;
    };

    struct FunctionCall
    {
        Name name;
        FunctionArgumentsPtr functionArguments;

        void Print(const int indent = 0) const;
    };


    using TypeStatement = std::variant<
        AssignmentPtr,
        FunctionDefinitionPtr,
        FunctionCallPtr,
        StatBreakPtr,
        StatGotoPtr,
        StatDoPtr,
        StatWhilePtr,
        StatRepeatPtr,
        StatIfPtr,
        StatForPtr>;
    struct Statement
    {
        TypeStatement statement;

        void Print(const int indent = 0) const;
    };

    struct StatReturn 
    {
        std::vector<ExpressionPtr> expressions;

        void Print(const int indent = 0) const;
    };

    struct StatBreak 
    {
        std::string value;

        void Print(const int indent = 0) const;
    };

    struct StatGoto
    {
        Name name;

        void Print(const int indent = 0) const;
    };

    struct StatDo
    {
        BlockPtr block;

        void Print(const int indent = 0) const;
    };

    struct StatWhile
    {
        ExpressionPtr expression;
        StatDoPtr statDo;

        void Print(const int indent = 0) const;
    };

    struct StatRepeat
    {
        BlockPtr block;
        ExpressionPtr expression;

        void Print(const int indent = 0) const;
    };

    struct StatIf
    {
        ExpressionPtr expression;
        BlockPtr block;
        StatIfPtr statIf;

        void Print(const int indent = 0) const;
    };

    struct StatFor
    {
        Name name;
        ExpressionPtr expression1;
        ExpressionPtr expression2;
        ExpressionPtr expression3;

        void Print(const int indent = 0) const;
    };

    struct StatFunction
    {
        FunctionNamePtr functionName;
        FunctionBodyPtr functionBody;

        void Print(const int indent = 0) const;
    };

    void print(const std::string str);










}
