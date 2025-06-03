//#pragma once
//
//class CDSLScript
//{
//public:
//	using Args = std::vector<std::string>;
//	using FunctionMap = std::unordered_map<std::string, std::function<void(const Args&)>>;
//
//public:
//	CDSLScript();
//
//	bool Initialize(std::wstring strFileName);
//
//	void RegisterFunction(const std::string& name, std::function<void(const Args&)> fn);
//
//private:
//	std::wstring m_strFileName;
//
//	FunctionMap m_functionTable;
//	std::unordered_map<std::string, std::vector<std::string>> m_userFunctions;
//};
//
