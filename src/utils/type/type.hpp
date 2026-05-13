#pragma once
#include <string>

using namespace std;


struct TypeResult{
    enum class Kind{
        BUILTIN,
        EXTERNAL,
        NOT_FOUND
    };

    Kind kind;
    string path;
};

TypeResult resolveType(const string& typeName);