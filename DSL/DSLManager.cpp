
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

size_t DSLManager::GetASTFunctionCount() const 
{ 
	std::shared_lock lock(m_slock);

	size_t size = 0;
	for (const auto& [strScript, spFuncMap] : m_ASTFuncMap)
	{
		if (!spFuncMap)
			continue;

		size += spFuncMap->size();
	}
	
	return size;
}

size_t DSLManager::GetASTFunctionCount(const std::wstring& strScriptName) const
{
	std::shared_lock lock(m_slock);

	auto iter = m_ASTFuncMap.find(strScriptName);
	if (iter == m_ASTFuncMap.end())
		return 0;

	const FunctionMapCPtr& spFuncMap = iter->second;
	if (!spFuncMap)
		return 0;

	return spFuncMap->size();
}

size_t DSLManager::GetApiFunctionCount() const 
{ 
	return m_apiFuncMap.size(); 
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
	ASTPtr spAST = makeAST(strScript);
	if (!spAST)
	{
		std::wcout << std::format(L"AST 생성 실패. FileName = {}", strFileName) << std::endl;
		return false;
	}

	FunctionMapPtr spFunctionMap = makeFunctionMap(spAST);
	if (!spFunctionMap)
	{
		std::wcout << std::format(L"AST FunctionMap 생성 실패. FileName = {}", strFileName) << std::endl;
		return false;
	}

	// insert
	std::unique_lock lock(m_slock);
	m_ASTMap[strFileName] = spAST;
	m_ASTFuncMap[strFileName] = spFunctionMap;

	return true;
}


// Environment 생성
EnvironmentPtr DSLManager::MakeEnvironment()
{
	return nullptr;
}



// AST 함수 제거
void DSLManager::RemoveASTFunction(const std::wstring& scriptName, const std::wstring& funcName)
{
	std::unique_lock lock(m_slock);
	
	auto iter = m_ASTFuncMap.find(scriptName);
	if (iter == m_ASTFuncMap.end())
		return;

	const FunctionMapPtr spFunctionMap = iter->second;
	if (!spFunctionMap)
		return;

	spFunctionMap->erase(funcName);
}

// AST 함수 가져오기
const DSLManager::FunctionType& DSLManager::GetASTFunction(const std::wstring& scriptName, const std::wstring& funcName)
{
	static const FunctionType empty;

	std::shared_lock lock(m_slock);

	auto iter = m_ASTFuncMap.find(scriptName);
	if (iter == m_ASTFuncMap.end())
		return empty;

	const FunctionMapCPtr spFunctionMap = iter->second;
	if (!spFunctionMap)
		return empty;

	auto iter2 = spFunctionMap->find(funcName);
	if (iter2 == spFunctionMap->end())
		return empty;
		
	return iter2->second;
}

// AST 함수 실행 (인자값 전달)
std::any DSLManager::ExecuteASTFunction(const std::wstring& scriptName, const std::wstring& funcName, const std::vector<std::any>& args)
{
	std::shared_lock lock(m_slock);

	auto iter = m_ASTFuncMap.find(scriptName);
	if (iter == m_ASTFuncMap.end())
		return nullptr;

	const FunctionMapCPtr spFunctionMap = iter->second;
	if (!spFunctionMap)
		return nullptr;

	auto iter2 = spFunctionMap->find(funcName);
	if (iter2 == spFunctionMap->end())
		return nullptr;

	const FunctionType& func = iter2->second;
	return func(args);

	return nullptr;
}

// AST 함수 존재 여부 확인
bool DSLManager::HasASTFunction(const std::wstring& scriptName, const std::wstring& funcName) const
{
	std::shared_lock lock(m_slock);

	auto iter = m_ASTFuncMap.find(scriptName);
	if (iter == m_ASTFuncMap.end())
		return false;

	const FunctionMapCPtr spFunctionMap = iter->second;
	if (!spFunctionMap)
		return false;

	return spFunctionMap->contains(funcName);
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


// 스크립트를 파싱해서 AST를 만든다.
ASTPtr DSLManager::makeAST(std::wstring& strScript)
{
	return ParseScript(strScript);
}

// AST를 순회하며 FunctionMap을 만든다.
DSLManager::FunctionMapPtr DSLManager::makeFunctionMap(const ASTCPtr spAST)
{
	if (!spAST)
		return nullptr;

	FunctionMapPtr spFunctionMap = std::make_shared<FunctionMap>();

	auto funcFactory = [&spFunctionMap](const BaseCPtr spBase)
		{
			if (!spBase)
				return;

			if (EASTType::FunctionDefinition != spBase->GetType())
				return;

			const FunctionDefinitionCPtr& spFunctionDefinition = static_pointer_cast<const FunctionDefinition>(spBase);

			spFunctionDefinition->name;
			// name 에 따라 매칭되는 함수를 찾아서 m_ASTFuncMap 에 등록해야 한다.
		};

	spAST->Iterate(funcFactory);

	return spFunctionMap;
}

}