#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <any>

namespace dsl
{

	class DSLManager
	{
	public:
		using GenericFunc = std::function<std::any(const std::vector<std::any>&)>;

		DSLManager();

		static DSLManager* GetInstance();

	public:
		bool Initialize();

		bool AddAPIFunction(const std::wstring& strFuncName, GenericFunc funcAPI);
		const GenericFunc GetAPIFunction(const std::wstring& strFuncName);
		std::any CallAPIFunction(const std::wstring& strFuncName, const std::vector<std::any>& args);

		bool LoadScript(const std::wstring& strFileName);

		ASTPtr MakeAST(std::wstring& strScript);
		ASTPtr GetAST(const std::wstring& strFileName);

		ExecutorPtr MakeEnvironment();


	private:
		// singleton
		static DSLManager* m_pInstance;
		static std::mutex m_lock;

		// API 함수 관리 map
		std::unordered_map<std::wstring, GenericFunc> m_apiFunctionMap;
		std::shared_mutex m_slockApiFunctionMap;
		

		// AST 관리 map
		std::unordered_map<std::wstring, ASTPtr> m_ASTMap;

	};


}