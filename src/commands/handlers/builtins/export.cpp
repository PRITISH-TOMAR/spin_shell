#include "export.hpp"
#include <iostream>
#include <cstdlib>

int handleExport(const ParsedInput& parsed, ShellState& state) {
    if (parsed.files.empty()) {
        for (const auto& [k, v] : state.variables)
            cout << "declare -x " << k << "=\"" << v << "\"\n";
        return state.recordCommandExitCode(0);
    }

    for (const string& arg : parsed.rawArgs) {
        size_t eq = arg.find('=');
        if (eq != string::npos) {
            string name = arg.substr(0, eq);
            string value = arg.substr(eq + 1);
            state.variables[name] = value;
#ifdef _WIN32
            _putenv_s(name.c_str(), value.c_str());
#else
            setenv(name.c_str(), value.c_str(), 1);
#endif
        } else {
            // export an already-defined shell variable
            auto it = state.variables.find(arg);
            if (it != state.variables.end()) {
#ifdef _WIN32
                _putenv_s(arg.c_str(), it->second.c_str());
#else
                setenv(arg.c_str(), it->second.c_str(), 1);
#endif
            }
        }
    }
    return state.recordCommandExitCode(0);
}
