#include <windows.h>
#include <iostream>
#include <mutex>


#include "ast.h"
#include "parser.h"
#include "Environment.h"
#include "DSLManager.h"


int wmain(int argc, wchar_t* argv[])
{


    dsl::Parser2Test();
    return 0;
}