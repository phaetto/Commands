
#include "CommandEngine.h"
#include "../command_clear.h"

////////////////////////////////////////////////////////////////////////////////
// Module definitions
////////////////////////////////////////////////////////////////////////////////

#define MAX_BUFFER 0xFF
#define NULL (0L)

byte CommandBuffer[MAX_BUFFER];

////////////////////////////////////////////////////////////////////////////////
// Example command implementation
//      Commands run on demand from terminal. They can have variable arguments,
//      like any terminal command and as long as they are executing the other
//      parts of the system are halted. If asynchronisity is needed, then pair
//      it with a background service.
//
//      Usage: example-command [???]
////////////////////////////////////////////////////////////////////////////////
byte* ExampleImplementation(const char* args[], struct CommandEngine* commandEngine)
{
    // TODO: interact with the service
    return CMD_CRLF "Done." CMD_CRLF;
}

const Command ExampleCommand = {
    "example-command",
    ExampleImplementation
};

////////////////////////////////////////////////////////////////////////////////
// Example process implementation
//      Processes are the same as commands, the only difference is that
//      they keep the focus from the command engine. No commands can run
//      when a process is running, however services keep looping on the
//      background. You can use CTRL+C to close a process
//
//      Usage: example.exe [???]
////////////////////////////////////////////////////////////////////////////////

void ExampleAppImplementation(const char key, struct CommandEngine* commandEngine)
{
    // TODO: Use buttons to orchestrate services
}

void ExampleAppLoadImplementation(const char* args[], struct CommandEngine* commandEngine)
{
    commandEngine->WriteToOutput(
    CMD_CRLF "Example process"
    // TODO: add description about the buttons
    CMD_CRLF);
}

void ExampleAppCloseImplementation(struct CommandEngine* commandEngine)
{
    commandEngine->WriteToOutput("\r\n[Ended]\r\n");
}

const Application ExampleApplication = {
    "example.exe",
    "Example process",
    ExampleAppImplementation,
    ExampleAppLoadImplementation,
    ExampleAppCloseImplementation
};

////////////////////////////////////////////////////////////////////////////////
// Example background services
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Background service that runs forever
//      Example of a service that recycles states forever doing a task. The
//      state implementations should be as smaller as possible to break the
//      computation time across services and processes/commands.
//
////////////////////////////////////////////////////////////////////////////////

byte ServiceExample1Implementation(byte state, struct CommandEngine* commandEngine)
{
    // Do something depending the state
    switch(state)
    {
        case Starting:
            return 0x01;
        case 0x01:
            // TODO: Loops and processes some input
            return 0x01;
        case 0x02:
            // TODO: Does something more specific, but still loops to state 0x01
            return 0x01;
        default:
            break;
    }

    return Stopped;
}

Service Example1Service = {
    "Example service 1",
    "Example background service that runs forever",
    ServiceExample1Implementation,
};

////////////////////////////////////////////////////////////////////////////////
// Background service that runs once
//      Example of a service that does a task after is has been requested from
//      a process or a command. This service starts as stopped.
//
////////////////////////////////////////////////////////////////////////////////

byte ServiceExample2Implementation(byte state, struct CommandEngine* commandEngine)
{
    // Do something depending the state
    switch(state)
    {
        case Starting:
            return 0x01;
        case 0x01:
            // TODO: Step 1
            return 0x02;
        case 0x02:
        default:
            // TODO: Step 2 then stopping
            break;
    }

    return Stopped;
}

Service Example2Service = {
    "Example service 2",
    "Example background service that runs on demand",
    ServiceExample2Implementation,
    Stopped,
};

////////////////////////////////////////////////////////////////////////////////
// Command engine definition
////////////////////////////////////////////////////////////////////////////////
const Command* Commands[] = {
    &HelpCommand,
    &ClearCommand,
    NULL
};

const Application* Applications[] = {
    &ExampleApplication,
    NULL
};

Service* Services[] = {
    &Example1Service,
    &Example2Service,
    NULL
};

void DefaultErrorToOutput(const char *string) {
    WriteString(CMD_MAKEYELLOW);
    WriteString(CMD_MAKEBOLD);
    WriteString(CMD_CRLF);
    WriteString(string);
    WriteString(CMD_CLEARATTRIBUTES);
};

CommandEngine CurrentCommandEngine = {
    CommandBuffer,
    MAX_BUFFER,
    Commands,
    Applications,
    Services,
    (WriterMethodType)WriteString,
    (WriterMethodType)DefaultErrorToOutput,
    "$> ",
};