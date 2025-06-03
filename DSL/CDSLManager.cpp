//#include <iostream>
//#include <unordered_map>
//#include <vector>
//#include <functional>
//#include <string>
//#include <sstream>
//#include <stack>
//#include <thread>
//#include <chrono>
//#include <mutex>
//#include <map>
//
//#include "CDSL.h"
//#include "CDSLScript.h"
//#include "CDSLManager.h"
//
//
//CDSLManager::CDSLManager()
//{
//
//}
//
//CDSLManager* CDSLManager::GetInstance()
//{
//	if (nullptr != m_pCDSLManager)
//	{
//		return m_pCDSLManager;
//	}
//	else
//	{
//		std::lock_guard<std::mutex> lockGuard(m_lock);
//		if (nullptr == m_pCDSLManager)
//		{
//			m_pCDSLManager = new CDSLManager();
//		}
//
//		return m_pCDSLManager;
//	}
//}
//
//bool Initialize()
//{
//
//}
//
//
//// DSL 스크립트 파일 하나를 로드해서 CDSLScript 객체를 만든다.
//void CDSLManager::LoadScript(std::wstring strFileName)
//{
//	std::lock_guard<std::mutex> lockGuard(m_lock);
//
//	if (m_dslScriptMap.end() != m_dslScriptMap.find(strFileName))
//		return;
//
//	std::shared_ptr<CDSLScript> spDSLScript = std::make_shared<CDSLScript>();
//	if (!spDSLScript)
//		return;
//
//	if (false == spDSLScript->Initialize(strFileName))
//		return;
//
//	m_dslScriptMap.insert(std::make_pair(strFileName, spDSLScript));
//
//}
//
//void CDSLManager::Insert(std::shared_ptr<CDSL> spDSL)
//{
//	if (!spDSL)
//		return;
//
//	std::lock_guard<std::mutex> lockGuard(m_lock);
//	m_dslMap.insert(std::make_pair(spDSL->GetID(), spDSL));
//}
//
//void CDSLManager::Run()
//{
//	std::lock_guard<std::mutex> lockGuard(m_lock);
//
//	for (const auto& [nId, spDSL] : m_dslMap)
//	{
//		if (!spDSL)
//			continue;
//
//		spDSL->Run();
//	}
//	
//}