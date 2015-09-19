
#include "commands.h"
#include "command_help.h"

static byte* HelpCommandImplementation(const char* args[], struct CommandEngine* commandEngine)
{
    commandEngine->WriteToOutput(CMD_MAKEBOLD CMD_MAKEGREEN CMD_LF 
            CMD_LF " (*)" CMD_MAKEWHITE " Available commands in this terminal"
            CMD_LF CMD_LF CMD_CLEARATTRIBUTES
            CMD_MAKEGREEN "Commands:"
            CMD_LF CMD_CLEARATTRIBUTES);

    unsigned short i = 0;
    for(i = 0; commandEngine->RegisteredCommands[i] != NULL; ++i)
    {
        const char * description = commandEngine->RegisteredCommands[i]->HelpText != NULL
            ? commandEngine->RegisteredCommands[i]->HelpText
            : "[ No description ]\0";

        commandEngine->WriteToOutput(commandEngine->RegisteredCommands[i]->Name);
        commandEngine->WriteToOutput(CMD_LF "\t");
        commandEngine->WriteToOutput(description);
        commandEngine->WriteToOutput(CMD_LF);
    }

    commandEngine->WriteToOutput(CMD_MAKEGREEN CMD_LF "Applications:" CMD_LF CMD_CLEARATTRIBUTES);

    for(i = 0; commandEngine->RegisteredApplications[i] != NULL; ++i)
    {
        const char * description = commandEngine->RegisteredApplications[i]->HelpText != NULL
            ? commandEngine->RegisteredApplications[i]->HelpText
            : "[ No description ]\0";

        commandEngine->WriteToOutput(commandEngine->RegisteredApplications[i]->Name);
        commandEngine->WriteToOutput(CMD_LF "\t");
        commandEngine->WriteToOutput(description);
        commandEngine->WriteToOutput(CMD_LF);
    }

    commandEngine->WriteToOutput(CMD_LF);

    return (byte*)NULL;
}

const Command HelpCommand = {
    "help",
    HelpCommandImplementation,
    "Provides descriptions for commands."
};
