#pragma once

namespace dsl
{

    enum class EEnvValType
    {
        Base,
        Int,
        Float,
        String,
    };

    struct EnvValBase
    {

    };

    struct EnvValInt : EnvValBase
    {
        static const EEnvValType GetValType() { return EEnvValType::Base; }

        long long val;
    };

    struct EnvValFloat : EnvValBase
    {
        double val;
    };

    struct EnvValString : EnvValBase
    {
        std::wstring val;
    };

    using EnvValBasePtr = std::shared_ptr<EnvValBase>;
    using EnvValIntPtr = std::shared_ptr<EnvValInt>;
    using EnvValFloatPtr = std::shared_ptr<EnvValFloat>;
    using EnvValStringPtr = std::shared_ptr<EnvValString>;


    class Executor
    {
    public:
        Executor();

    public:
        bool Initialize(const ASTPtr spAST);


    private:
        ASTPtr m_spAST;

        std::vector<BasePtr> m_callStack;
        std::vector<std::unordered_map<std::wstring, EnvValBasePtr>> m_variableStack;
    };

    using ExecutorPtr = std::shared_ptr<Executor>;


}