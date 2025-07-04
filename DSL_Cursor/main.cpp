/**
 * @file main.cpp
 * @brief DSL Parser 메인 실행 파일
 * @details Boost.Spirit를 사용한 수학 표현식 파서의 모든 테스트를 순차적으로 실행
 * 
 * 이 프로그램은 다음과 같은 기능들을 테스트합니다:
 * 1. 기본 계산기 테스트 - 단일 수학 표현식 파싱 및 계산
 * 2. 다중 표현식 테스트 - 여러 표현식을 한 번에 처리
 * 3. 파일 파싱 테스트 - 파일에서 표현식 읽기
 * 4. 문자열 파싱 테스트 - 다중 줄 문자열에서 표현식 파싱
 */

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

#include "parser.h"
#include "calculator.h"

/**
 * @brief 메인 함수
 * @details DSL Parser의 모든 테스트를 순차적으로 실행
 * 
 * 실행 순서:
 * 1. 기본 계산기 테스트 - 수학 표현식 파싱 및 계산 검증
 * 2. 다중 표현식 테스트 - 여러 표현식의 일괄 처리
 * 3. 파일 파싱 테스트 - 파일 I/O 및 파싱 기능
 * 4. 문자열 파싱 테스트 - 다중 줄 문자열 처리
 * 5. 변수 기능 테스트
 * 
 * 각 테스트는 구분선과 함께 실행되어 결과를 명확히 구분할 수 있습니다.
 */
int main() {
    // 프로그램 시작 메시지 출력
    std::cout << "=== DSL Parser - Variable Test Only ===" << std::endl;
    std::cout << "Running variable test..." << std::endl;
    
    // 변수 기능 테스트만 실행
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "VARIABLE FUNCTIONALITY TEST" << std::endl;
    std::cout << "Test: Variable assignment, reference, and calculation using variables" << std::endl;
    std::cout << "Features: variable table management, scope, identifier parsing" << std::endl;
    VariableTest();
    
    // ============================================================================
    // 최종 요약
    // ============================================================================
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "VARIABLE TEST COMPLETED!" << std::endl;
    std::cout << "Press Enter to exit the program...";
    std::cin.get();
    
    return 0;
} 