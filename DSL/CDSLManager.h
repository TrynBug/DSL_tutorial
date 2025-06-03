//#pragma once
//
//// DSL 매니저는 
//class CDSLManager
//{
//public:
//	CDSLManager();
//
//public:
//	static CDSLManager* GetInstance();
//
//public:
//	bool Initialize();
//
//	void LoadScript(std::wstring strFileName);
//
//	void Insert(std::shared_ptr<CDSL> spDSL);
//
//	void Run();
//
//
//private:
//	// singleton
//	static CDSLManager* m_pCDSLManager;
//	static std::mutex m_lock;
//
//private:
//	// DSL 스크립트 관리 map
//	std::unordered_map<std::wstring, std::shared_ptr<CDSLScript>> m_dslScriptMap;
//
//	// DSL 인스턴스 관리 map
//	std::unordered_map<__int64, std::shared_ptr<CDSL>> m_dslMap;
//
//};
//
