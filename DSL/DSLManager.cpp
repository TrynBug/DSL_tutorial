

#include <thread>
#include <chrono>
#include <fstream>

#include "ast.h"
#include "parser.h"
#include "Executor.h"

#include "DSLManager.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;
using std::placeholders::_6;
using std::placeholders::_7;
using std::placeholders::_8;
using std::placeholders::_9;

namespace dsl
{

DSLManager* DSLManager::m_pInstance;
std::mutex DSLManager::m_lock;

DSLManager::DSLManager()
{

}

DSLManager* DSLManager::GetInstance()
{
	if (nullptr != m_pInstance)
	{
		return m_pInstance;
	}
	else
	{
		std::lock_guard<std::mutex> lockGuard(m_lock);
		if (nullptr == m_pInstance)
		{
			m_pInstance = new DSLManager();
		}

		return m_pInstance;
	}
}

bool DSLManager::Initialize()
{



	return true;
}

// API 함수를 등록한다.
// API 함수는 모든 스크립트에서 사용가능한 함수이다.
bool DSLManager::AddAPIFunction(const std::wstring& strFuncName, GenericFunc funcAPI)
{
	std::unique_lock lock(m_slockApiFunctionMap);
	m_apiFunctionMap[strFuncName] = funcAPI;
	return true;
}


// 함수명으로 API 함수를 찾는다.
const DSLManager::GenericFunc DSLManager::GetAPIFunction(const std::wstring& strFuncName)
{
	std::shared_lock lock(m_slockApiFunctionMap);

	const auto iter = m_apiFunctionMap.find(strFuncName);
	if (iter == m_apiFunctionMap.end())
		return nullptr;

	return iter->second;
}

std::any DSLManager::CallAPIFunction(const std::wstring& strFuncName, const std::vector<std::any>& args)
{
	std::shared_lock lock(m_slockApiFunctionMap);

	const auto iter = m_apiFunctionMap.find(strFuncName);
	if (iter == m_apiFunctionMap.end())
	{
		throw std::runtime_error("Function not found");
	}

	const GenericFunc& func = iter->second;
	return func(args);
}

// 스크립트 파일 하나를 로드해서 AST를 만든다.
bool DSLManager::LoadScript(const std::wstring& strFileName)
{
	std::wifstream scriptFile(strFileName, std::ios::in | std::ios::binary);
	if (!scriptFile)
	{
		std::wcout << std::format(L"스크립트 파일 열기 실패. fileName = {}", strFileName) << std::endl;
		return false;
	}

	// 로케일 설정 (시스템 기본 로케일)
	scriptFile.imbue(std::locale(""));

	std::wstring strScript((std::istreambuf_iterator<wchar_t>(scriptFile)), std::istreambuf_iterator<wchar_t>());

	std::wcout << std::format(L"FileName = {}, Content = {}", strFileName, strScript) << std::endl;

	// 스크립트로 AST 생성
	ASTPtr spAST = MakeAST(strScript);
	if (!spAST)
	{
		std::wcout << std::format(L"AST 생성 실패. FileName = {}", strFileName) << std::endl;
		return false;
	}

	// insert
	std::lock_guard<std::mutex> lockGuard(m_lock);
	m_ASTMap[strFileName] = spAST;

	return true;
}

// 스크립트를 파싱해서 AST를 만든다.
ASTPtr DSLManager::MakeAST(std::wstring& strScript)
{
	return ParseScript(strScript);
}

// AST 얻기
ASTPtr DSLManager::GetAST(const std::wstring& strFileName)
{
	std::lock_guard<std::mutex> lockGuard(m_lock);

	auto iter = m_ASTMap.find(strFileName);
	if (iter == m_ASTMap.end())
		return nullptr;

	return iter->second;
}

// Environment 생성
ExecutorPtr DSLManager::MakeEnvironment()
{
	return nullptr;
}

}