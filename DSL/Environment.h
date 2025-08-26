#pragma once

namespace dsl
{

    class Environment
    {
    public:
        Environment();

    public:
        bool Initialize(const ASTCPtr spAST);

    public:
        bool RunOneStep();
        bool Run();


    private:
        EEnvCallStackState runCallStack(EnvCallStackInfo& inCallStackInfo);
        EnvValBaseUptr executeUnaryOperator(EnvValBaseUptr upEnvValBase, const std::wstring& strOperator);
        EnvValBaseUptr executeBinaryOperator(EnvValBaseUptr upLeftEnvValBase, const std::wstring& strOperator, EnvValBaseUptr upRightEnvValBase);


    private:
        ASTCPtr m_spAST;

        std::vector<EnvCallStackInfo> m_callStack;

        std::unordered_map<std::wstring, EnvValBasePtr> m_globalVariableMap;
        std::vector<std::unordered_map<std::wstring, EnvValBasePtr>> m_localVariableStack;
    };

    using EnvironmentPtr = std::shared_ptr<Environment>;
    using EnvironmentCPtr = std::shared_ptr<const Environment>;


}