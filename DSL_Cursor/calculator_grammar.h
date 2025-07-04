/**
 * @file calculator_grammar.h
 * @brief Boost.Spirit를 사용한 수학 표현식 파서 문법 정의
 * @details 재귀 하강 파서를 사용하여 수학 표현식을 AST로 파싱
 * 
 * 문법 규칙 (우선순위 순서):
 * 1. factor: 숫자, 변수, 또는 괄호 표현식 (가장 높은 우선순위)
 * 2. term: 곱셈/나눗셈 (중간 우선순위)
 * 3. expression: 덧셈/뺄셈 (중간 우선순위)
 * 4. comparison: 비교 연산 (중간 우선순위)
 * 5. assignment: 변수 할당 (가장 낮은 우선순위)
 * 
 * 지원 연산자:
 * - 산술 연산: +, -, *, /
 * - 비교 연산: ==, <, >, <=, >=
 * - 할당 연산: =
 * - 괄호: ( )
 * - 변수: 식별자 (알파벳, 숫자, 언더스코어)
 */

#ifndef CALCULATOR_GRAMMAR_H
#define CALCULATOR_GRAMMAR_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_real.hpp>
#include <boost/phoenix.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/stl.hpp>
#include <memory>

// Forward declarations
struct Expression;
using ExpressionPtr = std::shared_ptr<Expression>;
struct Number;
struct BinaryOp;
struct ComparisonOp;
struct Variable;
struct VariableAssignment;
struct UnaryOp;

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

/**
 * @brief Boost.Spirit 파서 문법 템플릿 클래스
 * @tparam Iterator 입력 문자열의 반복자 타입
 * @details 재귀 하강 파서를 사용하여 수학 표현식을 파싱하고 AST를 생성
 * 
 * 파싱 과정:
 * 1. 입력 문자열을 토큰으로 분리
 * 2. 우선순위에 따라 표현식을 재귀적으로 파싱
 * 3. 각 연산자에 맞는 AST 노드를 생성
 * 4. 최종적으로 완전한 AST를 반환
 * 
 * 새로운 기능:
 * - 변수 할당: identifier = expression
 * - 변수 참조: identifier
 * - 식별자 파싱: 알파벳, 숫자, 언더스코어로 구성
 */
template<typename Iterator>
struct CalculatorGrammar : qi::grammar<Iterator, ExpressionPtr(), qi::space_type> {
    
    /**
     * @brief 생성자
     * @details 파서 문법 규칙들을 초기화하고 우선순위를 설정
     * 
     * 파서 규칙 초기화 순서:
     * 1. 기본 요소 (숫자, 변수, 괄호)
     * 2. 곱셈/나눗셈 (term)
     * 3. 덧셈/뺄셈 (expression)
     * 4. 비교 연산 (comparison)
     * 5. 변수 할당 (assignment)
     */
    CalculatorGrammar() : CalculatorGrammar::base_type(assignment) {
        using namespace qi;
        
        // ============================================================================
        // 식별자 파싱 규칙
        // ============================================================================
        
        // 키워드 정의
        symbolKeyword.add("and", "and")("or", "or")("break", "break")("goto", "goto")("do", "do")("end", "end")("while", "while")
                    ("repeat", "repeat")("return", "return")("until", "until")("if", "if")("then", "then")("elseif", "elseif")("else", "else")
                    ("for", "for")("in", "in")("function", "function")("local", "local")("false", "false")("true", "true");

        /**
         * @brief 식별자 파싱 규칙
         * @details 변수 이름을 파싱
         * 
         * 식별자 규칙:
         * - 첫 글자는 알파벳 또는 언더스코어
         * - 나머지 글자는 알파벳, 숫자, 언더스코어
         * - 예: value, val2, _temp, x1, not_value, not_x
         * - 단, 정확히 "not"만은 키워드로 처리
         */
        identifier = lexeme[qi::raw[(qi::alpha | qi::char_('_')) >> *(qi::alnum | qi::char_('_'))] - symbolKeyword];
        
        // ============================================================================
        // 숫자 파싱 규칙
        // ============================================================================
        
        /**
         * @brief 정수 파싱 규칙
         * @details 소수점이 없는 숫자를 정수로 파싱
         * 파싱 성공 시 Number 노드를 생성하여 AST에 추가
         */
        integer = int_[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<Number>(_1))];
        
        /**
         * @brief 실수 파싱 규칙
         * @details 소수점이 있는 숫자를 실수로 파싱
         * double_ 파서를 사용하여 다양한 실수 형태 지원 (예: 3.14, 2.0, .5)
         */
        float_number = double_[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<Number>(_1))];
        
        /**
         * @brief 숫자 파싱 규칙 (실수 우선)
         * @details 실수를 먼저 시도하고, 실패하면 정수로 파싱
         * 이는 "3.14"와 같은 실수가 "3"과 "14"로 분리되는 것을 방지
         */
        number = float_number | integer;
        
        // ============================================================================
        // 기본 표현식 파싱 규칙
        // ============================================================================
        
        /**
         * @brief 기본 요소(primary) 파싱 규칙
         * @details 숫자, 변수, 또는 괄호로 둘러싸인 표현식을 파싱
         * 
         * 파싱 순서:
         * 1. 숫자 (정수 또는 실수)
         * 2. 변수 (식별자)
         * 3. 괄호 표현식
         * 
         * 괄호는 우선순위를 높이는 역할을 함
         * 재귀적으로 assignment를 호출하여 괄호 안의 표현식을 파싱
         */
        primary = number[_val = _1] | 
                  identifier[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<Variable>(_1))] |
                  ('(' >> assignment >> ')')[_val = _1];
        
        /**
         * @brief 인수(factor) 파싱 규칙 - 단항 연산자 처리
         * @details 단항 연산자(not, -)를 처리
         * 
         * 파싱 과정:
         * 1. 단항 연산자(not, -)를 파싱
         * 2. 피연산자(primary)를 파싱
         * 3. UnaryOp 노드를 생성하여 AST에 추가
         * 
         * 우선순위: 단항 연산자는 곱셈/나눗셈보다 높음
         * 예시: "-5" → UnaryOp("-", Number(5))
         *       "not x" → UnaryOp("not", Variable("x"))
         */
        factor = ("not" >> primary)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<UnaryOp>("not", _1))] |
                 ('-' >> primary)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<UnaryOp>("-", _1))] |
                 primary[_val = _1];
        
        /**
         * @brief 항(term) 파싱 규칙 - 곱셈과 나눗셈
         * @details factor를 시작으로 하고, 0개 이상의 곱셈/나눗셈 연산을 처리
         * 
         * 파싱 과정:
         * 1. 첫 번째 factor를 파싱
         * 2. '*' 또는 '/' 연산자와 다음 factor를 반복적으로 파싱
         * 3. 각 연산에 대해 BinaryOp 노드를 생성하여 AST에 추가
         * 
         * 예시: "2*3*4" → ((2*3)*4)
         */
        term = factor[_val = _1] >> *(
            ('*' >> factor)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<BinaryOp>(_val, '*', _1))] |
            ('/' >> factor)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<BinaryOp>(_val, '/', _1))]
        );
        
        /**
         * @brief 표현식(expression) 파싱 규칙 - 덧셈과 뺄셈
         * @details term을 시작으로 하고, 0개 이상의 덧셈/뺄셈 연산을 처리
         * 
         * 파싱 과정:
         * 1. 첫 번째 term을 파싱
         * 2. '+' 또는 '-' 연산자와 다음 term을 반복적으로 파싱
         * 3. 각 연산에 대해 BinaryOp 노드를 생성하여 AST에 추가
         * 
         * 예시: "1+2-3" → ((1+2)-3)
         */
        expression = term[_val = _1] >> *(
            ('+' >> term)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<BinaryOp>(_val, '+', _1))] |
            ('-' >> term)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<BinaryOp>(_val, '-', _1))]
        );
        
        /**
         * @brief 비교 연산(comparison) 파싱 규칙
         * @details expression을 시작으로 하고, 0개 이상의 비교 연산을 처리
         * 
         * 지원하는 비교 연산자:
         * - == : 같음
         * - <= : 작거나 같음
         * - >= : 크거나 같음
         * - <  : 작음
         * - >  : 큼
         * 
         * 파싱 과정:
         * 1. 첫 번째 expression을 파싱
         * 2. 비교 연산자와 다음 expression을 반복적으로 파싱
         * 3. 각 비교 연산에 대해 ComparisonOp 노드를 생성하여 AST에 추가
         * 
         * 예시: "5 > 3 > 1" → ((5 > 3) > 1)
         */
        comparison = expression[_val = _1] >> *(
            ("==" >> expression)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<ComparisonOp>(_val, "==", _1))] |
            ("<=" >> expression)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<ComparisonOp>(_val, "<=", _1))] |
            (">=" >> expression)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<ComparisonOp>(_val, ">=", _1))] |
            ('<' >> expression)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<ComparisonOp>(_val, "<", _1))] |
            ('>' >> expression)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<ComparisonOp>(_val, ">", _1))]
        );
        
        /**
         * @brief 변수 할당(assignment) 파싱 규칙
         * @details 변수에 값을 할당하는 표현식을 파싱
         * 가장 낮은 우선순위를 가지므로 다른 모든 연산이 완료된 후 처리
         * 
         * 파싱 과정:
         * 1. 식별자(변수 이름)를 파싱
         * 2. '=' 연산자를 파싱
         * 3. 할당할 표현식을 파싱 (재귀적으로 comparison 호출)
         * 4. VariableAssignment 노드를 생성하여 AST에 추가
         * 
         * 예시: "value = 10" → VariableAssignment("value", Number(10))
         *       "val2 = value" → VariableAssignment("val2", Variable("value"))
         *       "val3 = (value + 5)" → VariableAssignment("val3", BinaryOp(Variable("value"), '+', Number(5)))
         */
        assignment = (identifier >> '=' >> comparison)[_val = phoenix::construct<ExpressionPtr>(phoenix::new_<VariableAssignment>(_1, _2))] |
                    comparison[_val = _1];

        // ============================================================================
        // 디버그 정보 출력 (개발 시에만 사용)
        // ============================================================================
        
        /**
         * @brief 파서 디버그 노드 설정
         * @details 각 파서 규칙의 디버그 정보를 활성화
         * 파싱 과정에서 각 규칙이 어떻게 매칭되는지 상세히 출력
         */
        // BOOST_SPIRIT_DEBUG_NODES((identifier)(integer)(float_number)(number)(primary)(factor)(term)(expression)(comparison)(assignment));
    }
    
    // ============================================================================
    // 파서 규칙 선언
    // ============================================================================
    qi::symbols<char, std::string> symbolKeyword;   // 키워드 정의

    qi::rule<Iterator, std::string(), qi::space_type> identifier;     ///< 식별자 파싱 규칙
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> integer;      ///< 정수 파싱 규칙
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> float_number; ///< 실수 파싱 규칙
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> number;       ///< 숫자 파싱 규칙 (실수 또는 정수)
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> primary;      ///< 기본 요소 파싱 규칙 (숫자, 변수, 괄호)
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> factor;       ///< 인수 파싱 규칙 (단항 연산자 포함)
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> term;         ///< 항 파싱 규칙 (곱셈/나눗셈)
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> expression;   ///< 표현식 파싱 규칙 (덧셈/뺄셈)
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> comparison;   ///< 비교 연산 파싱 규칙
    qi::rule<Iterator, ExpressionPtr(), qi::space_type> assignment;   ///< 변수 할당 파싱 규칙
};

#endif // CALCULATOR_GRAMMAR_H 