#ifndef COMMANDS_C
#define	COMMANDS_C

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "commands.h"

////////////////////////////////////////////////////////////////////////////////
// Imports
////////////////////////////////////////////////////////////////////////////////

extern char *   strncpy(char *, const char *, size_t);
extern size_t   strlen(const char *);
extern int      sprintf(char *, const char *, ...);
extern int	strcmp(const char *, const char *);

////////////////////////////////////////////////////////////////////////////////
// Local buffers
////////////////////////////////////////////////////////////////////////////////

static char characterEchoBuffer[2] = "\0\0";
static char stringFormatBuffer[COMMANDS_BUFFER_SIZE];

////////////////////////////////////////////////////////////////////////////////
// Public methods
////////////////////////////////////////////////////////////////////////////////

void DoTasks(CommandEngine* commandEngine)
{
    static const Command * parsedCommand;

    switch(commandEngine->Status)
    {
        case CanParseForCommand:
            parsedCommand = CheckCommand(commandEngine);
            commandEngine->Status = CanExecuteCommand;
            break;
        case CanExecuteCommand:
            ExecuteCommand(commandEngine, parsedCommand);
            parsedCommand = NULL;
        case Initialize:
            commandEngine->Status = CanRead;
            if (commandEngine->Prompt != NULL
                    && commandEngine->RunningApplication == NULL) {
                commandEngine->WriteToOutput(commandEngine->Prompt);
            }
            break;
        case CanRead:
            break;
        default:
            if (commandEngine->WriteError != NULL) {
#ifndef COMMANDS_SMALL_FOOTPRINT
                sprintf(stringFormatBuffer, "Invalid status with number %d" CMD_CRLF, commandEngine->Status);
                commandEngine->WriteError(stringFormatBuffer);
#else
                commandEngine->WriteError("Invalid status." CMD_CRLF);
#endif
            }
            break;
    }

    return;
}

void AddKeystroke(CommandEngine* commandEngine, unsigned char keystroke)
{
    if (commandEngine->RunningApplication != NULL) {
        if (keystroke != CTRL_C_ASCII) {
            // Feed all the characters synchronously in the application
            // It is the application's responsibility to be state-based
            commandEngine->RunningApplication->Execute(keystroke, commandEngine);
        } else {
            CloseApplication(commandEngine);
        }

        return;
    }

    if (keystroke == NULL) {
        return;
    }

    if (keystroke == RETURN_ASCII) {
        if (commandEngine->BufferPosition != 0) {
            commandEngine->Status = CanParseForCommand;
        }
        else {
            commandEngine->Status = Initialize;
            commandEngine->WriteToOutput(CMD_CRLF);
        }
        return;
    }

    if (commandEngine->BufferPosition < commandEngine->CommandBufferSize - 1)
    {
        switch (keystroke)
        {
            case BACKSPACE_ASCII:
                if (commandEngine->BufferPosition > 0)
                {
                    --commandEngine->BufferPosition;

                    characterEchoBuffer[0] = keystroke;
                    commandEngine->WriteToOutput(characterEchoBuffer);
                }
                break;
            case RETURN_ASCII:
                // Do not print CRLF here
                break;
            default:
                if (keystroke > 31)
                {
                    commandEngine->CommandBuffer[commandEngine->BufferPosition] = keystroke;
                    ++commandEngine->BufferPosition;

                    characterEchoBuffer[0] = keystroke;
                    commandEngine->WriteToOutput(characterEchoBuffer);
                }
                else
                {
#ifndef COMMANDS_SMALL_FOOTPRINT
                    sprintf(stringFormatBuffer, "ASCII character 0x%02x is not supported." CMD_CRLF, keystroke);
                    commandEngine->WriteError(stringFormatBuffer);
#else
                    commandEngine->WriteError("ASCII character is not supported." CMD_CRLF);
#endif
                }
        }

        commandEngine->CommandBuffer[commandEngine->BufferPosition] = NULL;

        return;
    }

    // Buffer overflow, we have to lose data
    if (commandEngine->WriteError != NULL) {
        commandEngine->WriteError("Buffer overflow!" CMD_CRLF);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Private methods
////////////////////////////////////////////////////////////////////////////////

static const char** CheckArguments(struct CommandEngine* commandEngine)
{
    static const char* argv[MAX_CMD_ARGS];
    argv[0] = NULL;

    byte * p = commandEngine->CommandBuffer;
    while(*p != ' ')
    {
        if (*p == NULL) {
            return argv;
        }

        ++p;
    }

    while(*p == ' ')
    {
        if (*p == NULL) {
            return argv;
        }

        ++p;
    }

    StringToArgs(p, argv);

    return argv;
}

static const Command* CheckCommand(struct CommandEngine* commandEngine)
{
    byte * p = commandEngine->CommandBuffer;
    while(*p != ' ' && *p != NULL)
    {
        ++p;
    }

    unsigned short index = (p - commandEngine->CommandBuffer);
    char * commandName = stringFormatBuffer;
    strncpy(commandName, commandEngine->CommandBuffer, index);
    commandName[index] = NULL;

    unsigned short i = 0;
    for(i = 0; i < commandEngine->RegisteredCommandsCount; ++i)
    {
        if (strcmp(commandName, commandEngine->RegisteredCommands[i]->Name) == 0)
        {
            return commandEngine->RegisteredCommands[i];
        }
    }

    for(i = 0; i < commandEngine->RegisteredApplicationsCount; ++i)
    {
        if (strcmp(commandName, commandEngine->RegisteredApplications[i]->Name) == 0)
        {
            commandEngine->RunningApplication = commandEngine->RegisteredApplications[i];
            if (commandEngine->RunningApplication->OnStart != NULL) {
                commandEngine->RunningApplication->OnStart(CheckArguments(commandEngine), commandEngine);
            }
            break;
        }
    }

    return (Command*) NULL;
}

static void ExecuteCommand(CommandEngine* commandEngine, const Command* command)
{
    if (command != NULL) {
        if (commandEngine->WriteToOutput != NULL) {
            const char ** args = CheckArguments(commandEngine);
            const char* output = command->Execute(args, commandEngine);
            
            if (output != NULL) {
                commandEngine->WriteToOutput(output);
            }
        }
    } else {
        if (commandEngine->RunningApplication != NULL) {
            commandEngine->Status = CanRead;
        } else if (commandEngine->WriteError != NULL) {
#ifndef COMMANDS_SMALL_FOOTPRINT
            sprintf(stringFormatBuffer, "Command '%s' cound not be found" CMD_CRLF, commandEngine->CommandBuffer);
            commandEngine->WriteError(stringFormatBuffer);
#else
            commandEngine->WriteError("Command cound not be found" CMD_CRLF);
#endif
        }
    }

    commandEngine->BufferPosition = 0;
    commandEngine->CommandBuffer[commandEngine->BufferPosition] = NULL;

    return;
}

void CloseApplication(CommandEngine* commandEngine)
{
    if (commandEngine->RunningApplication->OnClose != NULL) {
        commandEngine->RunningApplication->OnClose(commandEngine);
    }

    commandEngine->Status = Initialize;
    commandEngine->RunningApplication = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Default Commands
////////////////////////////////////////////////////////////////////////////////

#ifndef COMMANDS_SMALL_FOOTPRINT
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

// Help command
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
    for(i = 0; i < commandEngine->RegisteredCommandsCount; ++i)
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

    for(i = 0; i < commandEngine->RegisteredApplicationsCount; ++i)
    {
        const char * description = commandEngine->RegisteredApplications[i]->HelpText != NULL
            ? commandEngine->RegisteredApplications[i]->HelpText
            : "[ No description ]\0";

        sprintf(stringFormatBuffer, "%s" CMD_CRLF "\t%s" CMD_CRLF,
                commandEngine->RegisteredApplications[i]->Name,
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

#endif

////////////////////////////////////////////////////////////////////////////////
// Helper methods
////////////////////////////////////////////////////////////////////////////////
static int StringToArgs(char *pRawString, const char *argv[])
{
    size_t argc = 0, i = 0, strsize = 0;

    if(pRawString == NULL) {
        return 0;
    }

    strsize = strlen(pRawString);

    while(argc < MAX_CMD_ARGS) {
        while ((*pRawString == ' ') || (*pRawString == '\t')) {
            ++pRawString;
            if (++i >= strsize) {
                return (argc);
            }
        }

        if (*pRawString == '\0') {
            argv[argc] = NULL;
            return (argc);
        }

        argv[argc++] = pRawString;

        while (*pRawString && (*pRawString != ' ') && (*pRawString != '\t')) {
            ++pRawString;
        }

        if (*pRawString == '\0') {
            argv[argc] = NULL;
            return (argc);
        }

        *pRawString++ = '\0';
    }

    return (0);
}

#endif