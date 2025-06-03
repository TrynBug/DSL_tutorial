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
//#include "parser.hpp"
//#include "CDSLScript.h"
//
//CDSLScript::CDSLScript()
//{
//
//}
//
//bool CDSLScript::Initialize(std::wstring strFileName)
//{
//    m_strFileName = strFileName;
//
//	std::ifstream in("strFileName");
//	std::string input((std::istreambuf_iterator<char>(in)),
//		std::istreambuf_iterator<char>());
//
//	using boost::spirit::x3::ascii::space;
//	auto iter = input.begin(), end = input.end();
//
//	std::vector<dsl::Statement> ast;
//	bool success = phrase_parse(iter, end, dsl::parser::program, space, ast);
//
//	if (success && iter == end) {
//		std::cout << "✅ Parsed DSL successfully!\n";
//		std::cout << "Statement count: " << ast.size() << "\n";
//	}
//	else {
//		std::cerr << "❌ Failed to parse DSL!\n";
//	}
//
//	return 0;
//}
//
//
//void CDSLScript::RegisterFunction(const std::string& name, std::function<void(const Args&)> fn)
//{
//    m_functionTable[name] = fn;
//}
