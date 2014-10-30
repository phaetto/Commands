#ifndef COMMANDS_C
#define	COMMANDS_C

#include <stdio.h>
#include <stdarg.h>

#include "commands.h"

////////////////////////////////////////////////////////////////////////////////
// Imports
////////////////////////////////////////////////////////////////////////////////

extern char * strncpy(char *, const char *, size_t);
extern size_t strlen(const char *);

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

const char* Crlf = "\r\n";
const char* ClearScreen = "\x1B[2J\x1B[;H";
const char* MakeWhite = "\x1B[37;40m";
const char* MakeYellow = "\x1B[33;40m";
const char* MakeGreen = "\x1B[32;40m";
const char* MakeRed = "\x1B[31;40m";
const char* MakeBold = "\x1B[1m";
const char* ClearAttributes = "\x1B[0m";
char stringFormatBuffer[0x3F];
char characterEchoBuffer[2] = "\0\0";

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
                sprintf(stringFormatBuffer, "Invalid status with number %d%s", commandEngine->Status, Crlf);
                commandEngine->WriteError(stringFormatBuffer);
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
        commandEngine->Status = CanParseForCommand;
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
                // Do not print anything
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
                    sprintf(stringFormatBuffer, "ASCII character 0x%02x is not supported.%s", keystroke, Crlf);
                    commandEngine->WriteError(stringFormatBuffer);
                }
        }

        commandEngine->CommandBuffer[commandEngine->BufferPosition] = NULL;

        return;
    }

    // Buffer overflow, we have to lose data
    if (commandEngine->WriteError != NULL) {
        sprintf(stringFormatBuffer, "Buffer overflow!%s", Crlf);
        commandEngine->WriteError(stringFormatBuffer);
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

    const unsigned short index = (p - commandEngine->CommandBuffer);
    char commandName[commandEngine->CommandBufferSize];
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
                commandEngine->RunningApplication->OnStart(commandEngine);
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
            const char* output = command->Execute(CheckArguments(commandEngine), commandEngine);
            
            if (output != NULL) {
                commandEngine->WriteToOutput(output);
            }
        }
    } else {
        if (commandEngine->RunningApplication != NULL) {
            commandEngine->Status = CanRead;
        } else if (commandEngine->WriteError != NULL) {
            sprintf(stringFormatBuffer, "Command '%s' cound not be found%s", commandEngine->CommandBuffer, Crlf);
            commandEngine->WriteError(stringFormatBuffer);
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

// Clear command
static byte* ClearCommandImplementation(const char* args[], struct CommandEngine* commandEngine)
{
    return (byte*)ClearScreen;
}

const Command ClearCommand = {
    "clear",
    (ExecuteMethodType)ClearCommandImplementation,
    "Clears the console."
};

// Help command
static byte* HelpCommandImplementation(const char* args[], struct CommandEngine* commandEngine)
{
    WriteString(MakeBold);
    WriteString(MakeGreen);
    WriteString("\r\n***********************************************************************\r\n");
    WriteString("**");
    WriteString(MakeWhite);
    WriteString(" Available commands in this terminal                               ");
    WriteString(MakeGreen);
    WriteString("**\r\n");
    WriteString("***********************************************************************\r\n\r\n");
    WriteString(ClearAttributes);
    
    WriteString(MakeGreen);
    WriteString("Commands:\r\n");
    WriteString(ClearAttributes);

    unsigned short i = 0;
    for(i = 0; i < commandEngine->RegisteredCommandsCount; ++i)
    {
        const char * description = commandEngine->RegisteredCommands[i]->HelpText != NULL
            ? commandEngine->RegisteredCommands[i]->HelpText
            : "[ No description ]";

        sprintf(stringFormatBuffer, "%s%s\t%s%s",
                commandEngine->RegisteredCommands[i]->Name,
                Crlf,
                description,
                Crlf);
        commandEngine->WriteToOutput(stringFormatBuffer);
    }

    WriteString(MakeGreen);
    WriteString("\r\nApplications:\r\n");
    WriteString(ClearAttributes);

    for(i = 0; i < commandEngine->RegisteredApplicationsCount; ++i)
    {
        const char * description = commandEngine->RegisteredApplications[i]->HelpText != NULL
            ? commandEngine->RegisteredApplications[i]->HelpText
            : "[ No description ]";

        sprintf(stringFormatBuffer, "%s%s\t%s%s",
                commandEngine->RegisteredApplications[i]->Name,
                Crlf,
                description,
                Crlf);
        commandEngine->WriteToOutput(stringFormatBuffer);
    }

    commandEngine->WriteToOutput("\r\n");

    return (byte*)NULL;
}

const Command HelpCommand = {
    "help",
    (ExecuteMethodType)HelpCommandImplementation,
    "Provides descriptions for commands."
};

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