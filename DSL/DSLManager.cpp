
#include "ast.h"
#include "parser.h"
#include "Environment.h"

#include "DSLManager.h"

namespace dsl
{

DSLManager* DSLManager::sm_pInstance;
std::mutex DSLManager::sm_lock;

DSLManager::DSLManager()
{

}

DSLManager::~DSLManager()
{

}

DSLManager* DSLManager::GetInstance()
{
	if (nullptr != sm_pInstance)
	{
		return sm_pInstance;
	}
	else
	{
		std::lock_guard<std::mutex> lockGuard(sm_lock);
		if (nullptr == sm_pInstance)
		{
			sm_pInstance = new DSLManager();
		}

		return sm_pInstance;
	}
}

bool DSLManager::Initialize()
{

	return true;
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
	std::unique_lock lock(m_slock);
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
	std::shared_lock lock(m_slock);

	auto iter = m_ASTMap.find(strFileName);
	if (iter == m_ASTMap.end())
		return nullptr;

	return iter->second;
}

// Environment 생성
EnvironmentPtr DSLManager::MakeEnvironment()
{
	return nullptr;
}



// 함수 제거
void DSLManager::RemoveFunction(const std::wstring& name)
{
	std::unique_lock lock(m_slock);
	m_funcMap.erase(name);
}

// 함수 가져오기
DSLManager::FunctionType DSLManager::GetFunction(const std::wstring& name)
{
	std::shared_lock lock(m_slock);

	auto iter = m_funcMap.find(name);
	if (iter != m_funcMap.end())
		return iter->second;

	return nullptr;
}

// 함수 실행 (인자값 전달)
std::any DSLManager::ExecuteFunction(const std::wstring& name, const std::vector<std::any>& args)
{
	std::shared_lock lock(m_slock);

	auto it = m_funcMap.find(name);
	if (it != m_funcMap.end())
	{
		return it->second(args);
	}
	else
	{
		std::wcout << L"Function '" + name + L"' not found" << std::endl;
	}

	return 0;
}

// 함수 존재 여부 확인
bool DSLManager::HasFunction(const std::wstring& name) const
{
	std::shared_lock lock(m_slock);

	return m_funcMap.contains(name);
}


// 함수 제거
void DSLManager::RemoveApiFunction(const std::wstring& name)
{
	std::unique_lock lock(m_slock);
	m_apiFuncMap.erase(name);
}

// 함수 가져오기
DSLManager::FunctionType DSLManager::GetApiFunction(const std::wstring& name)
{
	std::shared_lock lock(m_slock);

	auto iter = m_apiFuncMap.find(name);
	if (iter != m_apiFuncMap.end())
		return iter->second;

	return nullptr;
}

// 함수 실행 (인자값 전달)
std::any DSLManager::ExecuteApiFunction(const std::wstring& name, const std::vector<std::any>& args)
{
	std::shared_lock lock(m_slock);

	auto it = m_apiFuncMap.find(name);
	if (it != m_apiFuncMap.end())
	{
		return it->second(args);
	}
	else
	{
		std::wcout << L"API Function '" + name + L"' not found" << std::endl;
	}

	return 0;
}

// 함수 존재 여부 확인
bool DSLManager::HasApiFunction(const std::wstring& name) const
{
	std::shared_lock lock(m_slock);

	return m_apiFuncMap.contains(name);
}

}