#ifndef COMMAND_LIBCTL_H
#define COMMAND_LIBCTL_H

#include "../../console_pipe.h"

BOOL CommandLibCtl(CONSOLE_PIPE cp, LPCSTR argv);
VOID CloseConnectionsOnService(LPFORWARD_LIST_NODE connections, DISPATCH_SERVER ds, LPCSTR serviceName);

#endif // !COMMAND_LIBCTL_H