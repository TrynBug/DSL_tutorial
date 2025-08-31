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
		m_callStack.clear();
		m_globalVariableMap.clear();
		m_localVariableStack.clear();

		m_localVariableStack.emplace_back();

		insertCallStack(m_spAST);

		return true;
	}

	// 실행
	bool Environment::Run()
	{
		if (m_callStack.empty())
			return true;

		while (true)
		{
			// 현재 call stack 상태
			const EEnvCallStackState eCallStackState = getCallStackState();

			if (EEnvCallStackState::Error == eCallStackState)
			{
				return false;
			}
			else if (EEnvCallStackState::Init == eCallStackState)
			{
				// init 이면 call stack을 준비한다.
				if (false == prepareCallStack())
				{
					setCallStackState(EEnvCallStackState::Error);
					return false;
				}
			}
			else if (EEnvCallStackState::Run == eCallStackState || EEnvCallStackState::Wait == eCallStackState)
			{
				// run 또는 wait이면 call stack을 실행한다.
				if (false == runCallStack())
				{
					setCallStackState(EEnvCallStackState::Error);
					return false;
				}
			}
			else if (EEnvCallStackState::Success == eCallStackState)
			{
				// call stack이 완료됐으면 제거한다.
				if (m_callStack.empty())
					return false;

				m_callStack.pop_back();
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	EnvCallStackInfo& Environment::insertCallStack(const BaseCPtr& spBase)
	{
		EnvCallStackInfo& callStackInfo = m_callStack.emplace_back(spBase);
		return callStackInfo;
	}

	EEnvCallStackState Environment::getCallStackState()
	{
		if (m_callStack.empty())
			return EEnvCallStackState::Error;

		const EnvCallStackInfo& callStackInfo = m_callStack.back();
		return callStackInfo.eState;
	}

	const BaseCPtr Environment::getCallStackAST()
	{
		if (m_callStack.empty())
			return nullptr;

		const EnvCallStackInfo& callStackInfo = m_callStack.back();
		return callStackInfo.spBase;
	}

	void Environment::setCallStackState(const EEnvCallStackState eState)
	{
		if (m_callStack.empty())
			return;

		EnvCallStackInfo& callStackInfo = m_callStack.back();
		callStackInfo.eState = eState;
	}

	// Call Stack을 준비한다.
	bool Environment::prepareCallStack()
	{
		// prepare는 call stack이 Init 상태일때만 실행된다.
		EEnvCallStackState eCallStackState = getCallStackState();
		if (EEnvCallStackState::Init != eCallStackState)
			return false;

		const BaseCPtr spBase = getCallStackAST();
		if (!spBase)
			return false;

		// call stack 상태를 run으로 변경
		setCallStackState(EEnvCallStackState::Run);

		// AST 타입별로 Call Stack에 멤버들을 추가한다.
		const EASTType eASTType = spBase->GetType();
		switch (eASTType)
		{
		case EASTType::Base:
			return false;

		case EASTType::Name:
			return true;

		case EASTType::NameList:
			return true;

		case EASTType::Numeral:
			return true;

		case EASTType::Boolean:
			return true;

		case EASTType::LiteralString:
			return true;

		case EASTType::AST:
		{
			const ASTCPtr& spAST = static_pointer_cast<const AST>(spBase);
			m_callStack.emplace_back(spAST->block);
			return true;
		}

		case EASTType::Block:
		{
			const BlockCPtr& spBlock = static_pointer_cast<const Block>(spBase);
			for (size_t i = spBlock->statements.size() - 1; i <= 0; --i)
			{
				m_callStack.emplace_back(spBlock->statements[i]);
			}
			return true;
		}

		case EASTType::Assignment:
		{
			const AssignmentCPtr& spAssignment = static_pointer_cast<const Assignment>(spBase);
			m_callStack.emplace_back(spAssignment->expression);
			m_callStack.emplace_back(spAssignment->name);
			return true;
		}

		case EASTType::Expression:
		{
			const ExpressionCPtr& spExpression = static_pointer_cast<const Expression>(spBase);
			m_callStack.emplace_back(spExpression->expression);
			return true;
		}

		case EASTType::ExpressionList:
		{
			const ExpressionListCPtr& spExpressionList = static_pointer_cast<const ExpressionList>(spBase);
			for (int i = spExpressionList->expressions.size() - 1; i <= 0; --i)
			{
				m_callStack.emplace_back(spExpressionList->expressions[i]);
			}
			return true;
		}

		case EASTType::PrimaryExpression:
		{
			const PrimaryExpressionCPtr& spPrimaryExpression = static_pointer_cast<const PrimaryExpression>(spBase);
			m_callStack.emplace_back(spPrimaryExpression->primaryExpression);
			return true;
		}

		case EASTType::BinaryExpression:
		{
			const BinaryExpressionCPtr& spBinaryExpression = static_pointer_cast<const BinaryExpression>(spBase);
			m_callStack.emplace_back(spBinaryExpression->primaryExpression2);
			m_callStack.emplace_back(spBinaryExpression->primaryExpression1);
			return true;
		}

		case EASTType::UnaryExpression:
		{
			const UnaryExpressionCPtr& spUnaryExpression = static_pointer_cast<const UnaryExpression>(spBase);
			m_callStack.emplace_back(spUnaryExpression->primaryExpression);
			return true;
		}

		case EASTType::FunctionDefinition:
			// noting to do
			// 함수 선언은 스크립트를 등록할 때 처리했으므로 여기서는 아무것도 하지 않는다.
			return true;

		case EASTType::FunctionParameter:
		{
			const FunctionParameterCPtr& spFunctionParameter = static_pointer_cast<const FunctionParameter>(spBase);
			m_callStack.emplace_back(spFunctionParameter->nameList);
			return true;
		}

		case EASTType::FunctionCall:
		{
			const FunctionCallCPtr& spFunctionCall = static_pointer_cast<const FunctionCall>(spBase);
			m_callStack.emplace_back(spFunctionCall->functionArgument);
			m_callStack.emplace_back(spFunctionCall->name);
			return true;
		}

		case EASTType::FunctionArgument:
		{
			const FunctionArgumentCPtr& spFunctionArgument = static_pointer_cast<const FunctionArgument>(spBase);
			m_callStack.emplace_back(spFunctionArgument->expressionList);
			return true;
		}

		case EASTType::Statement:
		{
			const StatementCPtr& spStatement = static_pointer_cast<const Statement>(spBase);
			m_callStack.emplace_back(spStatement->statement);
			return true;
		}

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

		return false;
	}


	// call stack을 실행한다.
	bool Environment::runCallStack()
	{
		if (m_callStack.empty())
			return false;

		// run은 call stack 상태가 run 이거나 wait 일때만 실행된다.
		EnvCallStackInfo& inCallStackInfo = m_callStack.back();
		if (EEnvCallStackState::Run != inCallStackInfo.eState && EEnvCallStackState::Wait != inCallStackInfo.eState)
			return false;

		const BaseCPtr& inBase = inCallStackInfo.spBase;
		if (!inBase)
			return false;

		const EASTType eASTType = inBase->GetType();
		switch (eASTType)
		{
			case EASTType::Base:
			{
				return false;
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
					return false;

				EnvCallStackInfo& callStackInfo = insertCallStack(spAST->block);
				const EEnvCallStackState eState = runCallStack(callStackInfo);

				if (EEnvCallStackState::Success == eState)
					m_callStack.pop_back();

				return eState;
			}
			break;

			case EASTType::Block:
			{
				const BlockCPtr& spBlock = static_pointer_cast<const Block>(inBase);

				int& refLoopCount = inCallStackInfo.nLoopCount;
				for (; refLoopCount < spBlock->statements.size(); ++refLoopCount)
				{
					const BaseCPtr& spBase = spBlock->statements[refLoopCount];
					if (!spBase)
						return false;

					EnvCallStackInfo& callStackInfo = insertCallStack(spBase);
					const EEnvCallStackState eState = runCallStack(callStackInfo);

					if (EEnvCallStackState::Success == eState)
						m_callStack.pop_back();
					
					return eState;
				}
			}
			break;

			case EASTType::Assignment:
			{
				const AssignmentCPtr& spAssignment = static_pointer_cast<const Assignment>(inBase);

				const NameCPtr& spName = static_pointer_cast<const Name>(inBase);
				const ExpressionCPtr& spExpression = static_pointer_cast<const Expression>(inBase);
				if (!spName || !spExpression)
					return false;

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
					return false;

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
					return false;

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
					return false;

				EnvCallStackInfo& callStackInfo = insertCallStack(spStatement->statement);
				const EEnvCallStackState eState = runCallStack(callStackInfo);

				if (EEnvCallStackState::Success == eState)
					m_callStack.pop_back();

				return eState;
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


		return false;
	}


	// 단항 연산자를 실행한다.
	// @upEnvValBase		: 피연산자
	// @strOperator			: 연산자
	// @return				: 결과값
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

	// 이항 연산자를 실행한다.
	// @upLeftEnvValBase	: 왼쪽 피연산자
	// @strOperator			: 연산자
	// @upRightEnvValBase	: 오른쪽 피연산자
	// @return				: 결과값
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