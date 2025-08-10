#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <stack>
#include <thread>
#include <chrono>
#include <unordered_map>

#include "ast.h"

#include "EnvironmentDefine.h"
#include "Environment.h"


namespace dsl
{

	Environment::Environment()
	{
		
	}

	bool Environment::Initialize(const ASTCPtr spAST)
	{
		if (!spAST)
			return false;

		m_spAST = spAST;

		m_callStack.emplace_back(m_spAST);

		return true;
	}

	bool Environment::RunOneStep()
	{
		// 미완성
		if (m_callStack.empty())
			return false;

		const EnvCallStackInfo& callStackInfo = m_callStack.back();
		const BaseCPtr& spBase = callStackInfo.spBase;
		if (!spBase)
			return false;


		return true;
	}

	bool Environment::Run()
	{
		// 미완성
		if (m_callStack.empty())
			return false;

		return true;
	}


	// AST를 실행한다.
	EEnvCallStackState Environment::runCallStack(EnvCallStackInfo& inCallStackInfo)
	{
		const BaseCPtr& inBase = inCallStackInfo.spBase;
		if (!inBase)
			return EEnvCallStackState::Error;

		const EASTType eASTType = inBase->GetType();
		switch (eASTType)
		{
			case EASTType::Base:
			{
				
			}
			break;

			case EASTType::Name:
			{
				
			}
			break;

			case EASTType::NameList:
			{

			}
			break;

			case EASTType::Numeral:
			{
				const NumeralCPtr& spNumeral = static_pointer_cast<const Numeral>(inBase);


			}
			break;

			case EASTType::Boolean:
			{

			}
			break;

			case EASTType::LiteralString:
			{

			}
			break;

			case EASTType::AST:
			{
				const ASTCPtr& spAST = static_pointer_cast<const AST>(inBase);
				if (!spAST->block)
					return EEnvCallStackState::Error;

				EnvCallStackInfo& callStackInfo = m_callStack.emplace_back(spAST->block);
				callStackInfo.eState = runCallStack(callStackInfo);

				if (EEnvCallStackState::Success == callStackInfo.eState)
					m_callStack.pop_back();
				else
					return callStackInfo.eState;
			}
			break;

			case EASTType::Block:
			{
				const BlockCPtr& spBlock = static_pointer_cast<const Block>(inBase);

				int& nLoopCount = inCallStackInfo.nLoopCount;
				for (; nLoopCount < spBlock->statements.size(); ++nLoopCount)
				{
					const BaseCPtr& spBase = spBlock->statements[nLoopCount];
					EnvCallStackInfo& callStackInfo = m_callStack.emplace_back(spBase);

					callStackInfo.eState = runCallStack(callStackInfo);

					if (EEnvCallStackState::Success == callStackInfo.eState)
						m_callStack.pop_back();
					else
						return callStackInfo.eState;
				}
			}
			break;

			case EASTType::Assignment:
			{

			}
			break;

			case EASTType::Expression:
			{

			}
			break;

			case EASTType::ExpressionList:
			{

			}
			break;

			case EASTType::PrimaryExpression:
			{

			}
			break;

			case EASTType::BinaryExpression:
			{

			}
			break;

			case EASTType::UnaryExpression:
			{

			}
			break;

			case EASTType::FunctionName:
			{

			}
			break;

			case EASTType::FunctionDefinition:
			{

			}
			break;

			case EASTType::FunctionParameter:
			{

			}
			break;

			case EASTType::FunctionCall:
			{

			}
			break;

			case EASTType::FunctionArgument:
			{

			}
			break;

			case EASTType::Statement:
			{
				const StatementCPtr& spStatement = static_pointer_cast<const Statement>(inBase);
				if (!spStatement->statement)
					return EEnvCallStackState::Error;

				EnvCallStackInfo& callStackInfo = m_callStack.emplace_back(spStatement->statement);
				callStackInfo.eState = runCallStack(callStackInfo);

				if (EEnvCallStackState::Success == callStackInfo.eState)
					m_callStack.pop_back();
				else
					return callStackInfo.eState;
			}
			break;

			case EASTType::Return:
			{

			}
			break;

			case EASTType::Break:
			{

			}
			break;

			case EASTType::While:
			{

			}
			break;

			case EASTType::If:
			{

			}
			break;

			case EASTType::For:
			{

			}
			break;

		}


		return EEnvCallStackState::Error;
	}

}