/**
 * @file calculator.cpp
 * @brief DSL Parser의 핵심 구현 파일
 * @details Boost.Spirit 파서를 사용한 수학 표현식 파싱 및 AST 처리 기능 구현
 * 
 * 주요 구성 요소:
 * 1. AST (Abstract Syntax Tree) 노드 정의
 *    - Expression: 기본 추상 클래스
 *    - Number: 숫자 리터럴 (정수/실수 구분)
 *    - BinaryOp: 이진 연산 (+, -, *, /)
 *    - ComparisonOp: 비교 연산 (==, <, >, <=, >=)
 * 
 * 2. ExpressionContainer 클래스 구현
 *    - 다중 표현식 저장 및 관리
 *    - 일괄 처리 기능
 * 
 * 3. Calculator 클래스 구현
 *    - Boost.Spirit 파서를 사용한 표현식 파싱
 *    - AST 생성 및 평가
 *    - 파일 및 문자열에서 표현식 읽기
 * 
 * 4. 테스트 함수들
 *    - 다양한 시나리오에서의 기능 검증
 */

#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_real.hpp>
#include <boost/phoenix.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include "calculator.h"
#include "calculator_grammar.h"

// Boost.Spirit 네임스페이스 별칭
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

// ============================================================================
// AST (Abstract Syntax Tree) 노드 정의
// ============================================================================

/**
 * @brief 표현식의 기본 추상 클래스
 * @details 모든 AST 노드는 이 클래스를 상속받아야 함
 * 
 * 가상 함수:
 * - print(): AST 구조를 트리 형태로 출력
 * - evaluate(): 표현식을 계산하여 결과 반환
 */
struct Expression {
    virtual ~Expression() = default;
    virtual void print(int indent = 0) const = 0;  ///< AST 구조를 출력
    virtual double evaluate() const = 0;           ///< 표현식을 계산하여 결과 반환
};

/**
 * @brief 숫자 리터럴을 나타내는 AST 노드
 * @details 정수와 실수를 구분해서 저장하고 처리
 * 
 * 특징:
 * - 정수와 실수를 타입으로 구분
 * - 실수 값이 정수인 경우 자동으로 정수로 변환
 * - 타입에 따른 적절한 값 반환
 */
struct Number : Expression {
    /**
     * @brief 숫자 타입 열거형
     * @details 정수와 실수를 구분하여 저장
     */
    enum class Type {
        INTEGER,  ///< 정수 타입
        FLOAT     ///< 실수 타입
    };
    
    Type type;         ///< 숫자 타입 (정수 또는 실수)
    int intValue;      ///< 정수 값 (정수 타입일 때 사용)
    double floatValue; ///< 실수 값 (실수 타입일 때 사용)
    
    /**
     * @brief 정수 생성자
     * @param v 정수 값
     * @details 정수 타입으로 초기화하고 floatValue는 0.0으로 설정
     */
    Number(int v) : type(Type::INTEGER), intValue(v), floatValue(0.0) {}
    
    /**
     * @brief 실수 생성자
     * @param v 실수 값
     * @details 실수 타입으로 초기화하되, 정수인 경우 자동으로 정수 타입으로 변환
     *          예: 3.0 → 정수 타입, 3.14 → 실수 타입
     */
    Number(double v) : type(Type::FLOAT), intValue(0), floatValue(v) {
        // 실수 값이 정수인지 확인하여 타입 최적화
        if (v == static_cast<int>(v)) {
            type = Type::INTEGER;
            intValue = static_cast<int>(v);
        }
    }
    
    /**
     * @brief AST 구조를 트리 형태로 출력
     * @param indent 들여쓰기 레벨
     * @details 타입에 따라 "Number(INT): 값" 또는 "Number(FLOAT): 값" 형태로 출력
     */
    void print(int indent = 0) const override {
        std::string spaces(indent * 2, ' ');
        if (type == Type::INTEGER) {
            std::cout << spaces << "Number(INT): " << intValue << std::endl;
        } else {
            std::cout << spaces << "Number(FLOAT): " << floatValue << std::endl;
        }
    }
    
    /**
     * @brief 숫자 값을 반환
     * @return 계산된 숫자 값 (double 타입)
     * @details 타입에 따라 적절한 값을 반환
     *          정수 타입: intValue를 double로 변환
     *          실수 타입: floatValue 그대로 반환
     */
    double evaluate() const override {
        if (type == Type::INTEGER) {
            return static_cast<double>(intValue);
        } else {
            return floatValue;
        }
    }
    
    // ============================================================================
    // 유틸리티 함수들
    // ============================================================================
    
    /**
     * @brief 정수인지 확인
     * @return true if integer, false otherwise
     */
    bool isInteger() const {
        return type == Type::INTEGER;
    }
    
    /**
     * @brief 실수인지 확인
     * @return true if float, false otherwise
     */
    bool isFloat() const {
        return type == Type::FLOAT;
    }
    
    /**
     * @brief 정수 값 반환
     * @return 정수 값 (타입과 관계없이 intValue 반환)
     */
    int getIntValue() const {
        return intValue;
    }
    
    /**
     * @brief 실수 값 반환
     * @return 실수 값 (타입과 관계없이 floatValue 반환)
     */
    double getFloatValue() const {
        return floatValue;
    }
};

/**
 * @brief 이진 연산을 나타내는 AST 노드
 * @details 두 개의 하위 표현식과 연산자로 구성
 * 
 * 지원 연산자:
 * - + : 덧셈
 * - - : 뺄셈
 * - * : 곱셈
 * - / : 나눗셈 (0으로 나누기 방지)
 */
struct BinaryOp : Expression {
    std::shared_ptr<Expression> left;   ///< 왼쪽 피연산자
    std::shared_ptr<Expression> right;  ///< 오른쪽 피연산자
    char op;                            ///< 연산자 (+, -, *, /)
    
    /**
     * @brief 생성자
     * @param l 왼쪽 피연산자
     * @param o 연산자
     * @param r 오른쪽 피연산자
     */
    BinaryOp(std::shared_ptr<Expression> l, char o, std::shared_ptr<Expression> r)
        : left(l), right(r), op(o) {}
    
    /**
     * @brief AST 구조를 트리 형태로 출력
     * @param indent 들여쓰기 레벨
     * @details 연산자를 출력하고 왼쪽/오른쪽 피연산자를 재귀적으로 출력
     */
    void print(int indent = 0) const override {
        std::string spaces(indent * 2, ' ');
        std::cout << spaces << "BinaryOp: " << op << std::endl;
        if (left) left->print(indent + 1);   ///< 왼쪽 피연산자 출력
        if (right) right->print(indent + 1); ///< 오른쪽 피연산자 출력
    }
    
    /**
     * @brief 이진 연산을 수행하여 결과 반환
     * @return 계산된 결과값
     * @details 연산자에 따라 적절한 연산을 수행
     *          나눗셈의 경우 0으로 나누기를 방지
     */
    double evaluate() const override {
        double l = left ? left->evaluate() : 0.0;   ///< 왼쪽 피연산자 계산
        double r = right ? right->evaluate() : 0.0; ///< 오른쪽 피연산자 계산
        
        // 연산자에 따라 적절한 연산 수행
        switch (op) {
            case '+': return l + r;  ///< 덧셈
            case '-': return l - r;  ///< 뺄셈
            case '*': return l * r;  ///< 곱셈
            case '/': return r != 0.0 ? l / r : 0.0;  ///< 나눗셈 (0으로 나누기 방지)
            default: return 0.0;     ///< 알 수 없는 연산자
        }
    }
};

/**
 * @brief 비교 연산을 나타내는 AST 노드
 * @details 두 개의 하위 표현식과 비교 연산자로 구성
 * 
 * 지원 연산자:
 * - == : 같음 (1.0 = true, 0.0 = false)
 * - <  : 작음
 * - >  : 큼
 * - <= : 작거나 같음
 * - >= : 크거나 같음
 */
struct ComparisonOp : Expression {
    std::shared_ptr<Expression> left;   ///< 왼쪽 피연산자
    std::shared_ptr<Expression> right;  ///< 오른쪽 피연산자
    std::string op;                     ///< 비교 연산자 (==, <, >, <=, >=)
    
    /**
     * @brief 생성자
     * @param l 왼쪽 피연산자
     * @param o 비교 연산자
     * @param r 오른쪽 피연산자
     */
    ComparisonOp(std::shared_ptr<Expression> l, const std::string& o, std::shared_ptr<Expression> r)
        : left(l), right(r), op(o) {}
    
    /**
     * @brief AST 구조를 트리 형태로 출력
     * @param indent 들여쓰기 레벨
     * @details 비교 연산자를 출력하고 왼쪽/오른쪽 피연산자를 재귀적으로 출력
     */
    void print(int indent = 0) const override {
        std::string spaces(indent * 2, ' ');
        std::cout << spaces << "ComparisonOp: " << op << std::endl;
        if (left) left->print(indent + 1);   ///< 왼쪽 피연산자 출력
        if (right) right->print(indent + 1); ///< 오른쪽 피연산자 출력
    }
    
    /**
     * @brief 비교 연산을 수행하여 결과 반환
     * @return 비교 결과 (1.0 = true, 0.0 = false)
     * @details 비교 연산자에 따라 적절한 비교를 수행
     *          결과는 부울값을 double로 표현 (1.0 = true, 0.0 = false)
     */
    double evaluate() const override {
        double l = left ? left->evaluate() : 0.0;   ///< 왼쪽 피연산자 계산
        double r = right ? right->evaluate() : 0.0; ///< 오른쪽 피연산자 계산
        
        // 비교 연산자에 따라 적절한 비교 수행
        if (op == "==") return (l == r) ? 1.0 : 0.0;  ///< 같음
        if (op == "<") return (l < r) ? 1.0 : 0.0;    ///< 작음
        if (op == ">") return (l > r) ? 1.0 : 0.0;    ///< 큼
        if (op == "<=") return (l <= r) ? 1.0 : 0.0;  ///< 작거나 같음
        if (op == ">=") return (l >= r) ? 1.0 : 0.0;  ///< 크거나 같음
        return 0.0;  ///< 알 수 없는 비교 연산자
    }
};

/**
 * @brief 변수를 나타내는 AST 노드
 * @details 변수 이름을 저장하고, 평가 시 변수 테이블에서 값을 조회
 */
struct Variable : Expression {
    std::string name;  ///< 변수 이름
    
    /**
     * @brief 생성자
     * @param varName 변수 이름
     */
    Variable(const std::string& varName) : name(varName) {}
    
    /**
     * @brief AST 구조를 트리 형태로 출력
     * @param indent 들여쓰기 레벨
     * @details 변수 이름을 출력
     */
    void print(int indent = 0) const override {
        std::string spaces(indent * 2, ' ');
        std::cout << spaces << "Variable: " << name << std::endl;
    }
    
    /**
     * @brief 변수 값을 반환
     * @return 변수 테이블에서 조회한 값
     * @details 전역 변수 테이블에서 변수 값을 조회
     *          변수가 정의되지 않은 경우 0.0 반환
     */
    double evaluate() const {
        return getVariable(name);
    }
    
    /**
     * @brief 변수 이름 반환
     * @return 변수 이름
     */
    const std::string& getName() const { return name; }
};

/**
 * @brief 변수 할당을 나타내는 AST 노드
 * @details 변수에 값을 할당하고, 할당된 값을 반환
 */
struct VariableAssignment : Expression {
    std::string variableName;           ///< 할당할 변수 이름
    std::shared_ptr<Expression> value;  ///< 할당할 값 (표현식)
    
    /**
     * @brief 생성자
     * @param varName 변수 이름
     * @param val 할당할 값 (표현식)
     */
    VariableAssignment(const std::string& varName, std::shared_ptr<Expression> val)
        : variableName(varName), value(val) {}
    
    /**
     * @brief AST 구조를 트리 형태로 출력
     * @param indent 들여쓰기 레벨
     * @details 변수 할당을 출력하고 할당할 값을 재귀적으로 출력
     */
    void print(int indent = 0) const override {
        std::string spaces(indent * 2, ' ');
        std::cout << spaces << "VariableAssignment: " << variableName << " = " << std::endl;
        if (value) value->print(indent + 1);  ///< 할당할 값 출력
    }
    
    /**
     * @brief 변수 할당을 수행하고 할당된 값을 반환
     * @return 할당된 값
     * @details 할당할 값을 계산하고 전역 변수 테이블에 저장
     */
    double evaluate() const {
        double result = value ? value->evaluate() : 0.0;
        setVariable(variableName, result);
        return result;
    }
    
    /**
     * @brief 변수 이름 반환
     * @return 변수 이름
     */
    const std::string& getVariableName() const { return variableName; }
};

/**
 * @brief 표현식 포인터 타입 별칭
 * @details std::shared_ptr를 사용하여 메모리 관리를 자동화
 */
using ExpressionPtr = std::shared_ptr<Expression>;

/**
 * @brief 단항 연산을 나타내는 AST 노드
 * @details 하나의 피연산자와 단항 연산자로 구성
 * 
 * 지원 연산자:
 * - not : 논리 부정 (0이 아닌 값은 0, 0은 1)
 * - - : 부호 반전 (양수는 음수, 음수는 양수)
 */
struct UnaryOp : Expression {
    std::shared_ptr<Expression> operand;  ///< 피연산자
    std::string op;                       ///< 단항 연산자 (not, -)
    
    /**
     * @brief 생성자
     * @param o 단항 연산자
     * @param expr 피연산자
     */
    UnaryOp(const std::string& o, std::shared_ptr<Expression> expr)
        : operand(expr), op(o) {}
    
    /**
     * @brief AST 구조를 트리 형태로 출력
     * @param indent 들여쓰기 레벨
     * @details 단항 연산자를 출력하고 피연산자를 재귀적으로 출력
     */
    void print(int indent = 0) const override {
        std::string spaces(indent * 2, ' ');
        std::cout << spaces << "UnaryOp: " << op << std::endl;
        if (operand) operand->print(indent + 1);  ///< 피연산자 출력
    }
    
    /**
     * @brief 단항 연산을 수행하여 결과 반환
     * @return 계산된 결과값
     * @details 연산자에 따라 적절한 단항 연산을 수행
     */
    double evaluate() const override {
        double val = operand ? operand->evaluate() : 0.0;
        
        if (op == "not") {
            return (val == 0.0) ? 1.0 : 0.0;  ///< 논리 부정
        } else if (op == "-") {
            return -val;  ///< 부호 반전
        }
        return val;  ///< 알 수 없는 연산자
    }
};

// ============================================================================
// 전역 변수 테이블
// ============================================================================

/**
 * @brief 전역 변수 테이블
 * @details 변수 이름과 값을 저장하는 맵
 */
static std::map<std::string, double> variableTable;

/**
 * @brief 변수 값을 설정
 * @param name 변수 이름
 * @param value 설정할 값
 */
void setVariable(const std::string& name, double value) {
    variableTable[name] = value;
}

/**
 * @brief 변수 값을 조회
 * @param name 변수 이름
 * @return 변수 값 (정의되지 않은 경우 0.0)
 */
double getVariable(const std::string& name) {
    auto it = variableTable.find(name);
    return (it != variableTable.end()) ? it->second : 0.0;
}

/**
 * @brief 변수가 정의되어 있는지 확인
 * @param name 변수 이름
 * @return true if defined, false otherwise
 */
bool isVariableDefined(const std::string& name) {
    return variableTable.find(name) != variableTable.end();
}

/**
 * @brief 모든 변수를 출력
 * @details 디버깅 목적으로 사용
 */
void printAllVariables() {
    std::cout << "=== Variable Table ===" << std::endl;
    if (variableTable.empty()) {
        std::cout << "No variables defined." << std::endl;
    } else {
        for (const auto& pair : variableTable) {
            std::cout << pair.first << " = " << pair.second << std::endl;
        }
    }
}

/**
 * @brief 변수 테이블을 초기화
 * @details 모든 변수를 제거
 */
void clearVariables() {
    variableTable.clear();
}

// ============================================================================
// ExpressionContainer 구현
// ============================================================================

void ExpressionContainer::addExpression(const std::string& text, ExpressionPtr ast, double result, bool isValid, const std::string& error) {
    expressions.emplace_back(text, ast, result, isValid, error);
}

const ParsedExpression& ExpressionContainer::getExpression(size_t index) const {
    if (index >= expressions.size()) {
        throw std::out_of_range("Expression index out of range");
    }
    return expressions[index];
}

void ExpressionContainer::printAllResults() const {
    std::cout << "\n=== All Parsed Expressions Results ===" << std::endl;
    for (size_t i = 0; i < expressions.size(); ++i) {
        const auto& expr = expressions[i];
        std::cout << "[" << i << "] ";
        if (expr.isValid) {
            std::cout << expr.originalText << " = " << expr.result << std::endl;
        } else {
            std::cout << expr.originalText << " -> ERROR: " << expr.errorMessage << std::endl;
        }
    }
}

void ExpressionContainer::printASTs() const {
    std::cout << "\n=== All AST Structures ===" << std::endl;
    for (size_t i = 0; i < expressions.size(); ++i) {
        const auto& expr = expressions[i];
        std::cout << "\n[" << i << "] AST for: " << expr.originalText << std::endl;
        if (expr.isValid && expr.ast) {
            expr.ast->print();
            std::cout << "Result: " << expr.result << std::endl;
        } else {
            std::cout << "ERROR: " << expr.errorMessage << std::endl;
        }
    }
}

// ============================================================================
// Calculator 클래스 구현
// ============================================================================

// GrammarImpl 구조체 정의 - CalculatorGrammar의 구체적인 인스턴스화를 래핑
struct Calculator::GrammarImpl {
    CalculatorGrammar<std::string::const_iterator> grammar;
    
    GrammarImpl() : grammar() {}
};

Calculator::Calculator() : grammar(std::make_unique<GrammarImpl>()) {
}

Calculator::~Calculator() = default;

ExpressionContainer Calculator::parseMultipleExpressions(const std::vector<std::string>& inputs) {
    ExpressionContainer container;
    
    for (const auto& input : inputs) {
        try {
            auto iter = input.cbegin();
            auto end = input.cend();
            
            ExpressionPtr result;
            bool success = qi::phrase_parse(iter, end, grammar->grammar, qi::space, result);
            
            if (success && iter == end) {
                double calculatedResult = result->evaluate();
                container.addExpression(input, result, calculatedResult, true, "");
            } else {
                container.addExpression(input, nullptr, 0.0, false, "Parsing failed");
            }
        } catch (const std::exception& e) {
            container.addExpression(input, nullptr, 0.0, false, e.what());
        }
    }
    
    return container;
}

ExpressionContainer Calculator::parseFromFile(const std::string& filename) {
    ExpressionContainer container;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cout << "Error: Cannot open file " << filename << std::endl;
        return container;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // 빈 줄과 주석 줄 무시
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // 앞뒤 공백 제거
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty()) continue;
        
        try {
            auto iter = line.cbegin();
            auto end = line.cend();
            
            ExpressionPtr result;
            bool success = qi::phrase_parse(iter, end, grammar->grammar, qi::space, result);
            
            if (success && iter == end) {
                double calculatedResult = result->evaluate();
                container.addExpression(line, result, calculatedResult, true, "");
            } else {
                std::string errorMsg = "Parsing failed at line " + std::to_string(lineNumber);
                container.addExpression(line, nullptr, 0.0, false, errorMsg);
            }
        } catch (const std::exception& e) {
            std::string errorMsg = "Error at line " + std::to_string(lineNumber) + ": " + e.what();
            container.addExpression(line, nullptr, 0.0, false, errorMsg);
        }
    }
    
    return container;
}

ExpressionContainer Calculator::parseFromString(const std::string& multiLineInput) {
    ExpressionContainer container;
    std::istringstream stream(multiLineInput);
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(stream, line)) {
        lineNumber++;
        
        // 빈 줄과 주석 줄 무시
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // 앞뒤 공백 제거
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty()) continue;
        
        try {
            auto iter = line.cbegin();
            auto end = line.cend();
            
            ExpressionPtr result;
            bool success = qi::phrase_parse(iter, end, grammar->grammar, qi::space, result);
            
            if (success && iter == end) {
                double calculatedResult = result->evaluate();
                container.addExpression(line, result, calculatedResult, true, "");
            } else {
                std::string errorMsg = "Parsing failed at line " + std::to_string(lineNumber);
                container.addExpression(line, nullptr, 0.0, false, errorMsg);
            }
        } catch (const std::exception& e) {
            std::string errorMsg = "Error at line " + std::to_string(lineNumber) + ": " + e.what();
            container.addExpression(line, nullptr, 0.0, false, errorMsg);
        }
    }
    
    return container;
}

void Calculator::printAST(const std::string& input) {
    try {
        auto iter = input.cbegin();
        auto end = input.cend();
        
        ExpressionPtr result;
        bool success = qi::phrase_parse(iter, end, grammar->grammar, qi::space, result);
        
        if (success && iter == end) {
            std::cout << "Input: " << input << std::endl;
            std::cout << "AST Structure:" << std::endl;
            result->print();
            double calculatedResult = result->evaluate();
            std::cout << "Result: " << calculatedResult << std::endl;
        } else {
            std::cout << "Input: " << input << std::endl;
            std::cout << "ERROR: Parsing failed" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Input: " << input << std::endl;
        std::cout << "ERROR: " << e.what() << std::endl;
    }
}

// ============================================================================
// 새로운 테스트 함수들
// ============================================================================

/**
 * 여러 표현식을 한 번에 파싱하고 처리하는 테스트
 */
void MultiExpressionTest() {
    Calculator calc;
    
    std::cout << "=== Multi-Expression Parsing Test ===" << std::endl;
    
    // 테스트할 표현식들 (중요한 것들만)
    std::vector<std::string> expressions = {
        "1 + 2 * 3",
        "10 / 2",
        "5 > 3",
        "3.14 * 2",
        "invalid expression +"
    };
    
    // 여러 표현식을 한 번에 파싱
    ExpressionContainer container = calc.parseMultipleExpressions(expressions);
    
    // 결과 출력
    container.printAllResults();
    
    // 개별 표현식 처리 예시
    std::cout << "\n=== Individual Expression Processing ===" << std::endl;
    for (size_t i = 0; i < container.size(); ++i) {
        const auto& expr = container.getExpression(i);
        std::cout << "Processing expression " << i << ": " << expr.originalText << std::endl;
        
        if (expr.isValid) {
            std::cout << "  Result: " << expr.result << std::endl;
        } else {
            std::cout << "  Error: " << expr.errorMessage << std::endl;
        }
    }
}

/**
 * 파일에서 표현식을 읽어서 파싱하는 테스트
 */
void FileParsingTest() {
    Calculator calc;
    
    std::cout << "=== File Parsing Test ===" << std::endl;
    
    // 테스트 파일 생성
    std::ofstream testFile("test_expressions.txt");
    if (testFile.is_open()) {
        testFile << "# This is a comment line\n";
        testFile << "1 + 2 * 3\n";
        testFile << "10 / 2\n";
        testFile << "5 > 3\n";
        testFile << "invalid expression +\n";
        testFile.close();
        
        // 파일에서 파싱
        ExpressionContainer container = calc.parseFromFile("test_expressions.txt");
        
        // 결과 출력
        container.printAllResults();
        
        // 파일 삭제
        std::remove("test_expressions.txt");
    }
}

/**
 * @brief 문자열 파싱 기능 테스트
 * @details 다중 줄 문자열에서 표현식을 파싱하고 결과를 분석
 *          문자열 스트림 처리 및 결과 타입 분석 기능을 테스트
 */
void StringParsingTest() {
    Calculator calc;
    
    std::cout << "=== String Parsing Test ===" << std::endl;
    
    // 여러 줄의 표현식이 포함된 문자열 (중요한 것들만)
    std::string multiLineInput = R"(
# 여러 줄의 표현식들
1 + 2 * 3
10 / 2
5 > 3
invalid expression +
)";
    
    // 문자열에서 파싱
    ExpressionContainer container = calc.parseFromString(multiLineInput);
    
    // 결과 출력
    container.printAllResults();
    
    // 컨테이너에서 표현식들을 하나씩 처리
    std::cout << "\n=== Processing Expressions from Container ===" << std::endl;
    for (size_t i = 0; i < container.size(); ++i) {
        const auto& expr = container.getExpression(i);
        
        // 각 표현식에 대한 개별 처리
        if (expr.isValid) {
            std::cout << "Expression " << i << " (" << expr.originalText << ") = " << expr.result << std::endl;
        } else {
            std::cout << "Expression " << i << " (" << expr.originalText << ") - ERROR: " << expr.errorMessage << std::endl;
        }
    }
}

/**
 * @brief 변수 기능 테스트
 * @details 변수 할당, 참조, 그리고 변수를 사용한 계산을 테스트
 *          변수 테이블 관리 및 스코프 기능을 검증
 */
void VariableTest() {
    Calculator calc;
    
    std::cout << "=== Variable Functionality Test ===" << std::endl;
    
    // 변수 테이블 초기화
    clearVariables();
    
    // 테스트할 변수 표현식들 (중요한 것들만)
    std::vector<std::string> variableTests = {
        // 기본 변수 할당
        "value = 10",
        "x = 3.14",
        
        // 변수를 사용한 계산
        "y = x * 2",
        "result = value + y",
        
        // 단항 연산자 테스트
        "neg_value = -value",      // 변수 부호 반전
        "not_value = not value",   // 변수 논리 부정
        "neg_x = -x",              // 실수 변수 부호 반전
        "not_x = not x",           // 실수 변수 논리 부정
        
        // 변수 참조
        "value",
        "x",
        
        // 비교 연산
        "is_greater = value > 5"
    };
    
    std::cout << "Testing variable assignments and references..." << std::endl;
    
    // 각 변수 테스트를 실행
    for (const auto& test : variableTests) {
        try {
            std::cout << "\nTest: " << test << std::endl;
            calc.printAST(test);
            
            // 변수 테이블 상태 출력 (할당 후에만)
            if (test.find('=') != std::string::npos) {
                std::cout << "Variable table after assignment:" << std::endl;
                printAllVariables();
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    // 최종 변수 테이블 상태 출력
    std::cout << "\n=== Final Variable Table ===" << std::endl;
    printAllVariables();
    
    // 변수 테이블 초기화
    clearVariables();
    std::cout << "Variable table cleared." << std::endl;
}

// ============================================================================
// 테스트 함수
// ============================================================================

/**
 * @brief 계산기 기능을 테스트하는 함수
 * @details 다양한 수학 표현식을 파싱하고 계산하여 결과를 출력
 *          정수와 실수가 구분되어 처리되는지 확인
 *          비교 연산자와 변수 기능도 테스트
 */
void CalculatorTest() {
    Calculator calc;  // 계산기 인스턴스 생성
    
    std::cout << "=== Calculator Test (Integer & Float with Type Distinction, Comparison Operators, and Variables) ===" << std::endl;
    
    // 변수 테이블 초기화
    clearVariables();
    
    // 테스트할 수학 표현식들 (중요한 것들만)
    std::vector<std::string> tests = {
        // 기본 수학 연산
        "1+2*3",                    // 정수만 사용한 기본 연산자 우선순위 테스트
        "10.5-5.2/2",              // 실수만 사용한 계산 테스트
        "(1.5+2.5)*3",             // 실수와 정수 혼합 괄호 우선순위 테스트
        
        // 단항 연산자 테스트
        "-5",                      // 부호 반전
        "not 0",                   // 논리 부정 (0 → 1)
        "not 10",                  // 논리 부정 (10 → 0)
        "-3.14",                   // 실수 부호 반전
        "not -5",                  // 복합 단항 연산
        "-x",                      // 변수 부호 반전
        "not y",                   // 변수 논리 부정
        
        // 비교 연산자 테스트
        "5 > 3",                   // 기본 비교 연산
        "10 <= 10",                // 같거나 작음
        "1+2*3 == 7",              // 계산 결과 비교
        
        // 변수 기능 테스트
        "x = 10",                  // 기본 변수 할당
        "y = x + 5",               // 변수를 사용한 계산
        "x",                       // 변수 참조
        "result = (x + y) * 2"     // 복합 변수 표현식
    };
    
    // 각 테스트 케이스를 실행
    for (const auto& test : tests) {
        try {
            std::cout << "\nTest: " << test << std::endl;
            calc.printAST(test);  // AST 출력 및 계산
            
            // 변수 할당 후 변수 테이블 출력
            if (test.find('=') != std::string::npos) {
                std::cout << "Variable table:" << std::endl;
                printAllVariables();
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;  // 오류 처리
        }
    }
    
    // 최종 변수 테이블 출력
    std::cout << "\n=== Final Variable Table ===" << std::endl;
    printAllVariables();
    
    // 변수 테이블 초기화
    clearVariables();
}