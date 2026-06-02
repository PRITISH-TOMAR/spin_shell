#include "dispatch.hpp"

// All handler includes live here — this is the only file that needs them.
// Add a new #include + case whenever a new command is implemented.
#include "src/commands/handlers/tools/cat.hpp"
#include "src/commands/handlers/tools/ls.hpp"
#include "src/commands/handlers/tools/echo.hpp"
#include "src/commands/handlers/builtins/cd.hpp"
#include "src/commands/handlers/builtins/pwd.hpp"

int dispatchCommand(Command cmd, const ParsedInput& parsed, ShellState& state)
{
    switch (cmd)
    {
    case Command::CAT:  return handleCat(parsed, state);
    case Command::ECHO: return handleEcho(parsed, state);
    case Command::PWD:  return handlePwd(state);
    case Command::CD:   return handleCd(parsed.rawArgs, state);

    case Command::LS: return handleLs(parsed, state);

    // TODO: wire these as their handlers are implemented
    // case Command::GREP:    return handleGrep(parsed, state);
    case Command::MKDIR:   return handleMkdir(parsed, state);
    // case Command::RM:      return handleRm(parsed, state);
    // case Command::CP:      return handleCp(parsed, state);
    // case Command::MV:      return handleMv(parsed, state);
    // case Command::TOUCH:   return handleTouch(parsed, state);
    // case Command::CLEAR:   return handleClear(parsed, state);
    // case Command::HISTORY: return handleHistory(parsed, state);
    // case Command::HELP:    return handleHelp(parsed, state);

    default: return 0;
    }
}
