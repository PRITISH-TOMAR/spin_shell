#pragma once
#include<vector>
#include<string>

using namespace std;

// Split shell raw input on pipe character
// pipe inside quotes is not valid
vector<string> splitOnPipe(const string& input);

bool containsPipe(const string& input);