#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <stack>
#include <thread>
#include <chrono>

#include "ast.h"

#include "Executor.h"


namespace dsl
{

	Executor::Executor()
	{

	}

	bool Executor::Initialize(const ASTPtr spAST)
	{
		if (!spAST)
			return false;


	}

}