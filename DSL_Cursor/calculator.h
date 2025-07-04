/**
 * @file calculator.h
 * @brief DSL Parser의 핵심 계산기 클래스 정의
 * @details Boost.Spirit를 사용한 수학 표현식 파서와 AST 처리 기능을 제공
 * 
 * 주요 기능:
 * - 수학 표현식 파싱 (+, -, *, /, ==, <, >, <=, >=)
 * - AST (Abstract Syntax Tree) 생성 및 평가
 * - 다중 표현식 일괄 처리
 * - 파일 및 문자열에서 표현식 읽기
 * - 정수/실수 타입 구분 처리
 */

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "calculator_grammar.h"

// Forward declarations
struct Expression;
using ExpressionPtr = std::shared_ptr<Expression>;

template<typename Iterator>
struct CalculatorGrammar;

/**
 * @brief 표현식 포인터 타입 별칭
 * @details std::shared_ptr를 사용하여 메모리 관리를 자동화
 */
struct ParsedExpression {
    std::string originalText;    ///< 원본 표현식 텍스트
    ExpressionPtr ast;           ///< 파싱된 AST (Abstract Syntax Tree)
    double result;               ///< 계산된 결과값
    bool isValid;                ///< 파싱 성공 여부
    std::string errorMessage;    ///< 오류 발생 시 오류 메시지
    
    /**
     * @brief 생성자
     * @param text 원본 표현식 텍스트
     * @param astPtr 파싱된 AST 포인터
     * @param res 계산 결과
     * @param valid 파싱 성공 여부
     * @param error 오류 메시지
     */
    ParsedExpression(const std::string& text, ExpressionPtr astPtr, 
                     double res, bool valid, const std::string& error)
        : originalText(text), ast(astPtr), result(res), isValid(valid), errorMessage(error) {}
};

/**
 * @brief 여러 표현식을 저장하고 관리하는 컨테이너 클래스
 * @details 파싱된 표현식들을 벡터로 저장하고, 일괄 처리 기능을 제공
 */
class ExpressionContainer {
private:
    std::vector<ParsedExpression> expressions;  ///< 파싱된 표현식들을 저장하는 벡터

public:
    /**
     * @brief 표현식을 컨테이너에 추가
     * @param text 원본 표현식 텍스트
     * @param ast 파싱된 AST
     * @param result 계산 결과
     * @param isValid 파싱 성공 여부
     * @param error 오류 메시지
     */
    void addExpression(const std::string& text, ExpressionPtr ast, 
                      double result, bool isValid, const std::string& error);
    
    /**
     * @brief 특정 인덱스의 표현식을 반환
     * @param index 표현식 인덱스
     * @return 해당 인덱스의 ParsedExpression 참조
     * @throws std::out_of_range 인덱스가 범위를 벗어난 경우
     */
    const ParsedExpression& getExpression(size_t index) const;
    
    /**
     * @brief 컨테이너에 저장된 모든 표현식의 결과를 출력
     * @details 성공한 표현식은 결과값과 함께, 실패한 표현식은 오류 메시지와 함께 출력
     */
    void printAllResults() const;
    
    /**
     * @brief 컨테이너에 저장된 모든 표현식의 AST 구조를 출력
     * @details 각 표현식의 AST를 트리 형태로 출력하고 계산 결과도 함께 표시
     */
    void printASTs() const;
    
    /**
     * @brief 컨테이너에 저장된 표현식의 개수를 반환
     * @return 표현식 개수
     */
    size_t size() const { return expressions.size(); }
    
    /**
     * @brief 컨테이너가 비어있는지 확인
     * @return true if empty, false otherwise
     */
    bool empty() const { return expressions.empty(); }
    
    /**
     * @brief 컨테이너의 모든 표현식을 제거
     */
    void clear() { expressions.clear(); }
};

/**
 * @brief DSL Parser의 핵심 계산기 클래스
 * @details Boost.Spirit 파서를 사용하여 수학 표현식을 파싱하고 AST를 생성
 * 
 * 지원하는 기능:
 * - 기본 수학 연산: +, -, *, /
 * - 비교 연산: ==, <, >, <=, >=
 * - 괄호를 통한 우선순위 제어
 * - 정수/실수 혼합 계산
 * - 다중 표현식 일괄 처리
 * - 파일 및 문자열에서 표현식 읽기
 */
class Calculator {
private:
    // Boost.Spirit 파서 문법 인스턴스 (calculator_grammar.h에 정의됨)
    // 구체적인 타입은 calculator.cpp에서 정의됨
    struct GrammarImpl;
    std::unique_ptr<GrammarImpl> grammar;

public:
    /**
     * @brief 기본 생성자
     * @details Boost.Spirit 파서 문법을 초기화
     */
    Calculator();
    
    /**
     * @brief 소멸자
     * @details 동적 할당된 리소스를 정리
     */
    ~Calculator();
    
    /**
     * @brief 단일 표현식을 파싱하고 AST를 출력
     * @param input 파싱할 수학 표현식 문자열
     * @details 표현식을 파싱하여 AST 구조를 출력하고 계산 결과를 표시
     */
    void printAST(const std::string& input);
    
    /**
     * @brief 여러 표현식을 한 번에 파싱하여 컨테이너에 저장
     * @param inputs 파싱할 표현식들의 벡터
     * @return 파싱 결과를 담은 ExpressionContainer
     * @details 각 표현식을 개별적으로 파싱하고 결과를 컨테이너에 저장
     */
    ExpressionContainer parseMultipleExpressions(const std::vector<std::string>& inputs);
    
    /**
     * @brief 파일에서 표현식들을 읽어서 파싱
     * @param filename 읽을 파일명
     * @return 파싱 결과를 담은 ExpressionContainer
     * @details 파일의 각 줄을 읽어서 표현식으로 파싱
     *          빈 줄과 주석(#, ;)은 무시
     */
    ExpressionContainer parseFromFile(const std::string& filename);
    
    /**
     * @brief 다중 줄 문자열에서 표현식들을 파싱
     * @param multiLineInput 파싱할 다중 줄 문자열
     * @return 파싱 결과를 담은 ExpressionContainer
     * @details 문자열의 각 줄을 읽어서 표현식으로 파싱
     *          빈 줄과 주석(#, ;)은 무시
     */
    ExpressionContainer parseFromString(const std::string& multiLineInput);
};

// 전역 테스트 함수들 (calculator.cpp에 구현됨)

/**
 * @brief 기본 계산기 기능 테스트
 * @details 다양한 수학 표현식을 파싱하고 계산하여 결과를 출력
 *          정수/실수 구분, 연산자 우선순위, 비교 연산 등을 테스트
 */
void CalculatorTest();

/**
 * @brief 다중 표현식 처리 테스트
 * @details 여러 표현식을 한 번에 파싱하고 결과를 컨테이너에 저장
 *          일괄 처리 및 개별 결과 접근 기능을 테스트
 */
void MultiExpressionTest();

/**
 * @brief 파일 파싱 기능 테스트
 * @details 임시 파일을 생성하고 파일에서 표현식을 읽어서 파싱
 *          파일 I/O, 주석 처리, 오류 처리 기능을 테스트
 */
void FileParsingTest();

/**
 * @brief 문자열 파싱 기능 테스트
 * @details 다중 줄 문자열에서 표현식을 파싱하고 결과를 분석
 *          문자열 스트림 처리 및 결과 타입 분석 기능을 테스트
 */
void StringParsingTest();

/**
 * @brief 변수 기능 테스트
 * @details 변수 할당, 참조, 그리고 변수를 사용한 계산을 테스트
 *          변수 테이블 관리 및 스코프 기능을 검증
 */
void VariableTest();

// ============================================================================
// 변수 관리 함수들
// ============================================================================

/**
 * @brief 변수 값을 설정
 * @param name 변수 이름
 * @param value 설정할 값
 */
void setVariable(const std::string& name, double value);

/**
 * @brief 변수 값을 조회
 * @param name 변수 이름
 * @return 변수 값 (정의되지 않은 경우 0.0)
 */
double getVariable(const std::string& name);

/**
 * @brief 변수가 정의되어 있는지 확인
 * @param name 변수 이름
 * @return true if defined, false otherwise
 */
bool isVariableDefined(const std::string& name);

/**
 * @brief 모든 변수를 출력
 * @details 디버깅 목적으로 사용
 */
void printAllVariables();

/**
 * @brief 변수 테이블을 초기화
 * @details 모든 변수를 제거
 */
void clearVariables();

#endif // CALCULATOR_H


