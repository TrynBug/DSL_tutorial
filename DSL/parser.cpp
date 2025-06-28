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

using namespace boost::phoenix;
using boost::phoenix::push_back;
using boost::phoenix::ref;
using boost::phoenix::construct;
using boost::phoenix::new_;
using boost::phoenix::bind;

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
BOOST_FUSION_ADAPT_STRUCT(lua::Block, statements)
BOOST_FUSION_ADAPT_STRUCT(lua::Assignment, name, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::Expression, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::ExpressionList, expressions)
BOOST_FUSION_ADAPT_STRUCT(lua::PrimaryExpression, primaryExpression)
BOOST_FUSION_ADAPT_STRUCT(lua::BinaryExpression, primaryExpression1, binaryOperator, primaryExpression2)
BOOST_FUSION_ADAPT_STRUCT(lua::UnaryExpression, unaryOperator, primaryExpression)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionName, name)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionDefinition, name, params, block)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionCall, name, functionArguments)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionArguments, expressions)
BOOST_FUSION_ADAPT_STRUCT(lua::Statement, statement)
BOOST_FUSION_ADAPT_STRUCT(lua::StatReturn, expressions)
BOOST_FUSION_ADAPT_STRUCT(lua::StatBreak, value)
BOOST_FUSION_ADAPT_STRUCT(lua::StatGoto, name)
BOOST_FUSION_ADAPT_STRUCT(lua::StatDo, block)
BOOST_FUSION_ADAPT_STRUCT(lua::StatWhile, expression, statDo)
BOOST_FUSION_ADAPT_STRUCT(lua::StatRepeat, block, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::StatIf, expression, block, statIf)
BOOST_FUSION_ADAPT_STRUCT(lua::StatFor, name, expression1, expression2, expression3)


// lua 파서 구현
template <typename Iterator>
struct lua_grammar : qi::grammar<Iterator, lua::ChunkPtr(), skipper<Iterator>> {
    lua_grammar() : lua_grammar::base_type(ruleChunk) {

        // 키워드 정의
        symbolKeyword.add("and", "and")("or", "or")("not", "not")("break", "break")("goto", "goto")("do", "do")("end", "end")("while", "while")
                    ("repeat", "repeat")("return", "return")("until", "until")("if", "if")("then", "then")("elseif", "elseif")("else", "else")
                    ("for", "for")("in", "in")("function", "function")("local", "local")("false", "false")("true", "true");

        // 단항 연산자 정의
        symbolUnaryOperator.add("-", "-")("not", "not")("#", "#")("~", "~");

        // 이항 연산자 정의
        symbolBinaryOperator.add("+", "+")("-", "-")("*", "*")("/", "/")("//", "//")("^", "^")("%", "%")("&", "&")("~", "~")("|", "|")(">>", ">>")("<<", "<<")
                                ("..", "..")("<", "<")("<=", "<=")(">", ">")(">=", ">=")("==", "==")("~=", "~=")("and", "or");

        // 이름 rule 정의. 이름은 영문자로 시작하고 영문자, 숫자, 밑줄로 이루어진다.
        ruleName                %= lexeme[!symbolKeyword >> qi::raw[qi::alpha >> *(qi::alnum | '_')]];
        // 숫자 rule 정의
        ruleNumeral             = qi::int_ [_val = construct<lua::Numeral>(_1)];
        // 불리언 rule 정의
        ruleBoolean             = lit("false")[_val = construct<lua::Boolean>(false)] | lit("true")[_val = construct<lua::Boolean>(true)];
        // 문자열 rule 정의
        ruleLiteralString       %= lexeme['"' >> *(qi::char_ - '"') >> '"'];
        // 이항 연산자 rule 정의
        ruleBinaryOperator      %= symbolBinaryOperator;
        // 단항 연산자 rule 정의
        ruleUnaryOperator       %= symbolUnaryOperator;
        // 이름 리스트 rule 정의. 콤마로 구분된 이름 리스트이다.
        ruleNameList            = (ruleName % ',')[_val = construct<lua::NameList>(_1)];

        // rule Assignment는 변수 할당을 의미한다. "변수명 = 표현식"
        ruleAssignment          = (ruleName >> lit('=') >> ruleExpression)[_val = construct<lua::AssignmentPtr>(new_<lua::Assignment>(_1, _2))];

        // rule Expression은 표현식을 의미한다. 표현식은 단항 연산자, 이항 연산자, 함수 호출, 변수 등이 있다.
        ruleTypeExpression      = rulePrimaryExpression | ruleUnaryExpression | ruleBinaryExpression ;
        ruleExpression          = ruleTypeExpression[_val = bind([](const lua::TypeExpression& val) { return std::make_shared<lua::Expression>(val); }, _1)];   // 이렇게 안하면 ruleTypeExpression 에서 마지막 rule이 매칭됐을때만 값이 들어감
        ruleExpressionList      = (ruleExpression % ',')[_val = construct<lua::ExpressionListPtr>(new_<lua::ExpressionList>(_1))];

        ruleTypePrimaryExpression = ruleName | ruleNumeral | ruleLiteralString | ruleBoolean | (lit('(') >> ruleExpression >> lit(')'));
        rulePrimaryExpression   = ruleTypePrimaryExpression[_val = bind([](const lua::TypePrimaryExpression& val) { return std::make_shared<lua::PrimaryExpression>(val); }, _1)];
        ruleBinaryExpression    = (rulePrimaryExpression >> ruleBinaryOperator >> rulePrimaryExpression) [_val = construct<lua::BinaryExpressionPtr>(new_<lua::BinaryExpression>(_1, _2, _3))];
        ruleUnaryExpression     = (ruleUnaryOperator >> rulePrimaryExpression) [_val = construct<lua::UnaryExpressionPtr>(new_<lua::UnaryExpression>(_1, _2))];
        
        // rule FunctionDefinition은 함수 정의를 의미한다. "function 함수이름 (파라미터 리스트) 함수본문 end"
        ruleFunctionDefinition = (lit("function") >> ruleName >> lit('(') >> ruleNameList >> lit(')') >> ruleBlock >> lit("end")) [_val = construct<lua::FunctionDefinitionPtr>(new_<lua::FunctionDefinition>(_1, _2, _3))];
        // rule FunctionName은 함수이름이다.
        ruleFunctionName       = ruleName[_val = construct<lua::FunctionNamePtr>(new_<lua::FunctionName>(_1))];
        // rule FunctionCall은 함수 호출을 의미한다. "함수이름(Argument 리스트)"
        ruleFunctionCall       = (ruleName >> ruleFunctionArguments)[_val = construct<lua::FunctionCallPtr>(new_<lua::FunctionCall>(_1, _2))];
        // rule FunctionArguments은 함수 호출인자를 의미한다. "(Argument 리스트)"
        ruleFunctionArguments =     (lit('(') >> lit(')'))[_val = construct<lua::FunctionArgumentsPtr>(new_<lua::FunctionArguments>())]
                                | (lit('(') >> ruleExpressionList >> lit(')'))[_val = construct<lua::FunctionArgumentsPtr>(new_<lua::FunctionArguments>(_1))];

        // rule Statement는 1개의 구문을 의미한다. 구문은 if문, for문, 변수선언, 함수선언, 함수호출 등이 있다.
        ruleTypeStatement      = ruleStatIf | ruleFunctionDefinition | ruleFunctionCall | ruleAssignment;
        ruleStatement          = ruleTypeStatement[_val = bind([](const lua::TypeStatement& val) { return std::make_shared<lua::Statement>(val); }, _1)];

        // rule If는 if문을 의미한다. "if 조건식 then 구문 end"
        ruleStatIf             = (lit("if") >> ruleExpression >> lit("then") >> ruleBlock >> lit("end"))[_val = construct<lua::StatIfPtr>(new_<lua::StatIf>(_1, _2, nullptr))];

        // rule Block은 1개 이상의 Statement가 연속된 것이다.
        ruleBlock = (ruleStatement % qi::eps) [_val = construct<lua::BlockPtr>(new_<lua::Block>(_1))];
        // ruel Chunk는 스크립트 전체를 의미하며 1개의 Block을 가진다.
        ruleChunk = ruleBlock[_val = construct<lua::ChunkPtr>(new_<lua::Chunk>(_1))];

        BOOST_SPIRIT_DEBUG_NODES((ruleName)(ruleNumeral)(ruleBoolean)(ruleLiteralString)(ruleBinaryOperator)(ruleUnaryOperator)(ruleNameList));
        BOOST_SPIRIT_DEBUG_NODES((ruleAssignment)(ruleExpression)(ruleExpressionList)(rulePrimaryExpression)(ruleBinaryExpression)(ruleUnaryExpression));
        BOOST_SPIRIT_DEBUG_NODES((ruleFunctionDefinition)(ruleFunctionName)(ruleFunctionCall)(ruleFunctionArguments));
        BOOST_SPIRIT_DEBUG_NODES((ruleStatement)(ruleStatReturn)(ruleStatBreak)(ruleStatGoto)(ruleStatDo)(ruleStatWhile)(ruleStatRepeat)(ruleStatIf)(ruleStatFor));
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
    qi::rule<Iterator, lua::FunctionCallPtr(), skipper<Iterator>> ruleFunctionCall;
    qi::rule<Iterator, lua::FunctionArgumentsPtr(), skipper<Iterator>> ruleFunctionArguments;
    
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

    qi::rule<Iterator, lua::BlockPtr(), skipper<Iterator>> ruleBlock;
    qi::rule<Iterator, lua::ChunkPtr(), skipper<Iterator>> ruleChunk;
};



void ParserTest()
{
    std::string filePath = "D:/project/DSL_cursor/Script/dsl1.dsl";
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::wcout << L"Failed to open file: " << filePath.c_str() << std::endl;
        return;
    }

    std::string input;
    std::string line;
    while (std::getline(file, line))
        input += line + "\n";

    std::wcout << L"Input file content:" << std::endl;
    std::wcout << input.c_str() << std::endl;

    using It = std::string::iterator;
    It iter = input.begin(), end = input.end();

    skipper<It> skip;

    lua::ChunkPtr prog;
    lua_grammar<It> grammar;
    

    bool r = phrase_parse(iter, end, grammar, skip, prog);

    if (r && iter == end) {
        std::wcout << L"AST Parsing Success!" << std::endl;

        prog->Print();
    }
    else {
        std::wcout << L"Parsing Failed..." << std::endl;
        std::wcout << L"Parsing Success: " << (r ? L"true" : L"false") << std::endl;
        std::wcout << L"Parsing Location: " << (iter - input.begin()) << L"/" << input.length() << std::endl;
        if (iter != end) {
            std::wcout << L"Failed Location: ";
            for (int i = 0; i < 20 && iter != end; ++i, ++iter) {
                std::wcout << *iter;
            }
            std::wcout << std::endl;
        }
    }
}