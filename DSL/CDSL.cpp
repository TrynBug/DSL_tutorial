//#include "CDSL.h"
//
//
//CDSL::CDSL()
//{
//
//}
//
//
//
//std::vector<std::string> CDSL::tokenize(const std::string& line) {
//    std::istringstream iss(line);
//    std::string token;
//    std::vector<std::string> tokens;
//    while (iss >> token) tokens.push_back(token);
//    return tokens;
//}
//
//bool CDSL::evaluateCondition(const std::string& var, const std::string& op, const std::string& val) {
//    int lhs = variables[var];
//    int rhs = std::stoi(val);
//    if (op == "==") return lhs == rhs;
//    if (op == "!=") return lhs != rhs;
//    if (op == "<") return lhs < rhs;
//    if (op == "<=") return lhs <= rhs;
//    if (op == ">") return lhs > rhs;
//    if (op == ">=") return lhs >= rhs;
//    return false;
//}
//
//void CDSL::executeScript(const std::vector<std::string>& lines) {
//    bool executing = true;
//    std::stack<bool> execStack;
//    std::vector<std::string> currentFunction;
//    std::string functionName;
//    bool insideFunc = false;
//
//    for (size_t i = 0; i < lines.size(); ++i) {
//        std::vector<std::string> tokens = tokenize(lines[i]);
//        if (tokens.empty()) continue;
//
//        std::string cmd = tokens[0];
//
//        if (cmd == "FUNC") {
//            insideFunc = true;
//            functionName = tokens[1];
//            currentFunction.clear();
//
//        }
//        else if (cmd == "END_FUNC") {
//            insideFunc = false;
//            userFunctions[functionName] = currentFunction;
//
//        }
//        else if (insideFunc) {
//            currentFunction.push_back(lines[i]);
//
//        }
//        else if (cmd == "SET" && executing) {
//            variables[tokens[1]] = std::stoi(tokens[2]);
//
//        }
//        else if (cmd == "IF") {
//            bool cond = evaluateCondition(tokens[1], tokens[2], tokens[3]);
//            execStack.push(executing);
//            executing = executing && cond;
//
//        }
//        else if (cmd == "END_IF") {
//            if (!execStack.empty()) {
//                executing = execStack.top();
//                execStack.pop();
//            }
//
//        }
//        else if (cmd == "CALL" && executing) {
//            std::string funcName = tokens[1];
//            Args args(tokens.begin() + 2, tokens.end());
//
//            // 사용자 정의 함수 호출
//            if (userFunctions.find(funcName) != userFunctions.end()) {
//                executeScript(userFunctions[funcName]);
//            }
//            else if (functionTable.find(funcName) != functionTable.end()) {
//                functionTable[funcName](args);
//            }
//            else {
//                std::cerr << "[Error] Unknown function: " << funcName << std::endl;
//            }
//        }
//    }
//}