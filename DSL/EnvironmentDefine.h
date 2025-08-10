#pragma once

namespace dsl
{
    // Environment 변수 타입
    enum class EEnvValType
    {
        Base,
        Int,
        Float,
        String,
    };


    // Base
    struct EnvValBase
    {
        virtual EEnvValType GetValType() { return EEnvValType::Base; }
    };

    // Int 변수
    struct EnvValInt : EnvValBase
    {
        virtual EEnvValType GetValType() override { return EEnvValType::Int; }

        long long val;
    };

    // Float 변수
    struct EnvValFloat : EnvValBase
    {
        virtual EEnvValType GetValType() override { return EEnvValType::Float; }

        double val;
    };

    // String 변수
    struct EnvValString : EnvValBase
    {
        virtual EEnvValType GetValType() override { return EEnvValType::String; }

        std::wstring val;
    };

    using EnvValBasePtr = std::shared_ptr<EnvValBase>;
    using EnvValBaseCPtr = std::shared_ptr<const EnvValBase>;
    using EnvValIntPtr = std::shared_ptr<EnvValInt>;
    using EnvValIntCPtr = std::shared_ptr<const EnvValInt>;
    using EnvValFloatPtr = std::shared_ptr<EnvValFloat>;
    using EnvValFloatCPtr = std::shared_ptr<const EnvValFloat>;
    using EnvValStringPtr = std::shared_ptr<EnvValString>;
    using EnvValStringCPtr = std::shared_ptr<const EnvValString>;

    // CallStackInfo 상태
    enum class EEnvCallStackState
    {
        Init,   // 초기 상태
        Run,    // 실행 중
        Wait,   // 대기상태로 들어감
        Success,
        Error,  // 오류
    };

    // Call Stack 정보
    struct EnvCallStackInfo
    {
        const BaseCPtr      spBase; // 실행할 AST
        EEnvCallStackState  eState; // 상태
        int nLoopCount;             // 루프를 사용할 경우 현재 루프카운트

        EnvCallStackInfo() 
            : spBase(nullptr), eState(EEnvCallStackState::Init), nLoopCount(0)
        {}

        EnvCallStackInfo(const BaseCPtr& _spBase)
            : spBase(_spBase), eState(EEnvCallStackState::Init), nLoopCount(0)
        {}
    };

}