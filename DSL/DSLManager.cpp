#include "pch.h"

#include "ast.h"
#include "parser.h"
#include "EnvironmentDefine.h"
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
	// API 함수 등록
	initializeApiFunctionMap();

	return true;
}

// AST 함수 count
size_t DSLManager::GetASTFunctionCount() const 
{ 
	std::shared_lock lock(m_slock);

	size_t size = 0;
	for (const auto& [strScript, funcDefinitionMap] : m_ASTFuncMap)
	{
		size += funcDefinitionMap.size();
	}
	
	return size;
}

// AST 함수 count
size_t DSLManager::GetASTFunctionCount(const std::wstring& strScriptName) const
{
	std::shared_lock lock(m_slock);

	const auto iter = m_ASTFuncMap.find(strScriptName);
	if (iter == m_ASTFuncMap.end())
		return 0;

	const std::unordered_map<std::wstring, FunctionDefinitionCPtr>& funcDefinitionMap = iter->second;

	return funcDefinitionMap.size();
}

// API 함수 count
size_t DSLManager::GetApiFunctionCount() const 
{ 
	return m_apiFuncMap.size(); 
}

// AST 얻기
ASTPtr DSLManager::GetAST(const std::wstring& scriptName)
{
	std::shared_lock lock(m_slock);

	auto iter = m_ASTMap.find(scriptName);
	if (iter == m_ASTMap.end())
		return nullptr;

	return iter->second;
}

// 스크립트 파일 하나를 로드해서 AST를 만든다.
bool DSLManager::LoadScript(const std::wstring& scriptName)
{
	std::wifstream scriptFile(scriptName, std::ios::in | std::ios::binary);
	if (!scriptFile)
	{
		std::wcout << std::format(L"스크립트 파일 열기 실패. fileName = {}", scriptName) << std::endl;
		return false;
	}

	// 로케일 설정 (시스템 기본 로케일)
	scriptFile.imbue(std::locale(""));

	std::wstring strScript((std::istreambuf_iterator<wchar_t>(scriptFile)), std::istreambuf_iterator<wchar_t>());

	std::wcout << std::format(L"FileName = {}, Content = {}", scriptName, strScript) << std::endl;

	// 스크립트로 AST 생성
	ASTPtr spAST = makeAST(strScript);
	if (!spAST)
	{
		std::wcout << std::format(L"AST 생성 실패. FileName = {}", scriptName) << std::endl;
		return false;
	}

	std::unique_lock lock(m_slock);

	// AST를 순회하며 사용자 함수를 등록한다.
	AddASTFunction(scriptName, spAST);

	m_ASTMap[scriptName] = spAST;

	return true;
}


// Environment 생성
EnvironmentPtr DSLManager::MakeEnvironment()
{
	return nullptr;
}

// AST 함수 등록. AST를 순회하면서 모든 함수를 등록한다.
void DSLManager::AddASTFunction(const std::wstring& scriptName, const ASTCPtr spAST)
{
	if (!spAST)
		return;

	std::unique_lock lock(m_slock);

	auto astFuncFinder = [this, &scriptName](const BaseCPtr spBase)
	{
		if (!spBase)
			return;

		if (EASTType::FunctionDefinition != spBase->GetType())
			return;

		const FunctionDefinitionCPtr& spFunctionDefinition = static_pointer_cast<const FunctionDefinition>(spBase);

		// AST 함수 등록
		AddASTFunction(scriptName, spFunctionDefinition);
	};

	// 순회
	spAST->Iterate(astFuncFinder);
}

// AST 함수 등록. 인자로 받은 함수 1개만 등록한다.
void DSLManager::AddASTFunction(const std::wstring& scriptName, const FunctionDefinitionCPtr spFunctionDefinition)
{
	if (!spFunctionDefinition)
		return;

	std::unique_lock lock(m_slock);

	std::unordered_map<std::wstring, FunctionDefinitionCPtr>& funcDefinitionMap = m_ASTFuncMap[scriptName];

	const NameCPtr spName = static_pointer_cast<const Name>(spFunctionDefinition->name);

	auto iter = funcDefinitionMap.find(spName->name);
	if (iter != funcDefinitionMap.end())
		std::wcout << std::format(L"AST function Already Exists. scriptName={}, funcName={}", scriptName, spName->name) << std::endl;

	funcDefinitionMap[spName->name] = spFunctionDefinition;
}

// AST 함수 제거
void DSLManager::RemoveASTFunction(const std::wstring& scriptName, const std::wstring& funcName)
{
	std::unique_lock lock(m_slock);
	
	auto iter = m_ASTFuncMap.find(scriptName);
	if (iter == m_ASTFuncMap.end())
		return;

	std::unordered_map<std::wstring, FunctionDefinitionCPtr>& funcDefinitionMap = iter->second;

	funcDefinitionMap.erase(funcName);
}

// AST 함수 가져오기
const FunctionDefinitionCPtr& DSLManager::GetASTFunction(const std::wstring& scriptName, const std::wstring& funcName)
{
	static const FunctionDefinitionCPtr empty = nullptr;

	std::shared_lock lock(m_slock);

	auto iter = m_ASTFuncMap.find(scriptName);
	if (iter == m_ASTFuncMap.end())
		return empty;

	const std::unordered_map<std::wstring, FunctionDefinitionCPtr>& funcDefinitionMap = iter->second;

	auto iter2 = funcDefinitionMap.find(funcName);
	if (iter2 == funcDefinitionMap.end())
		return empty;
		
	return iter2->second;
}

// AST 함수 존재 여부 확인
bool DSLManager::HasASTFunction(const std::wstring& scriptName, const std::wstring& funcName) const
{
	std::shared_lock lock(m_slock);

	auto iter = m_ASTFuncMap.find(scriptName);
	if (iter == m_ASTFuncMap.end())
		return false;

	const std::unordered_map<std::wstring, FunctionDefinitionCPtr>& funcDefinitionMap = iter->second;

	return funcDefinitionMap.contains(funcName);
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

// API 함수 map 초기화
void DSLManager::initializeApiFunctionMap()
{
	std::unique_lock lock(m_slock);

	m_apiFuncMap.clear();

	AddApiFunction<const std::wstring&>(
		L"Print",
		std::function<std::any(const std::wstring&)>(
			[](const std::wstring& str) 
			{ 
				std::wcout << str << std::endl; 
				return 0; 
			}
		)
	);

	AddApiFunction<const int>(
		L"Sleep",
		std::function<std::any(const int)>(
			[](const int ms)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(ms));
				return 0;
			}
		)
	);
}

// 스크립트를 파싱해서 AST를 만든다.
ASTPtr DSLManager::makeAST(std::wstring& strScript)
{
	return ParseScript(strScript);
}

}