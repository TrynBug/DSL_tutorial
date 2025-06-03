//#pragma once
//
//// DSL �Ŵ����� 
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
//	// DSL ��ũ��Ʈ ���� map
//	std::unordered_map<std::wstring, std::shared_ptr<CDSLScript>> m_dslScriptMap;
//
//	// DSL �ν��Ͻ� ���� map
//	std::unordered_map<__int64, std::shared_ptr<CDSL>> m_dslMap;
//
//};
//
