#include <iostream>
#include <string>

using namespace std;

int main() {
    // Flush the output buffer after each output operation
    cout << unitbuf;
    cerr << unitbuf;

    string input;
    
    // The REPL (Read-Eval-Print Loop)
    while (true) {
        cout << "$ ";
        
        // 1. Read the input
        if (!getline(cin, input)) {
            break; // Exit if EOF (e.g., Ctrl+D) is reached
        }

        // 2. Handle the "exit 0" command
        if (input == "exit 0") {
            return 0;
        }

        // 3. Handle unknown commands
        if (!input.empty()) {
            cout << input << ": command not found\n";
        }
    }
}