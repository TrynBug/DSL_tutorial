# ============================================================================
# CMakeLists.txt - DSL Parser 프로젝트 빌드 설정
# ============================================================================
# 
# 이 파일은 DSL Parser 프로젝트의 빌드 설정을 정의합니다.
# 
# 주요 구성 요소:
# 1. 프로젝트 기본 설정
# 2. C++ 표준 설정
# 3. Boost 라이브러리 의존성 설정
# 4. 소스 파일 구성
# 5. 실행 파일 생성
# 6. 컴파일러 옵션 설정
# 
# 의존성:
# - CMake 3.10 이상
# - C++17 호환 컴파일러
# - Boost 라이브러리 (Spirit, Phoenix)
# ============================================================================

# 최소 CMake 버전 요구사항 설정
cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름과 버전 설정
project(DSL_Parser VERSION 1.0.0)

# 프로젝트 설명
set(PROJECT_DESCRIPTION "DSL Parser using Boost.Spirit for mathematical expression parsing")

# ============================================================================
# C++ 표준 설정
# ============================================================================

# C++17 표준 사용 설정
# Boost.Spirit와 Phoenix 기능을 위해 C++17 이상이 필요
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 컴파일러 특정 플래그 설정
if(MSVC)
    # Visual Studio 컴파일러 설정
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang 컴파일러 설정
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# ============================================================================
# Boost 라이브러리 설정
# ============================================================================

# Boost 라이브러리 찾기
# Boost.Spirit와 Phoenix 컴포넌트가 필요
find_package(Boost REQUIRED COMPONENTS 
    system      # 시스템 관련 기능
    filesystem  # 파일 시스템 접근
    program_options  # 명령행 옵션 처리 (향후 확장용)
)

# Boost 헤더 파일 경로 설정
include_directories(${Boost_INCLUDE_DIRS})

# ============================================================================
# 소스 파일 구성
# ============================================================================

# 헤더 파일들
set(HEADER_FILES
    calculator.h          # 계산기 클래스 정의
    calculator_grammar.h  # Boost.Spirit 파서 문법 정의
)

# 소스 파일들
set(SOURCE_FILES
    main.cpp             # 메인 실행 파일
    calculator.cpp       # 계산기 클래스 구현
)

# ============================================================================
# 실행 파일 생성
# ============================================================================

# 메인 실행 파일 생성
add_executable(DSL_Parser ${SOURCE_FILES} ${HEADER_FILES})

# 헤더 파일 경로 설정
target_include_directories(DSL_Parser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Boost 라이브러리 링크
target_link_libraries(DSL_Parser ${Boost_LIBRARIES})

# ============================================================================
# 빌드 출력 디렉토리 설정
# ============================================================================

# 실행 파일 출력 디렉토리 설정
set_target_properties(DSL_Parser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ============================================================================
# 컴파일러 특정 설정
# ============================================================================

# Windows 특정 설정
if(WIN32)
    # Windows에서 콘솔 애플리케이션으로 설정
    set_target_properties(DSL_Parser PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
    
    # Windows에서 Boost 라이브러리 링크 설정
    if(MSVC)
        target_compile_definitions(DSL_Parser PRIVATE
            _CRT_SECURE_NO_WARNINGS  # 보안 경고 비활성화
            NOMINMAX                 # min/max 매크로 충돌 방지
        )
    endif()
endif()

# ============================================================================
# 디버그 정보 설정
# ============================================================================

# 디버그 모드에서 상세한 정보 출력
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(DSL_Parser PRIVATE
        BOOST_SPIRIT_DEBUG  # Boost.Spirit 디버그 정보 활성화
    )
endif()

# ============================================================================
# 설치 설정 (선택사항)
# ============================================================================

# 설치 규칙 설정 (향후 패키징을 위해)
install(TARGETS DSL_Parser
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 헤더 파일 설치
install(FILES ${HEADER_FILES}
    DESTINATION include/dsl_parser
)

# ============================================================================
# 테스트 설정 (향후 확장용)
# ============================================================================

# 테스트 활성화 (선택사항)
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing()
    # 여기에 테스트 설정을 추가할 수 있습니다
endif()

# ============================================================================
# 빌드 완료 메시지
# ============================================================================

message(STATUS "DSL Parser project configured successfully")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/bin") 