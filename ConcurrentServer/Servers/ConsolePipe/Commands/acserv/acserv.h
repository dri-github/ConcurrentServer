#ifndef COMMAND_ACSERV_H
#define COMMAND_ACSERV_H

#include "../../console_pipe.h"

BOOL CommandAcServ(CONSOLE_PIPE cp, LPCSTR argv);
BOOL SetAllAcServ(LPFORWARD_LIST_NODE lpAss, DWORD state);

#endif // !COMMAND_ACSERV_H