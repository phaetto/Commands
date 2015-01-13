
#include "commands.h"
#include "command_help.h"

// Imported from commands.c
extern char stringFormatBuffer[COMMANDS_BUFFER_SIZE];

static byte* HelpCommandImplementation(const char* args[], struct CommandEngine* commandEngine)
{
    commandEngine->WriteToOutput(CMD_MAKEBOLD CMD_MAKEGREEN CMD_CRLF "***********************************************************************"
            CMD_CRLF "**" CMD_MAKEWHITE " Available commands in this terminal                               "
            CMD_MAKEGREEN "**"
            CMD_CRLF "***********************************************************************"
            CMD_CRLF CMD_CRLF CMD_CLEARATTRIBUTES
            CMD_MAKEGREEN "Commands:"
            CMD_CRLF CMD_CLEARATTRIBUTES);

    unsigned short i = 0;
    for(i = 0; commandEngine->RegisteredCommands[i] != NULL; ++i)
    {
        const char * description = commandEngine->RegisteredCommands[i]->HelpText != NULL
            ? commandEngine->RegisteredCommands[i]->HelpText
            : "[ No description ]\0";

        sprintf(stringFormatBuffer, "%s" CMD_CRLF "\t%s" CMD_CRLF,
                commandEngine->RegisteredCommands[i]->Name,
                description);
        commandEngine->WriteToOutput(stringFormatBuffer);
    }

    commandEngine->WriteToOutput(CMD_MAKEGREEN CMD_CRLF "Applications:" CMD_CRLF CMD_CLEARATTRIBUTES);

    for(i = 0; commandEngine->RegisteredApplications[i] != NULL; ++i)
    {
        const char * description = commandEngine->RegisteredApplications[i]->HelpText != NULL
            ? commandEngine->RegisteredApplications[i]->HelpText
            : "[ No description ]\0";

        sprintf(stringFormatBuffer, "%s" CMD_CRLF "\t%s" CMD_CRLF,
                commandEngine->RegisteredApplications[i]->Name,
                description);
        commandEngine->WriteToOutput(stringFormatBuffer);
    }

    commandEngine->WriteToOutput(CMD_MAKEGREEN CMD_CRLF "Services:" CMD_CRLF CMD_CLEARATTRIBUTES);

    for(i = 0; commandEngine->RegisteredServices[i] != NULL; ++i)
    {
        const char * description = commandEngine->RegisteredServices[i]->HelpText != NULL
            ? commandEngine->RegisteredServices[i]->HelpText
            : "[ No description ]\0";

        const char * state = commandEngine->RegisteredServices[i]->State == Stopped
            ? CMD_MAKERED "Stopped" CMD_CLEARATTRIBUTES
            : commandEngine->RegisteredServices[i]->State == Starting
                ? CMD_MAKEGREEN "Starting" CMD_CLEARATTRIBUTES
                : CMD_MAKEGREEN "Running" CMD_CLEARATTRIBUTES;

        sprintf(stringFormatBuffer, "%s\t\t[%s] / [0x%02x]" CMD_CRLF "\t%s" CMD_CRLF,
                commandEngine->RegisteredServices[i]->Name,
                state,
                commandEngine->RegisteredServices[i]->State,
                description);
        commandEngine->WriteToOutput(stringFormatBuffer);
    }

    commandEngine->WriteToOutput(CMD_CRLF);

    return (byte*)NULL;
}

const Command HelpCommand = {
    "help",
    HelpCommandImplementation,
    "Provides descriptions for commands."
};
