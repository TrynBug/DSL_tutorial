#include "pch.h"

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

BOOST_FUSION_ADAPT_STRUCT(dsl::Name, name)
BOOST_FUSION_ADAPT_STRUCT(dsl::NameList, names)
BOOST_FUSION_ADAPT_STRUCT(dsl::Numeral, isInteger, intValue, floatValue)
BOOST_FUSION_ADAPT_STRUCT(dsl::Boolean, value)
BOOST_FUSION_ADAPT_STRUCT(dsl::LiteralString, value)
BOOST_FUSION_ADAPT_STRUCT(dsl::AST, block)
BOOST_FUSION_ADAPT_STRUCT(dsl::Block, statements)
BOOST_FUSION_ADAPT_STRUCT(dsl::Assignment, name, expression)
BOOST_FUSION_ADAPT_STRUCT(dsl::Expression, expression)
BOOST_FUSION_ADAPT_STRUCT(dsl::ExpressionList, expressions)
BOOST_FUSION_ADAPT_STRUCT(dsl::PrimaryExpression, primaryExpression)
BOOST_FUSION_ADAPT_STRUCT(dsl::BinaryExpression, primaryExpression1, binaryOperator, primaryExpression2)
BOOST_FUSION_ADAPT_STRUCT(dsl::UnaryExpression, unaryOperator, primaryExpression)
BOOST_FUSION_ADAPT_STRUCT(dsl::FunctionDefinition, name, functionParameter, block)
BOOST_FUSION_ADAPT_STRUCT(dsl::FunctionParameter, nameList)
BOOST_FUSION_ADAPT_STRUCT(dsl::FunctionArgument, expressionList)
BOOST_FUSION_ADAPT_STRUCT(dsl::FunctionCall, name, functionArgument)
BOOST_FUSION_ADAPT_STRUCT(dsl::Statement, statement)
BOOST_FUSION_ADAPT_STRUCT(dsl::Return, expressions)
BOOST_FUSION_ADAPT_STRUCT(dsl::Break, value)
BOOST_FUSION_ADAPT_STRUCT(dsl::While, expression, statDo)
BOOST_FUSION_ADAPT_STRUCT(dsl::If, expression, block, statIf)
BOOST_FUSION_ADAPT_STRUCT(dsl::For, name, expression1, expression2, expression3)

namespace dsl
{

// '공백'과 '-- 주석'을 무시하도록 스키퍼 정의
template <typename Iterator>
struct skipper : qi::grammar<Iterator> 
{
    skipper() : skipper::base_type(skip) 
    {
        skip = space | ("--" >> *(qi::char_ - '\n') >> -qi::char_('\n'));
    }
   qi::rule<Iterator> skip;
};

// dsl 파서 구현
template <typename Iterator>
struct dsl_grammar : qi::grammar<Iterator, ASTPtr(), skipper<Iterator>> {
    dsl_grammar() : dsl_grammar::base_type(ruleAST) {

        // 키워드 정의
        symbolKeyword.add("and", "and")("or", "or")("not", "not")("break", "break")("goto", "goto")("do", "do")("end", "end")("while", "while")
            ("repeat", "repeat")("return", "return")("until", "until")("if", "if")("then", "then")("elseif", "elseif")("else", "else")
            ("for", "for")("in", "in")("function", "function")("local", "local")("false", "false")("true", "true");

        // 1순위 단항 연산자 정의
        symbol1OperatorUnary.add("not", "not")("-", "-");

        // 2순위 이항 연산자 정의
        symbol2OperatorBinary.add("*", "*")("/", "/")("%", "%");

        // 3순위 이항 연산자 정의
        symbol3OperatorBinary.add("+", "+")("-", "-");

        // 4순위 이항 연산자 정의
        symbol4OperatorBinary.add("<", "<")(">", ">")("<=", "<=")(">=", ">=");

        // 5순위 이항 연산자 정의
        symbol5OperatorBinary.add("==", "==")("!=", "!=");

        // 6순위 이항 연산자 정의
        symbol6OperatorBinary.add("and", "and");

        // 7순위 이항 연산자 정의
        symbol7OperatorBinary.add("or", "or");

        // 8순위 단항 연산자 정의
        symbol8OperatorBinary.add("*=", "*=")("/=", "/=")("+=", "+=")("-=", "-=");


        // 이름(식별자) 규칙
        // 첫 글자는 알파벳 또는 언더스코어. 나머지 글자는 알파벳 or 숫자 or 언더스코어. 그리고 symbolKeyword에 포함되는 이름이면 안됨.
        ruleName = lexeme[qi::as<std::wstring>()[(qi::alpha | qi::char_('_')) >> *(qi::alnum | qi::char_('_'))] - (symbolKeyword >> !(qi::alnum | qi::char_('_')))][_val = construct<BasePtr>(new_<Name>(_1))];

        // 이름 리스트 규칙
        // 콤마로 구분되는 이름 리스트
        ruleNameList = (ruleName % ',')[_val = construct<BasePtr>(new_<NameList>(_1))];

        // bool 규칙
        // true 또는 false
        ruleBoolean = lit("false")[_val = construct<BasePtr>(new_<Boolean>(false))] 
                    | lit("true")[_val = construct<BasePtr>(new_<Boolean>(true))];

        // 문자열 규칙
        // " " 로 둘러쌓인 문자열
        ruleLiteralString = lexeme[qi::as<std::wstring>()['"' >> *(qi::char_ - '"') >> '"']][_val = construct<BasePtr>(new_<LiteralString>(_1))];

        // 정수 규칙
        ruleInteger = qi::long_long[_val = construct<BasePtr>(new_<Numeral>(_1))];

        // 실수 규칙
        ruleFloat = qi::double_[_val = construct<BasePtr>(new_<Numeral>(_1))];

        // 숫자 규칙
        // 실수를 먼저 시도하고, 실패하면 정수로 파싱. 이는 "3.14"와 같은 실수가 "3"과 "14"로 분리되는 것을 방지
        // 현재 문제: 모든 숫자가 실수로 파싱됨.
        ruleNumeral = ruleFloat | ruleInteger;

        // 최하위 표현식 규칙
        // 이름, 숫자, bool, 문자열, 괄호로 둘러쌓인 표현식
        // 참고: 순서가 유의미함. FunctionCall은 Name 앞에 있어야 test() 같은 문자열이 함수호출로 제대로 인식됨. 그리고 "(ruleExpression)" 은 가장 마지막에 있어야 함.
        rulePrimaryExpression =   ruleNumeral[_val = _1] 
                                | ruleBoolean[_val = _1]
                                | ruleLiteralString[_val = _1]
                                | ruleFunctionCall[_val = _1]
                                | ruleName[_val = _1]
                                | (lit('(') >> ruleExpression >> lit(')'))[_val = _1];

        // 1순위 단항연산자 표현식 규칙
        // 단항연산자 표현식을 먼저 검사하고, 매칭되지 않는다면 최하위 표현식인지 검사한다.
        rule1OperatorUnary = (qi::as<std::wstring>()[symbol1OperatorUnary] >> rulePrimaryExpression)[_val = construct<BasePtr>(new_<UnaryExpression>(_1, _2))]
                                | rulePrimaryExpression[_val = _1];

        // 우선순위별 이항연산자 표현식 규칙
        // 각각의 규칙은 자신보다 우선순위가 높은 규칙을 먼저 검사한다. 우선순위가 가장 높은 규칙이 가장 먼저 검사되도록 하기 위해서이다.
        rule2OperatorBinary = rule1OperatorUnary[_val = _1]  >> *(qi::as<std::wstring>()[symbol2OperatorBinary] >> rule1OperatorUnary) [_val = construct<BasePtr>(new_<BinaryExpression>(_val, _1, _2))];
        rule3OperatorBinary = rule2OperatorBinary[_val = _1] >> *(qi::as<std::wstring>()[symbol3OperatorBinary] >> rule2OperatorBinary)[_val = construct<BasePtr>(new_<BinaryExpression>(_val, _1, _2))];
        rule4OperatorBinary = rule3OperatorBinary[_val = _1] >> *(qi::as<std::wstring>()[symbol4OperatorBinary] >> rule3OperatorBinary)[_val = construct<BasePtr>(new_<BinaryExpression>(_val, _1, _2))];
        rule5OperatorBinary = rule4OperatorBinary[_val = _1] >> *(qi::as<std::wstring>()[symbol5OperatorBinary] >> rule4OperatorBinary)[_val = construct<BasePtr>(new_<BinaryExpression>(_val, _1, _2))];
        rule6OperatorBinary = rule5OperatorBinary[_val = _1] >> *(qi::as<std::wstring>()[symbol6OperatorBinary] >> rule5OperatorBinary)[_val = construct<BasePtr>(new_<BinaryExpression>(_val, _1, _2))];
        rule7OperatorBinary = rule6OperatorBinary[_val = _1] >> *(qi::as<std::wstring>()[symbol7OperatorBinary] >> rule6OperatorBinary)[_val = construct<BasePtr>(new_<BinaryExpression>(_val, _1, _2))];
        rule8OperatorBinary = rule7OperatorBinary[_val = _1] >> *(qi::as<std::wstring>()[symbol8OperatorBinary] >> rule7OperatorBinary)[_val = construct<BasePtr>(new_<BinaryExpression>(_val, _1, _2))];

        // 표현식 규칙
        // 표현식은 단항 연산자, 이항 연산자, 이름, 숫자, bool, 함수 호출 등을 말한다.
        ruleExpression = rule8OperatorBinary[_val = _1];

        // 표현식 리스트 규칙
        // 콤마로 구분되는 표현식 리스트
        ruleExpressionList = (ruleExpression % ',')[_val = construct<BasePtr>(new_<ExpressionList>(_1))];

        // 함수 선언 규칙
        // "function 함수이름 함수파라미터 함수본문 end"
        ruleFunctionDefinition = (lit("function") >> ruleName >> ruleFunctionParameter >> ruleBlock >> lit("end"))[_val = construct<BasePtr>(new_<FunctionDefinition>(_1, _2, _3))];

        // 함수 파라미터 규칙
        // "( )" 또는 "( 파라미터 리스트 )"
        ruleFunctionParameter = (lit('(') >> lit(')'))[_val = construct<BasePtr>(new_<FunctionParameter>())]
                              | (lit('(') >> ruleNameList >> lit(')'))[_val = construct<BasePtr>(new_<FunctionParameter>(_1))];

        // 함수 인자 규칙
        // "( )" 또는 "( 인자 리스트 )"
        ruleFunctionArgument = (lit('(') >> lit(')'))[_val = construct<BasePtr>(new_<FunctionArgument>())]
                              | (lit('(') >> ruleExpressionList >> lit(')'))[_val = construct<BasePtr>(new_<FunctionArgument>(_1))];

        // 함수 호출 규칙
        // "함수이름 함수인자"
        ruleFunctionCall = (ruleName >> ruleFunctionArgument)[_val = construct<BasePtr>(new_<FunctionCall>(_1, _2))];
        
        // 변수 할당 규칙
        // "변수이름 = 표현식"
        ruleAssignment = (ruleName >> lit('=') >> ruleExpression)[_val = construct<BasePtr>(new_<Assignment>(_1, _2))];

        // If문 규칙
        ruleIf = (lit("if") >> ruleExpression >> lit("then") >> ruleBlock >> lit("end"))[_val = construct<BasePtr>(new_<If>(_1, _2, nullptr))];

        // 구문 규칙
        // 구문은 if문, for문, 함수선언, 변수선언, 표현식 등을 말한다.
        ruleStatement =  ruleFunctionDefinition[_val = _1]
                       | ruleAssignment[_val = _1]
                       | ruleIf[_val = _1]
                       | ruleExpression[_val = _1];

        // 블록 규칙
        // 블록은 구문이 1개 이상 나열되어 있는것을 말한다.
        ruleBlock = (ruleStatement % qi::eps)[_val = construct<BasePtr>(new_<Block>(_1))];

        // 프로그램
        ruleAST = ruleBlock[_val = construct<ASTPtr>(new_<AST>(_1))];

        BOOST_SPIRIT_DEBUG_NODES((ruleName)(ruleNameList)(ruleBoolean)(ruleLiteralString)(ruleInteger)(ruleFloat)(ruleNumeral));
        BOOST_SPIRIT_DEBUG_NODES((rulePrimaryExpression)(ruleExpression)(ruleExpressionList));
        BOOST_SPIRIT_DEBUG_NODES((rule1OperatorUnary)(rule2OperatorBinary)(rule3OperatorBinary)(rule4OperatorBinary)(rule5OperatorBinary)(rule6OperatorBinary)(rule7OperatorBinary)(rule8OperatorBinary));
        BOOST_SPIRIT_DEBUG_NODES((ruleFunctionDefinition)(ruleFunctionParameter)(ruleFunctionArgument)(ruleFunctionCall));
        BOOST_SPIRIT_DEBUG_NODES((ruleAssignment)(ruleIf)(ruleStatement));
        BOOST_SPIRIT_DEBUG_NODES((ruleBlock)(ruleAST));
    }


    /* 심볼(키워드) */
    qi::symbols<char, std::string> symbolKeyword;
    qi::symbols<char, std::string> symbol1OperatorUnary;
    qi::symbols<char, std::string> symbol2OperatorBinary;
    qi::symbols<char, std::string> symbol3OperatorBinary;
    qi::symbols<char, std::string> symbol4OperatorBinary;
    qi::symbols<char, std::string> symbol5OperatorBinary;
    qi::symbols<char, std::string> symbol6OperatorBinary;
    qi::symbols<char, std::string> symbol7OperatorBinary;
    qi::symbols<char, std::string> symbol8OperatorBinary;

    /* 파싱 rule */
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleName;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleNameList;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleBoolean;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleLiteralString;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleInteger;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleFloat;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleNumeral;

    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rule1OperatorUnary;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rule2OperatorBinary;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rule3OperatorBinary;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rule4OperatorBinary;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rule5OperatorBinary;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rule6OperatorBinary;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rule7OperatorBinary;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rule8OperatorBinary;

    qi::rule<Iterator, BasePtr(), skipper<Iterator>> rulePrimaryExpression;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleExpression;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleExpressionList;

    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleFunctionDefinition;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleFunctionParameter;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleFunctionArgument;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleFunctionCall;
    
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleAssignment;
    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleIf;

    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleStatement;

    qi::rule<Iterator, BasePtr(), skipper<Iterator>> ruleBlock;
    qi::rule<Iterator, ASTPtr(), skipper<Iterator>> ruleAST;
};



void dsl::Parser2Test()
{
    std::wstring filePath = L"D:/project/DSL_cursor/Script/dsl1.dsl";
    std::wifstream file(filePath);
    if (!file.is_open())
    {
        std::wcout << L"Failed to open file: " << filePath.c_str() << std::endl;
        return;
    }

    std::wstring input;
    std::wstring line;
    while (std::getline(file, line))
        input += line + L"\n";

    std::wcout << L"Input file content:" << std::endl;
    std::wcout << input.c_str() << std::endl;

    using It = std::wstring::iterator;
    It iter = input.begin(), end = input.end();

    skipper<It> skip;

    ASTPtr prog;
    dsl_grammar<It> grammar;
    

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

ASTPtr dsl::ParseScript(std::wstring& strScript)
{
    skipper<std::wstring::iterator> skip;

    ASTPtr prog;
    dsl_grammar<std::wstring::iterator> grammar;


    std::wstring::iterator iter = strScript.begin();
    bool r = phrase_parse(strScript.begin(), strScript.end(), grammar, skip, prog);

    if (r && iter == strScript.end()) {
        std::wcout << L"AST Parsing Success!" << std::endl;

        prog->Print();

        return prog;
    }
    else 
    {
        std::wcout << L"Parsing Failed..." << std::endl;
        std::wcout << L"Parsing Success: " << (r ? L"true" : L"false") << std::endl;
        std::wcout << L"Parsing Location: " << (iter - strScript.begin()) << L"/" << strScript.length() << std::endl;
        if (iter != strScript.end()) {
            std::wcout << L"Failed Location: ";
            for (int i = 0; i < 20 && iter != strScript.end(); ++i, ++iter) {
                std::wcout << *iter;
            }
            std::wcout << std::endl;
        }

        return nullptr;
    }

    return nullptr;
}

}