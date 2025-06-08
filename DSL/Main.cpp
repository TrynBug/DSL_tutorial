#include <windows.h>
#include <iostream>

#include "ast.h"
#include "parser.h"

#include "CDSL.h"



int wmain(int argc, wchar_t* argv[])
{
    
    lua::Boolean bo;
    lua::ExpressionPtr exp = std::make_shared<lua::Expression>(bo);
    lua::ExpressionPtr exp2 = std::make_shared<lua::Expression>(bo);
    lua::BinaryOperator bop;

    lua::BinaryExpressionPtr sp = std::make_shared<lua::BinaryExpression>(exp, bop, exp2);

    


    ParserTest();
    return 0;
}