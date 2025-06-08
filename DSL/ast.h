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
    
    struct Variable;
    struct Assignment;
    struct Expression;
    struct ExpressionList;
    struct BinaryExpression;
    struct UnaryExpression;
    struct PrefixExpression;
    struct FunctionName;
    struct FunctionBody;
    struct FunctionDefinition;
    struct Arg;
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

    using VariablePtr = std::shared_ptr<Variable>;
    using AssignmentPtr = std::shared_ptr<Assignment>;
    using ExpressionPtr = std::shared_ptr<Expression>;
    using ExpressionListPtr = std::shared_ptr<ExpressionList>;
    using BinaryExpressionPtr = std::shared_ptr<BinaryExpression>;
    using UnaryExpressionPtr = std::shared_ptr<UnaryExpression>;
    using PrefixExpressionPtr = std::shared_ptr<PrefixExpression>;
    using FunctionNamePtr = std::shared_ptr<FunctionName>;
    using FunctionBodyPtr = std::shared_ptr<FunctionBody>;
    using FunctionDefinitionPtr = std::shared_ptr<FunctionDefinition>;
    using ArgPtr = std::shared_ptr<Arg>;
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
        std::wstring name;

        void print()
        {
            std::wcout << "print Name" << std::endl;
        }
    };

    struct NameList
    {
        std::vector<Name> names;

        void print()
        {
            std::wcout << "print NameList" << std::endl;
        }
    };

    struct Numeral
    {
        double value;

        void print()
        {
            std::wcout << "print Numeral" << std::endl;
        }
    };

    struct Boolean
    {
        bool value;

        Boolean& operator=(const bool inVal)
        {
            value = inVal;
            return *this;
        }

        void print()
        {
            std::wcout << "print Boolean" << std::endl;
        }
    };

    struct LiteralString
    {
        std::wstring value;

        void print()
        {
            std::wcout << "print LiteralString" << std::endl;
        }
    };

    struct UnaryOperator
    {
        std::wstring value;

        void print()
        {
            std::wcout << "print UnaryOperator" << std::endl;
        }
    };

    struct BinaryOperator
    {
        std::wstring value;

        void print()
        {
            std::wcout << "print BinaryOperator" << std::endl;
        }
    };


    struct Chunk
    {
        BlockPtr block;

        Chunk(const BlockPtr& _block)
        {
            std::wcout << "chunk" << std::endl;
        }
        void print()
        {
            std::wcout << "print chunk" << std::endl;
        }
    };

    struct Block
    {
        std::vector<StatementPtr> statements;
        //StatReturnPtr statReturn;

        void print()
        {
            std::wcout << "print block" << std::endl;
        }
    };

    struct Variable
    {
        Name name;
    };

    struct Assignment
    {
        VariablePtr variable;
        ExpressionPtr expression;
    };

    struct Expression
    {
        std::variant<
            Boolean, 
            Numeral, 
            LiteralString, 
            FunctionDefinitionPtr,
            BinaryExpressionPtr, 
            UnaryExpressionPtr> expression;

        void print()
        {
            std::wcout << "print Expression" << std::endl;
        }
    };

    struct ExpressionList
    {
        std::vector<ExpressionPtr> expressions;
    };

    struct BinaryExpression
    {
        ExpressionPtr expression1;
        BinaryOperator binaryOperator;
        ExpressionPtr expression2;
    };

    struct UnaryExpression
    {
        UnaryOperator unaryOperator;
        ExpressionPtr expression;
    };

    struct PrefixExpression
    {
        std::variant<Variable, FunctionCallPtr, ExpressionPtr> prefixExpression;
    };

    struct FunctionName
    {
        Name name;
    };

    struct FunctionBody
    {
        NameList params;
        BlockPtr block;
    };

    struct FunctionDefinition
    {
        FunctionNamePtr functionName;
        FunctionBodyPtr functionBody;
    };

    struct Arg
    {
        ExpressionListPtr expressionList;
    };

    struct FunctionCall
    {
        PrefixExpressionPtr prefixExpression;
        ArgPtr arg;
    };


    struct Statement
    {
        std::variant<
            AssignmentPtr,
            FunctionCallPtr,
            StatBreakPtr,
            StatGotoPtr,
            StatDoPtr,
            StatWhilePtr,
            StatRepeatPtr,
            StatIfPtr,
            StatForPtr> statement;

        void print()
        {
            std::wcout << "print statement" << std::endl;
        }
    };

    struct StatReturn 
    {
        std::vector<ExpressionPtr> expressions;
    };

    struct StatBreak 
    {
        std::wstring value;
    };

    struct StatGoto
    {
        Name name;
    };

    struct StatDo
    {
        BlockPtr block;
    };

    struct StatWhile
    {
        ExpressionPtr expression;
        StatDoPtr statDo;
    };

    struct StatRepeat
    {
        BlockPtr block;
        ExpressionPtr expression;
    };

    struct StatIf
    {
        ExpressionPtr expression;
        BlockPtr block;
        StatIfPtr statIf;

        void print()
        {
            std::wcout << "print statif" << std::endl;
        }
    };

    struct StatFor
    {
        Name name;
        ExpressionPtr expression1;
        ExpressionPtr expression2;
        ExpressionPtr expression3;
    };

    struct StatFunction
    {
        FunctionNamePtr functionName;
        FunctionBodyPtr functionBody;
    };

    void print(const std::wstring str);


}
