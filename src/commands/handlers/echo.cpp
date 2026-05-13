#include "echo.hpp"
#include <iostream>

void handleEcho(const string command) {
        cout << command.substr(5) << "\n";
    cout << "\n";
}