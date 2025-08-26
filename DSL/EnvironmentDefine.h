#pragma once

namespace dsl
{
    // Environment 변수 타입
    enum class EEnvValType
    {
        Base,
        Bool,
        Int,
        Float,
        String,
    };


    // Base
    struct EnvValBase
    {
        virtual EEnvValType GetValType() const { return EEnvValType::Base; }

        virtual bool GetBool() const { return false; }
        virtual long long GetInt() const { return 0; }
        virtual double GetFloat() const { return 0.0; }
        virtual std::wstring GetString() const { return L""; }
    };

    // Bool 변수
    struct EnvValBool : EnvValBase
    {
        EnvValBool() {}
        EnvValBool(const bool inVal) : val(inVal) {}

        virtual EEnvValType GetValType() const override { return EEnvValType::Bool; }

        virtual bool GetBool() const override { return val; }
        virtual long long GetInt() const override { return static_cast<long long>(val); }
        virtual double GetFloat() const override  { return static_cast<double>(val); }

        bool val = false;
    };

    // Int 변수
    struct EnvValInt : EnvValBase
    {
        EnvValInt() {}
        EnvValInt(const long long inVal) : val(inVal) {}

        virtual EEnvValType GetValType() const override { return EEnvValType::Int; }

        virtual bool GetBool() const override { return static_cast<bool>(val); }
        virtual long long GetInt() const override { return val; }
        virtual double GetFloat() const override { return static_cast<double>(val); }

        long long val = 0;
    };

    // Float 변수
    struct EnvValFloat : EnvValBase
    {
        EnvValFloat() {}
        EnvValFloat(const double inVal) : val(inVal) {}

        virtual EEnvValType GetValType() const override { return EEnvValType::Float; }

        virtual bool GetBool() const override { return static_cast<bool>(val); }
        virtual long long GetInt() const override { return static_cast<long long>(val); }
        virtual double GetFloat() const override { return val; }

        double val = 0.0;
    };

    // String 변수
    struct EnvValString : EnvValBase
    {
        EnvValString() {}
        EnvValString(const std::wstring& inVal) : val(inVal) {}

        virtual EEnvValType GetValType() const override { return EEnvValType::String; }

        virtual std::wstring GetString() const { return val; }

        std::wstring val;
    };

    using EnvValBasePtr = std::shared_ptr<EnvValBase>;
    using EnvValBaseCPtr = std::shared_ptr<const EnvValBase>;
    using EnvValBaseUptr = std::unique_ptr<EnvValBase>;
    using EnvValBaseCUptr = std::unique_ptr<const EnvValBase>;
    using EnvValBoolPtr = std::shared_ptr<EnvValBool>;
    using EnvValBoolCPtr = std::shared_ptr<const EnvValBool>;
    using EnvValBoolUptr = std::unique_ptr<EnvValBool>;
    using EnvValBoolCUptr = std::unique_ptr<const EnvValBool>;
    using EnvValIntPtr = std::shared_ptr<EnvValInt>;
    using EnvValIntCPtr = std::shared_ptr<const EnvValInt>;
    using EnvValIntUptr = std::unique_ptr<EnvValInt>;
    using EnvValIntCUptr = std::unique_ptr<const EnvValInt>;
    using EnvValFloatPtr = std::shared_ptr<EnvValFloat>;
    using EnvValFloatCPtr = std::shared_ptr<const EnvValFloat>;
    using EnvValFloatUptr = std::unique_ptr<EnvValFloat>;
    using EnvValFloatCUptr = std::unique_ptr<const EnvValFloat>;
    using EnvValStringPtr = std::shared_ptr<EnvValString>;
    using EnvValStringCPtr = std::shared_ptr<const EnvValString>;
    using EnvValStringUptr = std::unique_ptr<EnvValString>;
    using EnvValStringCUptr = std::unique_ptr<const EnvValString>;

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
        EnvValBaseUptr      upVal;  // 결과값
        int                 nLoopCount; // 루프를 사용할 경우 현재 루프카운트

        EnvCallStackInfo() 
            : spBase(nullptr), eState(EEnvCallStackState::Init), upVal(nullptr), nLoopCount(0)
        {}

        EnvCallStackInfo(const BaseCPtr& _spBase)
            : spBase(_spBase), eState(EEnvCallStackState::Init), upVal(nullptr), nLoopCount(0)
        {}
    };

}