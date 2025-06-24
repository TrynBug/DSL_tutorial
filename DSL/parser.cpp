#define BOOST_SPIRIT_DEBUG

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_lexeme.hpp>
#include <boost/spirit/include/qi_char_.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_skip.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_auto.hpp>
#include <boost/phoenix.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <variant>
#include <thread>
#include <chrono>

#include "ast.h"

#include "parser.h"



using namespace boost::spirit;

using qi::char_;
using qi::int_;
using qi::double_;
using qi::lexeme;
using qi::lit;

using qi::ascii::space;
using qi::ascii::alpha;
using qi::ascii::alnum;


// '공백'과 '-- 주석'을 무시하도록 스키퍼 정의
template <typename Iterator>
struct skipper : qi::grammar<Iterator> 
{
    skipper() : skipper::base_type(skip) 
    {
        skip = space | ("--" >> *(char_ - '\n') >> -char_('\n'));
    }
   qi::rule<Iterator> skip;
};












using namespace boost::phoenix;
using boost::phoenix::push_back;
using boost::phoenix::ref;
using boost::phoenix::construct;
using boost::phoenix::new_;
using boost::phoenix::bind;

void lua::print(const std::string str)
{
    std::cout << str << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


BOOST_FUSION_ADAPT_STRUCT(lua::Name, name)
BOOST_FUSION_ADAPT_STRUCT(lua::NameList, names)
BOOST_FUSION_ADAPT_STRUCT(lua::Numeral, value)
BOOST_FUSION_ADAPT_STRUCT(lua::Boolean, value)
BOOST_FUSION_ADAPT_STRUCT(lua::LiteralString, value)
BOOST_FUSION_ADAPT_STRUCT(lua::UnaryOperator, value)
BOOST_FUSION_ADAPT_STRUCT(lua::BinaryOperator, value)
BOOST_FUSION_ADAPT_STRUCT(lua::Chunk, block)
//BOOST_FUSION_ADAPT_STRUCT(lua::Block, statements, statReturn)
//BOOST_FUSION_ADAPT_STRUCT(lua::Block, statements)
BOOST_FUSION_ADAPT_STRUCT(lua::Block, block)
BOOST_FUSION_ADAPT_STRUCT(lua::Assignment, name, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::Expression, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::ExpressionList, expressions)
BOOST_FUSION_ADAPT_STRUCT(lua::PrimaryExpression, primaryExpression)
BOOST_FUSION_ADAPT_STRUCT(lua::BinaryExpression, primaryExpression1, binaryOperator, primaryExpression2)
BOOST_FUSION_ADAPT_STRUCT(lua::UnaryExpression, unaryOperator, primaryExpression)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionName, name)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionBody, params, block)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionDefinition, functionBody, name)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionArguments, expressionList)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionCall, name, functionArguments)
BOOST_FUSION_ADAPT_STRUCT(lua::Statement, statement)
BOOST_FUSION_ADAPT_STRUCT(lua::StatReturn, expressions)
BOOST_FUSION_ADAPT_STRUCT(lua::StatBreak, value)
BOOST_FUSION_ADAPT_STRUCT(lua::StatGoto, name)
BOOST_FUSION_ADAPT_STRUCT(lua::StatDo, block)
BOOST_FUSION_ADAPT_STRUCT(lua::StatWhile, expression, statDo)
BOOST_FUSION_ADAPT_STRUCT(lua::StatRepeat, block, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::StatIf, expression, block, statIf)
BOOST_FUSION_ADAPT_STRUCT(lua::StatFor, name, expression1, expression2, expression3)
BOOST_FUSION_ADAPT_STRUCT(lua::StatFunction, functionName, functionBody)


// lua 파서 구현
template <typename Iterator>
struct lua_grammar : qi::grammar<Iterator, lua::ChunkPtr(), skipper<Iterator>> {
    lua_grammar() : lua_grammar::base_type(ruleChunk) {

        symbolKeyword.add("and", "and")("or", "or")("not", "not")("break", "break")("goto", "goto")("do", "do")("end", "end")("while", "while")
                    ("repeat", "repeat")("return", "return")("until", "until")("if", "if")("then", "then")("elseif", "elseif")("else", "else")
                    ("for", "for")("in", "in")("function", "function")("local", "local")("false", "false")("true", "true");

        symbolUnaryOperator.add("-", "-")("not", "not")("#", "#")("~", "~");

        symbolBinaryOperator.add("+", "+")("-", "-")("*", "*")("/", "/")("//", "//")("^", "^")("%", "%")("&", "&")("~", "~")("|", "|")(">>", ">>")("<<", "<<")
                                ("..", "..")("<", "<")("<=", "<=")(">", ">")(">=", ">=")("==", "==")("~=", "~=")("and", "or");

        ruleName                %= lexeme[!symbolKeyword >> qi::raw[qi::alpha >> *(qi::alnum | '_')]];
        ruleNumeral             = qi::int_ [_val = construct<lua::Numeral>(_1)];
        ruleBoolean             = lit("false")[_val = construct<lua::Boolean>(false)] | lit("true")[_val = construct<lua::Boolean>(true)];
        ruleLiteralString       %= lexeme['"' >> *(qi::char_ - '"') >> '"'];
        ruleBinaryOperator      %= symbolBinaryOperator;
        ruleUnaryOperator       %= symbolUnaryOperator;
        ruleNameList            = (ruleName % ',')[_val = construct<lua::NameList>(_1)];

        ruleAssignment          = (ruleName >> lit('=') >> ruleExpression)[_val = construct<lua::AssignmentPtr>(new_<lua::Assignment>(_1, _2))];

        ruleTypeExpression      = rulePrimaryExpression | ruleUnaryExpression | ruleBinaryExpression ;
        ruleExpression          = ruleTypeExpression[_val = bind([](const lua::TypeExpression& val) { return std::make_shared<lua::Expression>(val); }, _1)];   // 이렇게 안하면 ruleTypeExpression 에서 마지막 rule이 매칭됐을때만 값이 들어감
        ruleExpressionList      = (ruleExpression % ',')[_val = construct<lua::ExpressionListPtr>(new_<lua::ExpressionList>(_1))];

        ruleTypePrimaryExpression = ruleName | ruleNumeral | ruleLiteralString | ruleBoolean | (lit('(') >> ruleExpression >> lit(')'));
        rulePrimaryExpression   = ruleTypePrimaryExpression[_val = bind([](const lua::TypePrimaryExpression& val) { return std::make_shared<lua::PrimaryExpression>(val); }, _1)];
        ruleBinaryExpression    = (rulePrimaryExpression >> ruleBinaryOperator >> rulePrimaryExpression) [_val = construct<lua::BinaryExpressionPtr>(new_<lua::BinaryExpression>(_1, _2, _3))];
        ruleUnaryExpression     = (ruleUnaryOperator >> rulePrimaryExpression) [_val = construct<lua::UnaryExpressionPtr>(new_<lua::UnaryExpression>(_1, _2))];
        
        ruleFunctionDefinition = (lit("function") >> ruleName >> ruleFunctionBody)[_val = construct<lua::FunctionDefinitionPtr>(new_<lua::FunctionDefinition>(_1, _2))];
        ruleFunctionName       = ruleName[_val = construct<lua::FunctionNamePtr>(new_<lua::FunctionName>(_1))];
        ruleFunctionBody       = (lit('(') >> ruleNameList >> lit(')') >> ruleBlock >> lit("end"))[_val = construct<lua::FunctionBodyPtr>(new_<lua::FunctionBody>(_1, _2))];
        ruleFunctionArguments   =   (lit('(') >> lit(')'))[_val = construct<lua::FunctionArgumentsPtr>(new_<lua::FunctionArguments>())]
                                  | (lit('(') >> ruleExpressionList >> lit(')'))[_val = construct<lua::FunctionArgumentsPtr>(new_<lua::FunctionArguments>(_1))];
        ruleFunctionCall       = (ruleName >> ruleFunctionArguments)[_val = construct<lua::FunctionCallPtr>(new_<lua::FunctionCall>(_1, _2))];

        ruleTypeStatement      = ruleStatIf | ruleFunctionDefinition | ruleFunctionCall | ruleAssignment;
        ruleStatement          = ruleTypeStatement[_val = bind([](const lua::TypeStatement& val) { return std::make_shared<lua::Statement>(val); }, _1)];
        ruleStatReturn;
        ruleStatBreak;
        ruleStatGoto;
        ruleStatDo;
        ruleStatWhile;
        ruleStatRepeat;
        ruleStatIf             = (lit("if") >> ruleExpression >> lit("then") >> ruleBlock >> lit("end"))[_val = construct<lua::StatIfPtr>(new_<lua::StatIf>(_1, _2, nullptr))];
        ruleStatFor;
        ruleStatFunction;

        ruleTypeBlock          = ruleStatement >> *ruleStatement;
        ruleBlock               = ruleTypeBlock[_val = bind([](const lua::TypeBlock& val) { return std::make_shared<lua::Block>(val); }, _1)];
        // 현재 문제점: statement가 여러개여도 마지막 1개만 들어감
        //ruleBlock = +ruleStatement [_val = construct<lua::BlockPtr>(new_<lua::Block>(_1))];
        //ruleBlock = qi::eps[_val = construct<lua::BlockPtr>(new_<lua::Block>())] >> +ruleStatement[push_back(bind(&lua::Block::statements, *_val), _1)];
        //ruleBlock = ruleStatement[_val = construct<lua::BlockPtr>(new_<lua::Block>(_1))] >> ruleStatement[push_back(bind(&lua::Block::statements, *_val), _1)] ;
        //ruleBlock = ruleStatement[_val = bind([](const lua::StatementPtr& val) { return std::make_shared<lua::Block>(val); }, _1)];
        ruleChunk = ruleBlock[_val = construct<lua::ChunkPtr>(new_<lua::Chunk>(_1))];



        //qi::on_success(ruleBlock, [this](auto& context) {
        //    lua::BlockPtr block = context.get<lua::BlockPtr>();
        //    //static_cast<lua::BlockPtr>(_val(ctx))->statements.push_back(_attr(ctx));
        //    });

        BOOST_SPIRIT_DEBUG_NODES((ruleName)(ruleNumeral)(ruleBoolean)(ruleLiteralString)(ruleBinaryOperator)(ruleUnaryOperator)(ruleNameList));
        BOOST_SPIRIT_DEBUG_NODES((ruleAssignment)(ruleExpression)(ruleExpressionList)(rulePrimaryExpression)(ruleBinaryExpression)(ruleUnaryExpression));
        BOOST_SPIRIT_DEBUG_NODES((ruleFunctionDefinition)(ruleFunctionName)(ruleFunctionBody)(ruleFunctionArguments)(ruleFunctionCall));
        BOOST_SPIRIT_DEBUG_NODES((ruleStatement)(ruleStatReturn)(ruleStatBreak)(ruleStatGoto)(ruleStatDo)(ruleStatWhile)(ruleStatRepeat)(ruleStatIf)(ruleStatFor)(ruleStatFunction));
        BOOST_SPIRIT_DEBUG_NODES((ruleBlock)(ruleChunk));
    }


    // 타입별 파서
    qi::symbols<char, std::string> symbolKeyword;
    qi::symbols<char, std::string> symbolUnaryOperator;
    qi::symbols<char, std::string> symbolBinaryOperator;

    qi::rule<Iterator, lua::Name(), skipper<Iterator>> ruleName;
    qi::rule<Iterator, lua::Numeral(), skipper<Iterator>> ruleNumeral;
    qi::rule<Iterator, lua::Boolean(), skipper<Iterator>> ruleBoolean;
    qi::rule<Iterator, lua::LiteralString(), skipper<Iterator>> ruleLiteralString;
    qi::rule<Iterator, lua::BinaryOperator(), skipper<Iterator>> ruleBinaryOperator;
    qi::rule<Iterator, lua::UnaryOperator(), skipper<Iterator>> ruleUnaryOperator;
    qi::rule<Iterator, lua::NameList(), skipper<Iterator>> ruleNameList;

   
    qi::rule<Iterator, lua::AssignmentPtr(), skipper<Iterator>> ruleAssignment;

    qi::rule<Iterator, lua::TypeExpression(), skipper<Iterator>> ruleTypeExpression;
    qi::rule<Iterator, lua::ExpressionPtr(), skipper<Iterator>> ruleExpression;
    qi::rule<Iterator, lua::ExpressionListPtr(), skipper<Iterator>> ruleExpressionList;
    qi::rule<Iterator, lua::TypePrimaryExpression(), skipper<Iterator>> ruleTypePrimaryExpression;
    qi::rule<Iterator, lua::PrimaryExpressionPtr(), skipper<Iterator>> rulePrimaryExpression;
    qi::rule<Iterator, lua::BinaryExpressionPtr(), skipper<Iterator>> ruleBinaryExpression;
    qi::rule<Iterator, lua::UnaryExpressionPtr(), skipper<Iterator>> ruleUnaryExpression;
    
    qi::rule<Iterator, lua::FunctionDefinitionPtr(), skipper<Iterator>> ruleFunctionDefinition;
    qi::rule<Iterator, lua::FunctionNamePtr(), skipper<Iterator>> ruleFunctionName;
    qi::rule<Iterator, lua::FunctionBodyPtr(), skipper<Iterator>> ruleFunctionBody;
    qi::rule<Iterator, lua::FunctionArgumentsPtr(), skipper<Iterator>> ruleFunctionArguments;
    qi::rule<Iterator, lua::FunctionCallPtr(), skipper<Iterator>> ruleFunctionCall;
    
    qi::rule<Iterator, lua::TypeStatement(), skipper<Iterator>> ruleTypeStatement;
    qi::rule<Iterator, lua::StatementPtr(), skipper<Iterator>> ruleStatement;
    qi::rule<Iterator, lua::StatReturnPtr(), skipper<Iterator>> ruleStatReturn;
    qi::rule<Iterator, lua::StatBreakPtr(), skipper<Iterator>> ruleStatBreak;
    qi::rule<Iterator, lua::StatGotoPtr(), skipper<Iterator>> ruleStatGoto;
    qi::rule<Iterator, lua::StatDoPtr(), skipper<Iterator>> ruleStatDo;
    qi::rule<Iterator, lua::StatWhilePtr(), skipper<Iterator>> ruleStatWhile;
    qi::rule<Iterator, lua::StatRepeatPtr(), skipper<Iterator>> ruleStatRepeat;
    qi::rule<Iterator, lua::StatIfPtr(), skipper<Iterator>> ruleStatIf;
    qi::rule<Iterator, lua::StatForPtr(), skipper<Iterator>> ruleStatFor;
    qi::rule<Iterator, lua::StatFunctionPtr(), skipper<Iterator>> ruleStatFunction;

    qi::rule<Iterator, lua::TypeBlock(), skipper<Iterator>> ruleTypeBlock;
    qi::rule<Iterator, lua::BlockPtr(), skipper<Iterator>> ruleBlock;
    qi::rule<Iterator, lua::ChunkPtr(), skipper<Iterator>> ruleChunk;
};



void ParserTest()
{
    std::string filePath = "Script/dsl1.dsl";
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::wcout << L"파일 열기 실패" << std::endl;
        return;
    }

    std::string input;
    std::string line;
    while (std::getline(file, line))
        input += line + "\n";


    using It = std::string::iterator;
    It iter = input.begin(), end = input.end();

    skipper<It> skip;


    //mini_xml prog;
    //mini_xml_grammar<It> grammar;

    lua::ChunkPtr prog;
    //lua::ExpressionPtr prog;
    lua_grammar<It> grammar;
    
    

    bool r = phrase_parse(iter, end, grammar, skip, prog);

    if (r && iter == end) {
        std::wcout << "AST 파싱 성공!" << std::endl;
        //std::cout << "문장 수: " << prog.statements.size() << std::endl;

        prog->Print();
    }
    else {
        std::wcout << "파싱 실패..." << std::endl;
    }
}