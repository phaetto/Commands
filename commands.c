#ifndef COMMANDS_C
#define	COMMANDS_C

#include <stdbool.h>

#include "commands.h"

////////////////////////////////////////////////////////////////////////////////
// Imports
////////////////////////////////////////////////////////////////////////////////

static void strncpy(char * s1, const char * s2, unsigned short size)
{
    while (*s2 != '\0' && size > 0)
    {
        *s1 = *s2;
        s1++;
        s2++;
        size--;
    }
}

static unsigned short strlen(const char * s)
{
    int result = 0;

    while (*s != '\0')
    {
        result++;
        s++;
    }

    return result;
}

static short strcmp(const char * s1, const char * s2)
{
    while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2)
    {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

////////////////////////////////////////////////////////////////////////////////
// Private methods declarations
////////////////////////////////////////////////////////////////////////////////

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
        case InitializeStatus:
            commandEngine->KeyInputStatus = ReadyForKeyInputStatus;
            commandEngine->ServiceCount = 0;
            commandEngine->ServiceRunning = 0;

            while(commandEngine->RegisteredServices[commandEngine->ServiceCount] != NULL)
            {
                ++commandEngine->ServiceCount;
            }
            
            commandEngine->Status = ReadyForInputStatus;
            
            break;
        case ParseForCommandStatus:
            commandEngine->ParsedCommand = CheckCommand(commandEngine);
            
            if (commandEngine->RunningApplication == NULL)
            {
                commandEngine->Status = ExecuteServicesStatus;
            }
            else
            {
                commandEngine->Status = ExecuteApplicationStatus;
            }
            
            break;
        case ExecuteCommandStatus:
            ExecuteCommand(commandEngine, commandEngine->ParsedCommand);
            commandEngine->ParsedCommand = NULL;
            
        case ReadyForInputStatus:
            commandEngine->BufferPosition = 0;
            
            if (commandEngine->Prompt != NULL
                    && commandEngine->RunningApplication == NULL
                    && commandEngine->KeystrokeReceived)
            {
                commandEngine->WriteToOutput(commandEngine->Prompt);    
            }
            
            commandEngine->Status = LoopStatus;
            
            break;
        case LoopStatus:
            
            if (commandEngine->ParsedCommand != NULL)
            {
                commandEngine->Status = ExecuteCommandStatus;
            }
            else if (commandEngine->KeyInputStatus == ReadyToParseStatus)
            {
                commandEngine->Status = ParseForCommandStatus;
                commandEngine->KeyInputStatus = ReadyForKeyInputStatus;
            }
            else if (commandEngine->KeyInputStatus == ReadyToShowPromptStatus)
            {
                commandEngine->Status = ReadyForInputStatus;
                commandEngine->KeyInputStatus = ReadyForKeyInputStatus;
            }
            else if (commandEngine->RunningApplication == NULL)
            {
                commandEngine->Status = ExecuteServicesStatus;
            }
            else
            {
                commandEngine->Status = ExecuteApplicationStatus;
            }
            
            break;
        case ExecuteApplicationStatus:
            // ExecuteApplicationLoop(commandEngine);
            commandEngine->Status = ExecuteServicesStatus;
            
            break;
        case ExecuteServicesStatus:
            ExecuteService(commandEngine);
            commandEngine->Status = LoopStatus;
            
            break;
        default:
            commandEngine->Status = InitializeStatus;
            
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
            commandEngine->RunningApplication->OnInput(keystroke, commandEngine);
        } else {
            CloseApplication(commandEngine);
        }

        return;
    }

    if (keystroke == NULL) {
        return;
    }
    
    if (!commandEngine->KeystrokeReceived) {
        if (commandEngine->Intro != NULL) {
            commandEngine->WriteToOutput(commandEngine->Intro);
        }
            
        commandEngine->BufferPosition = 0;
        commandEngine->CommandBuffer[commandEngine->BufferPosition] = NULL;
        commandEngine->KeyInputStatus = ReadyToShowPromptStatus;
        
        commandEngine->KeystrokeReceived = true;
        
        return;
    }

    if (keystroke == RETURN_ASCII) {
        if (commandEngine->BufferPosition != 0) {
            commandEngine->KeyInputStatus = ReadyToParseStatus;
        }
        else {
            commandEngine->KeyInputStatus = ReadyToShowPromptStatus;
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
            case CTRL_C_ASCII:
                // Clear the buffer and press 'enter'
                commandEngine->KeyInputStatus = ReadyToShowPromptStatus;
                
                commandEngine->BufferPosition = 0;
                commandEngine->CommandBuffer[commandEngine->BufferPosition] = NULL;
                
                commandEngine->WriteToOutput(CMD_CRLF);
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
                    char hex[3];
                    hex[0] = TO_HEX(((keystroke & 0xF0) >> 4));
                    hex[1] = TO_HEX(((keystroke & 0x0F)));
                    hex[2] = '\0';

                    commandEngine->WriteError(CMD_CRLF "ASCII character 0x");
                    commandEngine->WriteError(hex);
                    commandEngine->WriteError(" is not supported." CMD_CRLF);
                }
        }

        commandEngine->CommandBuffer[commandEngine->BufferPosition] = NULL;

        return;
    }

    // Buffer overflow, we have to lose data
    if (commandEngine->WriteError != NULL) {
        commandEngine->WriteError(CMD_CRLF "Buffer overflow!" CMD_CRLF);
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
    
    if (commandEngine->RunningApplication == NULL && commandEngine->WriteError != NULL)
    {
        commandEngine->WriteError(CMD_CRLF "Command '");
        commandEngine->WriteError(commandName);
        commandEngine->WriteError("' not found" CMD_CRLF);
        
        commandEngine->KeyInputStatus = ReadyToShowPromptStatus;
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
    service->State = service->Run(service->State, service->Data, commandEngine);
    ++commandEngine->ServiceRunning;
    return;
}

void CloseApplication(CommandEngine* commandEngine)
{
    if (commandEngine->RunningApplication->OnClose != NULL) {
        commandEngine->RunningApplication->OnClose(commandEngine);
    }

    commandEngine->RunningApplication = NULL;
    commandEngine->KeyInputStatus = ReadyToShowPromptStatus;
}

////////////////////////////////////////////////////////////////////////////////
// Helper methods
////////////////////////////////////////////////////////////////////////////////
static int StringToArgs(char *pRawString, char *argv[])
{
    unsigned short argc = 0, i = 0, strsize = 0;

    if(pRawString == NULL)
    {
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