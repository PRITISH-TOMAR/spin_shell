#include "pwd.hpp"
#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = std ::filesystem;

void handlePwd()
{
    cout << fs::current_path().string() << "\n";
}