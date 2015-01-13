
#include "commands.h"
#include "command_clear.h"

// Clear command
static byte* ClearCommandImplementation(const char* args[], struct CommandEngine* commandEngine)
{
    return (byte*)CMD_CLEARSCREEN;
}

const Command ClearCommand = {
    "clear",
    ClearCommandImplementation,
    "Clears the console."
};
