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

#include "ast.h"

#include "parser.h"



BOOST_FUSION_ADAPT_STRUCT(ast::identifier, name)
BOOST_FUSION_ADAPT_STRUCT(ast::number, value)
BOOST_FUSION_ADAPT_STRUCT(ast::string, value)
BOOST_FUSION_ADAPT_STRUCT(ast::var_assign, lhs, rhs)
BOOST_FUSION_ADAPT_STRUCT(ast::print_stmt, value)
BOOST_FUSION_ADAPT_STRUCT(ast::if_stmt, cond_var, cond_value, body)
BOOST_FUSION_ADAPT_STRUCT(ast::program, statements)

BOOST_FUSION_ADAPT_STRUCT(
    ast::employee,
    (int, age)
    (std::string, surname)
    (std::string, forename)
    (double, salary)
)

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

// AST 파서 구현
template <typename Iterator>
struct script_grammar : qi::grammar<Iterator, ast::program(), skipper<Iterator>> {
    script_grammar() : script_grammar::base_type(start) {
        using namespace qi;
        using boost::phoenix::construct;
        using boost::phoenix::new_;
        using boost::phoenix::push_back;

        identifier %= lexeme[qi::alpha >> *qi::alnum];
        number_ %= int_;
        string_ %= lexeme['"' >> *(char_ - '"') >> '"'];

        var_assign = identifier >> '=' >> number_;
        print_stmt = lit("print") >> '(' >> string_ >> ')';

        if_stmt =
            lit("if") >> identifier >> "==" >> number_ >> "then"
            >> +statement
            >> lit("end")[_val = new_<ast::if_stmt>(_1, _2, _3)];

        statement = var_assign | print_stmt | if_stmt;
        start = +statement[_val = construct<ast::program>(_1)];

        // 디버그용
        BOOST_SPIRIT_DEBUG_NODES((start)(statement)(if_stmt)(print_stmt)(var_assign)(identifier)(number_)(string_))
    }

    // 타입별 파서
    qi::rule<Iterator, ast::identifier(), skipper<Iterator>> identifier;
    qi::rule<Iterator, ast::number(), skipper<Iterator>> number_;
    qi::rule<Iterator, ast::string(), skipper<Iterator>> string_;
    qi::rule<Iterator, ast::var_assign(), skipper<Iterator>> var_assign;
    qi::rule<Iterator, ast::print_stmt(), skipper<Iterator>> print_stmt;
    qi::rule<Iterator, std::shared_ptr<ast::if_stmt>(), skipper<Iterator>> if_stmt;
    qi::rule<Iterator, ast::statement(), skipper<Iterator>> statement;
    qi::rule<Iterator, ast::program(), skipper<Iterator>> start;
};

// AST 파서
template <typename Iterator>
struct dslGrammar : qi::grammar<Iterator, int(), skipper<Iterator>> 
{
    //dslGrammar() : dslGrammar::base_type(start)
    dslGrammar() : qi::grammar<Iterator, int(), skipper<Iterator>>(start)
    {
        start = lexeme[qi::alpha >> *qi::alnum];
    }

    qi::rule<Iterator, int(), skipper<Iterator>> start;
};

template <typename Iterator>
struct employee_parser : qi::grammar<Iterator, ast::employee(), skipper<Iterator>>
{
    employee_parser() : employee_parser::base_type(start)
    {
        using qi::int_;
        using qi::lit;
        using qi::double_;
        using qi::lexeme;
        using ascii::char_;

        quoted_string %= lexeme['"' >> +(char_ - '"') >> '"'];

        start %=
            lit("employee")
            >> '{'
            >> int_ >> ','
            >> quoted_string >> ','
            >> quoted_string >> ','
            >> double_
            >> '}'
            ;
    }

    qi::rule<Iterator, std::string(), skipper<Iterator>> quoted_string;
    qi::rule<Iterator, ast::employee(), skipper<Iterator>> start;
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
    std::string input = R"(
        i = 0 -- 변수 초기화
        if i == 0 then
            print("It's Zero.")
        end
    )";

    input = "employee{ 11, \"surname\", \"forename\", 22 }";

    input = R"(
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

    using It = std::string::iterator;
    It iter = input.begin(), end = input.end();

    //ast::program prog;
    //script_grammar<It> grammar;
    //skipper<It> skip;

    skipper<It> skip;


    //int prog;
    //dslGrammar<It> grammar;

    //ast::employee prog;
    //employee_parser<It> grammar;

    mini_xml prog;
    mini_xml_grammar<It> grammar;
    

    bool r = phrase_parse(iter, end, grammar, skip, prog);

    if (r && iter == end) {
        std::cout << "✅ AST 파싱 성공!" << std::endl;
        //std::cout << "문장 수: " << prog.statements.size() << std::endl;

        for (const mini_xml_node& child : prog.children)
            print_node(child);
    }
    else {
        std::cout << "❌ 파싱 실패..." << std::endl;
    }
}