
#include "commands.h"
#include "command_services.h"

static byte* ServicesCommandImplementation(const char* args[], struct CommandEngine* commandEngine)
{
    commandEngine->WriteToOutput(CMD_MAKEBOLD CMD_MAKEGREEN CMD_CRLF 
            CMD_CRLF " (*)" CMD_MAKEWHITE " Available services in this terminal"
            CMD_CRLF CMD_CRLF CMD_CLEARATTRIBUTES);

    unsigned short i = 0;
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

        char hex[3];
        hex[0] = TO_HEX(((commandEngine->RegisteredServices[i]->State & 0xF0) >> 4));
        hex[1] = TO_HEX(((commandEngine->RegisteredServices[i]->State & 0x0F)));
        hex[2] = '\0';

        commandEngine->WriteToOutput(commandEngine->RegisteredServices[i]->Name);
        commandEngine->WriteToOutput("\t\t[");
        commandEngine->WriteToOutput(state);
        commandEngine->WriteToOutput("] / [0x");
        commandEngine->WriteToOutput(hex);
        commandEngine->WriteToOutput("]" CMD_CRLF "\t");
        commandEngine->WriteToOutput(description);
        commandEngine->WriteToOutput(CMD_CRLF);
    }

    commandEngine->WriteToOutput(CMD_CRLF);

    return (byte*)NULL;
}

const Command ServicesCommand = {
    "services",
    ServicesCommandImplementation,
    "Provides details of the background services."
};
