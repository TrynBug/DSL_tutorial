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


struct mini_xml;

using mini_xml_node = std::variant<boost::recursive_wrapper<mini_xml>, std::string>;

struct mini_xml
{
    std::string name;                           // tag name
    std::vector<mini_xml_node> children;        // children
};

BOOST_FUSION_ADAPT_STRUCT(
    mini_xml,
    (std::string, name)
    (std::vector<mini_xml_node>, children)
)

template <typename Iterator>
struct mini_xml_grammar
    : qi::grammar<Iterator, mini_xml(), qi::locals<std::string>, skipper<Iterator>>
{
    mini_xml_grammar()
        : mini_xml_grammar::base_type(xml, "xml")
    {
        using qi::lit;
        using qi::lexeme;
        using qi::on_error;
        using qi::fail;
        using ascii::char_;
        using ascii::string;
        using namespace qi::labels;

        using boost::phoenix::construct;
        using boost::phoenix::val;

        text %= lexeme[+(char_ - '<')];
        node %= xml | text;

        start_tag %=
            '<'
            >> !lit('/')
            > lexeme[+(char_ - '>')]
            > '>'
            ;

        end_tag =
            "</"
            > lit(_r1)
            > '>'
            ;

        xml %=
            start_tag[_a = _1]
            > *node
            > end_tag(_a)
            ;

        xml.name("xml");
        node.name("node");
        text.name("text");
        start_tag.name("start_tag");
        end_tag.name("end_tag");

        on_error<fail>
            (
                xml
                , std::cout
                << val("Error! Expecting ")
                << _4                               // what failed?
                << val(" here: \"")
                << construct<std::string>(_3, _2)   // iterators to error-pos, end
                << val("\"")
                << std::endl
            );
    }

    qi::rule<Iterator, mini_xml(), qi::locals<std::string>, skipper<Iterator>> xml;
    qi::rule<Iterator, mini_xml_node(), skipper<Iterator>> node;
    qi::rule<Iterator, std::string(), skipper<Iterator>> text;
    qi::rule<Iterator, std::string(), skipper<Iterator>> start_tag;
    qi::rule<Iterator, void(std::string), skipper<Iterator>> end_tag;
};






using namespace boost::phoenix;
using boost::phoenix::push_back;
using boost::phoenix::ref;
using boost::phoenix::construct;
using boost::phoenix::new_;
using boost::phoenix::bind;

void lua::print(const std::wstring str)
{
    std::wcout << str << std::endl;
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
BOOST_FUSION_ADAPT_STRUCT(lua::Block, statements)
BOOST_FUSION_ADAPT_STRUCT(lua::Variable, name)
BOOST_FUSION_ADAPT_STRUCT(lua::Assignment, variable, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::Expression, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::ExpressionList, expressions)
BOOST_FUSION_ADAPT_STRUCT(lua::BinaryExpression, expression1, binaryOperator, expression2)
BOOST_FUSION_ADAPT_STRUCT(lua::UnaryExpression, unaryOperator, expression)
BOOST_FUSION_ADAPT_STRUCT(lua::PrefixExpression, prefixExpression)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionName, name)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionBody, params, block)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionDefinition, functionBody, functionName)
BOOST_FUSION_ADAPT_STRUCT(lua::Arg, expressionList)
BOOST_FUSION_ADAPT_STRUCT(lua::FunctionCall, prefixExpression, arg)
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

#define PRINT_MSG(msg) qi::eps[bind(&lua::print, msg)] >> 

// lua 파서 구현
template <typename Iterator>
struct lua_grammar : qi::grammar<Iterator, lua::ChunkPtr(), skipper<Iterator>> {
    lua_grammar() : lua_grammar::base_type(ruleChunk) {

        symbols.add(L"and", L"and")(L"or", L"or")(L"not", L"not")(L"break", L"break")(L"goto", L"goto")(L"do", L"do")(L"end", L"end")(L"while", L"while")
                    (L"repeat", L"repeat")(L"return", L"return")(L"until", L"until")(L"if", L"if")(L"then", L"then")(L"elseif", L"elseif")(L"else", L"else")
                    (L"for", L"for")(L"in", L"in")(L"function", L"function")(L"local", L"local")(L"false", L"false")(L"true", L"true");

        ruleName                %= PRINT_MSG(L"ruleName") lexeme[!symbols >> (qi::alpha >> *(qi::alnum | '_'))];
        ruleNumeral             %= PRINT_MSG(L"ruleNumeral") qi::double_;
        ruleBoolean             = PRINT_MSG(L"ruleBoolean") lit("false")[_val = false] | lit("true")[_val = true];
        ruleLiteralString       %= PRINT_MSG(L"ruleLiteralString") lexeme['"' >> *(qi::char_ - '"') >> '"'];
        ruleBinaryOperator      %= PRINT_MSG(L"ruleBinaryOperator")  lit('+') | lit('-') | lit('*') | lit('/') | lit("//") | lit('^') | lit('%') | lit('&') | lit('~') | lit('|') | lit(">>") | lit("<<") | lit("..") |
                                    lit('<') | lit("<=") | lit('>') | lit(">=") | lit("==") | lit("~=") | lit("and") | lit("or");
        ruleUnaryOperator       %= PRINT_MSG(L"ruleUnaryOperator") lit('-') | lit("not") | lit('#') | lit('~');
        ruleNameList            = PRINT_MSG(L"ruleNameList") (ruleName % ',')[_val = construct<lua::NameList>(_1)];

        ruleVariable            = PRINT_MSG(L"ruleVariable") ruleName[_val = construct<lua::VariablePtr>(new_<lua::Variable>(_1))];
        ruleAssignment          = PRINT_MSG(L"ruleAssignment") (ruleVariable >> lit('=') >> ruleExpression)[_val = construct<lua::AssignmentPtr>(new_<lua::Assignment>(_1, _2))];
        ruleArg                 = PRINT_MSG(L"ruleArg") (lit('(') >> ruleExpressionList >> lit(')'))[_val = construct<lua::ArgPtr>(new_<lua::Arg>(_1))];

        ruleExpression          = PRINT_MSG(L"ruleExpression") ruleBoolean | ruleNumeral | ruleLiteralString | ruleBinaryExpression | ruleUnaryExpression[_val = construct<lua::ExpressionPtr>(new_<lua::Expression>(_1))];
        ruleExpressionList      = PRINT_MSG(L"ruleExpressionList") (ruleExpression % ',')[_val = construct<lua::ExpressionListPtr>(new_<lua::ExpressionList>(_1))];
        rulePrefixExpression    = PRINT_MSG(L"rulePrefixExpression") ruleVariable | ruleFunctionCall | (lit('(') >> ruleExpression >> lit(')'))[_val = construct<lua::PrefixExpressionPtr>(new_<lua::PrefixExpression>(_1))];
        ruleBinaryExpression    = PRINT_MSG(L"ruleBinaryExpression") (ruleExpression >> ruleBinaryOperator >> ruleExpression) [_val = construct<lua::BinaryExpressionPtr>(new_<lua::BinaryExpression>(_1, _2, _3))];
        ruleUnaryExpression     = PRINT_MSG(L"ruleUnaryExpression") (ruleUnaryOperator >> ruleExpression) [_val = construct<lua::UnaryExpressionPtr>(new_<lua::UnaryExpression>(_1, _2))];
        
        ruleFunctionDefinition = PRINT_MSG(L"ruleFunctionDefinition") (lit("function") >> ruleFunctionName >> ruleFunctionBody)[_val = construct<lua::FunctionDefinitionPtr>(new_<lua::FunctionDefinition>(_1, _2))];
        ruleFunctionName       = PRINT_MSG(L"ruleFunctionName") ruleName[_val = construct<lua::FunctionNamePtr>(new_<lua::FunctionName>(_1))];
        ruleFunctionBody       = PRINT_MSG(L"ruleFunctionBody") (lit('(') >> ruleNameList >> lit(')') >> ruleBlock >> lit("end"))[_val = construct<lua::FunctionBodyPtr>(new_<lua::FunctionBody>(_1, _2))];
        ruleFunctionCall       = PRINT_MSG(L"ruleFunctionCall") (rulePrefixExpression >> ruleArg)[_val = construct<lua::FunctionCallPtr>(new_<lua::FunctionCall>(_1, _2))];

        ruleStatement          = PRINT_MSG(L"ruleStatement") ruleStatIf | ruleFunctionCall[_val = construct<lua::StatementPtr>(new_<lua::Statement>(_1))];
        ruleStatReturn;
        ruleStatBreak;
        ruleStatGoto;
        ruleStatDo;
        ruleStatWhile;
        ruleStatRepeat;
        ruleStatIf             = PRINT_MSG(L"ruleStatIf") (lit("if") >> ruleExpression >> lit("then") >> ruleBlock >> lit("end"))[_val = construct<lua::StatIfPtr>(new_<lua::StatIf>(_1, _2, nullptr))];
        ruleStatFor;
        ruleStatFunction;


        ruleBlock = PRINT_MSG(L"ruleBlock") (*ruleStatement)[_val = construct<lua::BlockPtr>(new_<lua::Block>(_1))];
        ruleChunk = PRINT_MSG(L"ruleChunk") ruleBlock[_val = construct<lua::ChunkPtr>(new_<lua::Chunk>(_1))];
    }

    // 타입별 파서
    qi::symbols<wchar_t, std::wstring> symbols;

    qi::rule<Iterator, lua::Name(), skipper<Iterator>> ruleName;
    qi::rule<Iterator, lua::Numeral(), skipper<Iterator>> ruleNumeral;
    qi::rule<Iterator, lua::Boolean(), skipper<Iterator>> ruleBoolean;
    qi::rule<Iterator, lua::LiteralString(), skipper<Iterator>> ruleLiteralString;
    qi::rule<Iterator, lua::BinaryOperator(), skipper<Iterator>> ruleBinaryOperator;
    qi::rule<Iterator, lua::UnaryOperator(), skipper<Iterator>> ruleUnaryOperator;
    qi::rule<Iterator, lua::NameList(), skipper<Iterator>> ruleNameList;

    qi::rule<Iterator, lua::VariablePtr(), skipper<Iterator>> ruleVariable;
    qi::rule<Iterator, lua::AssignmentPtr(), skipper<Iterator>> ruleAssignment;
    qi::rule<Iterator, lua::ArgPtr(), skipper<Iterator>> ruleArg;

    qi::rule<Iterator, lua::ExpressionPtr(), skipper<Iterator>> ruleExpression;
    qi::rule<Iterator, lua::ExpressionListPtr(), skipper<Iterator>> ruleExpressionList;
    qi::rule<Iterator, lua::PrefixExpressionPtr(), skipper<Iterator>> rulePrefixExpression;
    qi::rule<Iterator, lua::BinaryExpressionPtr(), skipper<Iterator>> ruleBinaryExpression;
    qi::rule<Iterator, lua::UnaryExpressionPtr(), skipper<Iterator>> ruleUnaryExpression;
    
    qi::rule<Iterator, lua::FunctionDefinitionPtr(), skipper<Iterator>> ruleFunctionDefinition;
    qi::rule<Iterator, lua::FunctionNamePtr(), skipper<Iterator>> ruleFunctionName;
    qi::rule<Iterator, lua::FunctionBodyPtr(), skipper<Iterator>> ruleFunctionBody;
    qi::rule<Iterator, lua::FunctionCallPtr(), skipper<Iterator>> ruleFunctionCall;
    
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

    qi::rule<Iterator, lua::BlockPtr(), skipper<Iterator>> ruleBlock;
    qi::rule<Iterator, lua::ChunkPtr(), skipper<Iterator>> ruleChunk;
};





void print_node(const mini_xml_node& node, int indent = 0)
{
    std::visit( [indent](auto&& value)
        {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, boost::recursive_wrapper<mini_xml>>)
            {
                const mini_xml& xml = value.get();
                std::cout << std::string(indent, ' ') << "<" << xml.name << ">\n";
                for (const auto& child : xml.children)
                {
                    print_node(child, indent + 2);
                }
                std::cout << std::string(indent, ' ') << "</" << xml.name << ">\n";
            }
            else if constexpr (std::is_same_v<T, std::string>) 
            {
                std::cout << std::string(indent, ' ') << value << "\n";
            }
        }, node);
}

void ParserTest()
{
    std::wstring input = LR"(
        i = 0 -- 변수 초기화
        if i == 0 then
            print("It's Zero.")
        end
    )";

    input = L"employee{ 11, \"surname\", \"forename\", 22 }";

    input = LR"(
<start>
  <try>080100000113fc208dff</try>
  <avl_array>
    <try>080100000113fc208dff</try>
    <success></success>
    <attributes>[[8, [[1185345998335, 42]]]]</attributes><locals>(1)</locals>
  </avl_array>
  <success></success>
  <attributes>[[8, [[1185345998335, 42]]]]</attributes>
</start> )";

    input = L"if true then func() end";

    using It = std::wstring::iterator;
    It iter = input.begin(), end = input.end();

    skipper<It> skip;


    //mini_xml prog;
    //mini_xml_grammar<It> grammar;

    lua::ChunkPtr prog;
    //lua::ExpressionPtr prog;
    lua_grammar<It> grammar;
    
    

    bool r = phrase_parse(iter, end, grammar, skip, prog);

    if (r && iter == end) {
        std::cout << "AST 파싱 성공!" << std::endl;
        //std::cout << "문장 수: " << prog.statements.size() << std::endl;

        /*for (const mini_xml_node& child : prog.children)
            print_node(child);*/
    }
    else {
        std::cout << "파싱 실패..." << std::endl;
    }
}