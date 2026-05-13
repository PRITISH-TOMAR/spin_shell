#include "echo.hpp"
#include <iostream>

void handleEcho(const vector<string>& args) {
    for (const string& arg : args) {
        cout << arg << " ";
    }
    cout << "\n";
}