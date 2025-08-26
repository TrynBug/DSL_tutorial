#include "pch.h"

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
				// error
				std::cout << "AST type is base" << std::endl;
			}
			break;

			case EASTType::Name:
			{
				const NameCPtr& spName = static_pointer_cast<const Name>(inBase);


			}
			break;

			case EASTType::NameList:
			{

			}
			break;

			case EASTType::Numeral:
			{
				const NumeralCPtr& spNumeral = static_pointer_cast<const Numeral>(inBase);

				// call stack에 값이 없다면 넣어준다.
				if (inCallStackInfo.upVal == nullptr)
				{
					if (spNumeral->isInteger)
					{
						EnvValIntUptr upVal = std::make_unique<EnvValInt>();
						upVal->val = spNumeral->intValue;

						inCallStackInfo.upVal = std::move(upVal);
					}
					else
					{
						EnvValFloatUptr upVal = std::make_unique<EnvValFloat>();
						upVal->val = spNumeral->floatValue;

						inCallStackInfo.upVal = std::move(upVal);
					}
				}
			}
			break;

			case EASTType::Boolean:
			{

			}
			break;

			case EASTType::LiteralString:
			{
				const LiteralStringCPtr& spLiteralString = static_pointer_cast<const LiteralString>(inBase);

				// call stack에 값이 없다면 넣어준다.
				if (inCallStackInfo.upVal == nullptr)
				{
					EnvValStringUptr upVal = std::make_unique<EnvValString>();
					upVal->val = spLiteralString->value;

					inCallStackInfo.upVal = std::move(upVal);
				}
			}
			break;

			case EASTType::AST:
			{
				// call stack에 block을 추가하고 run 한다.
				const ASTCPtr& spAST = static_pointer_cast<const AST>(inBase);
				if (!spAST->block)
					return EEnvCallStackState::Error;

				m_localVariableStack.emplace_back();

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

				int& refLoopCount = inCallStackInfo.nLoopCount;
				for (; refLoopCount < spBlock->statements.size(); ++refLoopCount)
				{
					const BaseCPtr& spBase = spBlock->statements[refLoopCount];
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
				const AssignmentCPtr& spAssignment = static_pointer_cast<const Assignment>(inBase);

				const NameCPtr& spName = static_pointer_cast<const Name>(inBase);
				const ExpressionCPtr& spExpression = static_pointer_cast<const Expression>(inBase);
				if (!spName || !spExpression)
					return EEnvCallStackState::Error;

				EnvCallStackInfo& callStackInfo = m_callStack.emplace_back(spExpression);

				callStackInfo.eState = runCallStack(callStackInfo);
				if (EEnvCallStackState::Success != callStackInfo.eState)
					return callStackInfo.eState;

				std::unordered_map<std::wstring, EnvValBasePtr>& localVariableMap = m_localVariableStack.back();
				localVariableMap[spName->name];
			}
			break;

			case EASTType::Expression:
			{
				const ExpressionCPtr& spExpression = static_pointer_cast<const Expression>(inBase);
				if (!spExpression->expression)
					return EEnvCallStackState::Error;

				EnvCallStackInfo& callStackInfo = m_callStack.emplace_back(spExpression->expression);

				callStackInfo.eState = runCallStack(callStackInfo);
				if (EEnvCallStackState::Success != callStackInfo.eState)
					return callStackInfo.eState;


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
				const UnaryExpressionCPtr& spUnaryExpression = static_pointer_cast<const UnaryExpression>(inBase);

				const PrimaryExpressionCPtr& spPrimaryExpression = static_pointer_cast<const PrimaryExpression>(spUnaryExpression->primaryExpression);
				if (!spPrimaryExpression)
					return EEnvCallStackState::Error;

				EnvCallStackInfo& callStackInfo = m_callStack.emplace_back(spPrimaryExpression);

				callStackInfo.eState = runCallStack(callStackInfo);
				if (EEnvCallStackState::Success != callStackInfo.eState)
					return callStackInfo.eState;

				callStackInfo.upVal = executeUnaryOperator(std::move(callStackInfo.upVal), spUnaryExpression->unaryOperator);

			}
			break;

			case EASTType::FunctionDefinition:
			{
				// noting to do
				// 함수 선언은 스크립트를 등록할 때 처리했으므로 여기서는 아무것도 하지 않는다.
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



	EnvValBaseUptr Environment::executeUnaryOperator(EnvValBaseUptr upEnvValBase, const std::wstring& strOperator)
	{
		if (!upEnvValBase)
			return nullptr;

		const EEnvValType eEnvValType = upEnvValBase->GetValType();

		if (L"not" == strOperator)
		{
			switch (eEnvValType)
			{
				case EEnvValType::Bool:
				{
					EnvValBoolUptr upValBool(static_cast<EnvValBool*>(upEnvValBase.release()));
					upValBool->val = !upValBool->val;
					return upValBool;
				}
			}
		}
		else if (L"-" == strOperator)
		{
			switch (eEnvValType)
			{
				case EEnvValType::Int:
				{
					EnvValIntUptr upValInt(static_cast<EnvValInt*>(upEnvValBase.release()));
					upValInt->val = -upValInt->val;
					return upValInt;
				}

				case EEnvValType::Float:
				{
					EnvValFloatUptr upValFloat(static_cast<EnvValFloat*>(upEnvValBase.release()));
					upValFloat->val = -upValFloat->val;
					return upValFloat;
				}
			}
		}

		return nullptr;
	}


	EnvValBaseUptr executeBinaryOperator(EnvValBaseUptr upLeftEnvValBase, const std::wstring& strOperator, EnvValBaseUptr upRightEnvValBase)
	{
		if (!upLeftEnvValBase || !upRightEnvValBase)
			return nullptr;

		auto isInteger = [](const EEnvValType eValType)
		{
			return (eValType == EEnvValType::Bool || eValType == EEnvValType::Int);
		};

		auto isFloat = [](const EEnvValType eValType)
		{
			return (eValType == EEnvValType::Float);
		};

		auto isNumeral = [](const EEnvValType eValType)
		{
			return (eValType == EEnvValType::Bool || eValType == EEnvValType::Int || eValType == EEnvValType::Float);
		};

		const EEnvValType eLeftType = upLeftEnvValBase->GetValType();
		const EEnvValType eRightType = upLeftEnvValBase->GetValType();

		if (L"*" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
			{
				if (isFloat(eLeftType) || isFloat(eRightType))
					return std::make_unique<EnvValFloat>(upLeftEnvValBase->GetFloat() * upRightEnvValBase->GetFloat());
				else
					return std::make_unique<EnvValInt>(upLeftEnvValBase->GetInt() * upRightEnvValBase->GetInt());
			}
		}
		else if (L"/" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
			{
				if (upRightEnvValBase->GetInt() == 0)
					return nullptr;

				if (isFloat(eLeftType) || isFloat(eRightType))
					return std::make_unique<EnvValFloat>(upLeftEnvValBase->GetFloat() / upRightEnvValBase->GetFloat());
				else
					return std::make_unique<EnvValInt>(upLeftEnvValBase->GetInt() / upRightEnvValBase->GetInt());
			}
		}
		else if (L"%" == strOperator)
		{
			if (isInteger(eLeftType) && isInteger(eRightType))
			{
				if (upRightEnvValBase->GetInt() == 0)
					return nullptr;

				return std::make_unique<EnvValInt>(upLeftEnvValBase->GetInt() % upRightEnvValBase->GetInt());
			}
		}
		else if (L"+" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
			{
				if (isFloat(eLeftType) || isFloat(eRightType))
					return std::make_unique<EnvValFloat>(upLeftEnvValBase->GetFloat() + upRightEnvValBase->GetFloat());
				else
					return std::make_unique<EnvValInt>(upLeftEnvValBase->GetInt() + upRightEnvValBase->GetInt());
			}
		}
		else if (L"-" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
			{
				if (isFloat(eLeftType) || isFloat(eRightType))
					return std::make_unique<EnvValFloat>(upLeftEnvValBase->GetFloat() - upRightEnvValBase->GetFloat());
				else
					return std::make_unique<EnvValInt>(upLeftEnvValBase->GetInt() - upRightEnvValBase->GetInt());
			}
		}
		else if (L"<" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
				return std::make_unique<EnvValBool>(upLeftEnvValBase->GetFloat() < upRightEnvValBase->GetFloat());
		}
		else if (L">" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
				return std::make_unique<EnvValBool>(upLeftEnvValBase->GetFloat() > upRightEnvValBase->GetFloat());
		}
		else if (L"<=" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
				return std::make_unique<EnvValBool>(upLeftEnvValBase->GetFloat() <= upRightEnvValBase->GetFloat());
		}
		else if (L">=" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
				return std::make_unique<EnvValBool>(upLeftEnvValBase->GetFloat() >= upRightEnvValBase->GetFloat());
		}
		else if (L"==" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
				return std::make_unique<EnvValBool>(upLeftEnvValBase->GetFloat() == upRightEnvValBase->GetFloat());
		}
		else if (L"!=" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
				return std::make_unique<EnvValBool>(upLeftEnvValBase->GetFloat() != upRightEnvValBase->GetFloat());
		}
		else if (L"and" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
				return std::make_unique<EnvValBool>(upLeftEnvValBase->GetBool() && upRightEnvValBase->GetBool());
		}
		else if (L"or" == strOperator)
		{
			if (isNumeral(eLeftType) && isNumeral(eRightType))
				return std::make_unique<EnvValBool>(upLeftEnvValBase->GetBool() || upRightEnvValBase->GetBool());
		}

		return nullptr;
	}
}