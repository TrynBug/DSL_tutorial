#pragma once

#include <iostream>
#include <format>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <any>
#include <memory>
#include <fstream>
#include <tuple>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <type_traits>
#include <typeindex>
#include <stdexcept>
#include <exception>

#include "Utils.h"

namespace dsl
{

	class DSLManager
	{
	public:
		// 함수 파라미터 타입
		using ParameterTypes = std::vector<std::type_index>;

		// 실제 함수 타입 (파라미터 타입으로부터 함수 시그니처를 생성한다.)
		using FunctionType = std::function<std::any(const std::vector<std::any>&)>;

	public:
		DSLManager();
		~DSLManager();

		static DSLManager* GetInstance();

	public:
		bool Initialize();

	public:
		size_t GetASTFunctionCount() const;
		size_t GetASTFunctionCount(const std::wstring& strScriptName) const;
		size_t GetApiFunctionCount() const;

		ASTPtr GetAST(const std::wstring& scriptName);

		/* AST */
		bool LoadScript(const std::wstring& scriptName);

		EnvironmentPtr MakeEnvironment();

		/* AST Function */
		void AddASTFunction(const std::wstring& scriptName, const ASTCPtr spAST);
		void AddASTFunction(const std::wstring& scriptName, const FunctionDefinitionCPtr spFunctionDefinition);
		void RemoveASTFunction(const std::wstring& scriptName, const std::wstring& funcName);
		const FunctionDefinitionCPtr& GetASTFunction(const std::wstring& scriptName, const std::wstring& funcName);
		bool HasASTFunction(const std::wstring& scriptName, const std::wstring& funcName) const;

		/* API Function */
		template<typename... Args>
		void AddApiFunction(const std::wstring& name, std::function<std::any(Args...)> func);

		void RemoveApiFunction(const std::wstring& name);
		FunctionType GetApiFunction(const std::wstring& name);
		std::any ExecuteApiFunction(const std::wstring& name, const std::vector<std::any>& args = {});
		bool HasApiFunction(const std::wstring& name) const;

	private:
		void initializeApiFunctionMap();
		ASTPtr makeAST(std::wstring& strScript);

	private:

		// 전달받은 인자(args)들을 함수 시그니처에 맞는 타입으로 변환시킨다음, 함수(func)에 인자를 전달하여 호출하는 헬퍼함수
		template<typename... Args, size_t... I>
		static std::any callFunction(std::function<std::any(Args...)> func, const std::vector<std::any>& args, std::index_sequence<I...>);

		// 인자(arg)를 올바른 타입(T)으로 변환하는 헬퍼함수.
		template<typename T>
		static T extractArg(const std::any& arg);

		// 타입 이름을 읽기쉬운 형태로 변환하는 헬퍼함수
		template<typename T>
		static std::string getTypeName();

		// 타입 이름을 읽기쉬운 형태로 변환하는 헬퍼함수
		static inline std::string getTypeName(const std::type_info& type);

		// 템플릿 특수화를 사용한 타입 이름 변환
		template<typename T>
		static std::string getReadableTypeName();

		// 템플릿 특수화를 사용한 타입 이름 변환
		static inline std::string getReadableTypeName(const std::type_info& type);


	private:
		// singleton
		static DSLManager* sm_pInstance;
		static std::mutex sm_lock;

		// lock
		mutable std::shared_mutex m_slock;

		// AST map
		// Key=script 파일명, Value=AST
		std::unordered_map<std::wstring, ASTPtr> m_ASTMap;

		// 사용자 함수 map
		// 사용자 함수는 스크립트 내에서 사용할 수 있는 함수이다.
		// Key=script 파일명, Value=<Key=함수명, Value=Function AST객체>
		std::unordered_map<std::wstring, std::unordered_map<std::wstring, FunctionDefinitionCPtr>> m_ASTFuncMap;

		// API 함수 map
		// API 함수는 모든 스크립트에서 공용으로 사용할 수 있는 함수이다.
		// Key=함수명, Value=함수
		std::unordered_map<std::wstring, FunctionType> m_apiFuncMap;
	};


	// API 함수 등록 (템플릿 타입은 파라미터 타입만 나열하면됨. 리턴타입은 std::any 고정)
	template<typename... Args>
	void DSLManager::AddApiFunction(const std::wstring& funcName, std::function<std::any(Args...)> func)
	{
		// 함수를 래핑하여 std::vector<std::any>를 받는 형태로 변환
		auto wrapper = [this, funcName, func](const std::vector<std::any>& args) -> std::any
			{
				if (sizeof...(Args) != args.size()) // 함수 파라미터 개수와 전달된 인자 개수가 일치하는지 체크
				{
					throw wexception(std::format(L"Argument count mismatch. funcName={}, sizeof(Args)={}, args.size={}", funcName, sizeof...(Args), args.size()));
				}

				// 인자들을 올바른 타입으로 변환하여 함수 호출
				return callFunction<Args...>(func, args, std::make_index_sequence<sizeof...(Args)>{});
			};

		std::unique_lock lock(m_slock);

		if (m_apiFuncMap.contains(funcName))
			std::wcout << std::format(L"function already exists. funcName={}", funcName) << std::endl;

		m_apiFuncMap[funcName] = wrapper;
	}


	// 전달받은 인자(args)들을 함수 시그니처에 맞는 타입으로 변환시킨다음, 함수(func)에 인자를 전달하여 호출하는 헬퍼함수
	template<typename... Args, size_t... I>
	std::any DSLManager::callFunction(std::function<std::any(Args...)> func, const std::vector<std::any>& args, std::index_sequence<I...>)
	{
		// 예: func=Add(int, int) 이고, args={10, 20} 이라면 이 코드는 return Add(10, 20); 이 된다.
		return func(extractArg<Args>(args[I])...);
	}

	// 인자(arg)를 올바른 타입(T)으로 변환하는 헬퍼함수.
	template<typename T>
	T DSLManager::extractArg(const std::any& arg) 
	{
		try 
		{
			return std::any_cast<T>(arg);
		}
        catch (const std::bad_any_cast&) 
		{
            std::string error_msg = "Type mismatch in function argument. Expected: ";
            error_msg += getTypeName<T>();
            error_msg += ", Got: ";
            error_msg += getTypeName(arg.type());
            throw std::runtime_error(error_msg);
        }
	}

	// 타입 이름을 읽기쉬운 형태로 변환하는 헬퍼함수
	template<typename T>
	std::string DSLManager::getTypeName()
	{
		return getReadableTypeName<T>();
	}

	// 타입 이름을 읽기쉬운 형태로 변환하는 헬퍼함수
	inline std::string DSLManager::getTypeName(const std::type_info& type)
	{
		return getReadableTypeName(type);
	}

	// 템플릿 특수화를 사용한 타입 이름 변환
	template<typename T>
	std::string DSLManager::getReadableTypeName()
	{
		if constexpr (std::is_same_v<T, int>)				return "int";
		else if constexpr (std::is_same_v<T, double>)		return "double";
		else if constexpr (std::is_same_v<T, bool>)			return "bool";
		else if constexpr (std::is_same_v<T, std::wstring>) return "std::wstring";
		else if constexpr (std::is_same_v<T, std::string>)	return "std::string";
		else if constexpr (std::is_same_v<T, float>)		return "float";
		else if constexpr (std::is_same_v<T, char>)			return "char";
		else if constexpr (std::is_same_v<T, wchar_t>)		return "wchar_t";
		else												return typeid(T).name(); // 기본값
	}

	// 템플릿 특수화를 사용한 타입 이름 변환
	inline std::string DSLManager::getReadableTypeName(const std::type_info& type)
	{
		if (type == typeid(int))			return "int";
		if (type == typeid(double))			return "double";
		if (type == typeid(bool))			return "bool";
		if (type == typeid(std::wstring))	return "std::wstring";
		if (type == typeid(std::string))	return "std::string";
		if (type == typeid(float))			return "float";
		if (type == typeid(char))			return "char";
		if (type == typeid(wchar_t))		return "wchar_t";

		return	type.name(); // 기본값
	}
}