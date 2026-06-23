#include "dispatch.hpp"
#include "src/commands/handlers/tools/cat.hpp"
#include "src/commands/handlers/tools/ls.hpp"
#include "src/commands/handlers/tools/echo.hpp"
#include "src/commands/handlers/builtins/cd.hpp"
#include "src/commands/handlers/builtins/pwd.hpp"
#include "src/commands/handlers/tools/mkdir.hpp"
#include "src/commands/handlers/tools/touch.hpp"
#include "src/commands/handlers/tools/rm.hpp"
#include "src/commands/handlers/tools/rmdir.hpp"
#include "src/commands/handlers/builtins/history.hpp"
#include "src/commands/handlers/tools/clear.hpp"
#include "src/commands/handlers/tools/grep.hpp"
#include "src/commands/handlers/tools/cp.hpp"
#include "src/commands/handlers/tools/mv.hpp"
#include "src/commands/handlers/builtins/help.hpp"


int dispatchCommand(Command cmd, const ParsedInput& parsed, ShellState& state)
{
    switch (cmd)
    {
    case Command::CAT:  return handleCat(parsed, state);
    case Command::ECHO: return handleEcho(parsed, state);
    case Command::PWD:  return handlePwd(state);
    case Command::CD:   return handleCd(parsed.rawArgs, state);

    case Command::LS: return handleLs(parsed, state);

    case Command::GREP:    return handleGrep(parsed, state);
    case Command::MKDIR:   return handleMkdir(parsed, state);
    case Command::RM:      return handleRm(parsed, state);
    case Command::RMDIR:   return handleRmdir(parsed, state);
    case Command::CP:      return handleCp(parsed, state);
    case Command::MV:      return handleMv(parsed, state);
    case Command::TOUCH:   return handleTouch(parsed, state);
    case Command::CLEAR:   return handleClear(parsed, state);
    case Command::HISTORY: return handleHistory(parsed, state);
    case Command::HELP:    return handleHelp(parsed, state);

    default: return 0;
    }
}
