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
extern int	strcmp(const char *, const char *);

////////////////////////////////////////////////////////////////////////////////
// Private methods declarations
////////////////////////////////////////////////////////////////////////////////

static void ProcessBuffer(CommandEngine* commandEngine);
static const Command* CheckCommand(struct CommandEngine* commandEngine);
static void ExecuteCommand(struct CommandEngine* commandEngine, const Command* command);
static void ExecuteService(CommandEngine* commandEngine);
static int StringToArgs(char *pRawString, char *argv[]);

////////////////////////////////////////////////////////////////////////////////
// Local buffers
////////////////////////////////////////////////////////////////////////////////

static char characterEchoBuffer[2] = "\0\0";
static char stringFormatBuffer[COMMANDS_BUFFER_SIZE];
static char* argv[MAX_CMD_ARGS];

////////////////////////////////////////////////////////////////////////////////
// Public methods
////////////////////////////////////////////////////////////////////////////////

void DoTasks(CommandEngine* commandEngine)
{
    switch(commandEngine->Status)
    {
        case CanParseForCommand:
            commandEngine->ParsedCommand = CheckCommand(commandEngine);
            commandEngine->Status = CanExecuteCommand;
            break;
        case CanExecuteCommand:
            ExecuteCommand(commandEngine, commandEngine->ParsedCommand);
            commandEngine->ParsedCommand = NULL;
        case Initialize:
            commandEngine->ServiceCount = 0;
            commandEngine->ServiceRunning = 0;
            commandEngine->Status = CanRead;
            if (commandEngine->Prompt != NULL
                    && commandEngine->RunningApplication == NULL) {
                commandEngine->WriteToOutput(commandEngine->Prompt);
            }

            while(commandEngine->RegisteredServices[commandEngine->ServiceCount] != NULL)
            {
                ++commandEngine->ServiceCount;
            }
            break;
        case CanRead:
            ExecuteService(commandEngine);
            break;
        default:
            commandEngine->Status = Initialize;
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
                    characterEchoBuffer[0] = keystroke;
                    commandEngine->WriteError("ASCII character ");
                    commandEngine->WriteError(characterEchoBuffer);
                    commandEngine->WriteError(" is not supported.");
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

static void CheckArguments(struct CommandEngine* commandEngine)
{
    argv[0] = NULL;

    byte * p = commandEngine->CommandBuffer;
    while(*p != ' ')
    {
        if (*p == NULL) {
            return;
        }

        ++p;
    }

    while(*p == ' ')
    {
        if (*p == NULL) {
            return;
        }

        ++p;
    }

    StringToArgs(p, argv);

    return;
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
    for(i = 0; commandEngine->RegisteredCommands[i] != NULL; ++i)
    {
        if (strcmp(commandName, commandEngine->RegisteredCommands[i]->Name) == 0)
        {
            return commandEngine->RegisteredCommands[i];
        }
    }

    for(i = 0; commandEngine->RegisteredApplications[i] != NULL; ++i)
    {
        if (strcmp(commandName, commandEngine->RegisteredApplications[i]->Name) == 0)
        {
            commandEngine->RunningApplication = commandEngine->RegisteredApplications[i];
            if (commandEngine->RunningApplication->OnStart != NULL) {
                CheckArguments(commandEngine);
                commandEngine->RunningApplication->OnStart((const char **)argv, commandEngine);
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
            CheckArguments(commandEngine);
            const char* output = command->Execute((const char **)argv, commandEngine);
            
            if (output != NULL) {
                commandEngine->WriteToOutput(output);
            }
        }
    } else {
        if (commandEngine->RunningApplication != NULL) {
            commandEngine->Status = CanRead;
        } else if (commandEngine->WriteError != NULL) {
            commandEngine->WriteError("Command not found");
        }
    }

    commandEngine->BufferPosition = 0;
    commandEngine->CommandBuffer[commandEngine->BufferPosition] = NULL;

    return;
}

static void ExecuteService(CommandEngine* commandEngine)
{
    if (commandEngine->ServiceCount == 0)
    {
        return;
    }

    if (commandEngine->RegisteredServices[commandEngine->ServiceRunning] == NULL)
    {
        commandEngine->ServiceRunning = 0;
    }

    byte referenceToServiceRunning = commandEngine->ServiceRunning;
    while(commandEngine->RegisteredServices[commandEngine->ServiceRunning]->State == Stopped)
    {
        ++commandEngine->ServiceRunning;

        if (commandEngine->RegisteredServices[commandEngine->ServiceRunning] == NULL) {
            commandEngine->ServiceRunning = 0;
        }
        
        if (commandEngine->ServiceRunning == referenceToServiceRunning)
        {
            return;
        }
    }

    Service * service = commandEngine->RegisteredServices[commandEngine->ServiceRunning];
    service->State = service->Run(service->State, commandEngine);
    ++commandEngine->ServiceRunning;
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
// Helper methods
////////////////////////////////////////////////////////////////////////////////
static int StringToArgs(char *pRawString, char *argv[])
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