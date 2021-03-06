#ifndef COMMANDS_H
#define	COMMANDS_H

#ifdef	__cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#ifndef MAX_CMD_ARGS
#define MAX_CMD_ARGS 3
#endif

#ifndef COMMANDS_BUFFER_SIZE
#define COMMANDS_BUFFER_SIZE 0x1F
#endif

#ifndef NULL
#define NULL (0)
#endif

#define ESC_ASCII (0x1b)
#define TAB_ASCII (0x09)
#define RETURN_ASCII (0x0D)
#define BACKSPACE_ASCII (0x7F)
#define CTRL_C_ASCII (0x03)                 // ETX, End Of Text

#define CMD_CRLF "\r\n"
#define CMD_CLEARSCREEN "\x1B[2J\x1B[;H"
#define CMD_MAKEWHITE "\x1B[37;40m"
#define CMD_MAKEGREEN "\x1B[32;40m"
#define CMD_MAKERED "\x1B[31;40m"
#define CMD_MAKEYELLOW "\x1B[33;40m"
#define CMD_MAKEBOLD "\x1B[1m"
#define CMD_CLEARATTRIBUTES "\x1B[0m"

#define TO_HEX(i) ((i) <= 9 ? '0' + (i) : 'A' - 10 + (i))

struct CommandEngine;

typedef unsigned char byte;

typedef void (*WriterMethodType)(const char *string);
typedef byte* (*CommandExecuteMethodType)(const char* args[], struct CommandEngine* commandEngine);
typedef void (*ApplicationOnStartMethodType)(const char* args[], struct CommandEngine* commandEngine);
typedef void (*ApplicationOnInputMethodType)(const char input, struct CommandEngine* commandEngine);
typedef void (*ApplicationOnCloseMethodType)(struct CommandEngine* commandEngine);
typedef byte (*ApplicationOnStateExecuteMethodType)(byte step, struct CommandEngine* commandEngine);
typedef byte (*ServiceStateExecuteMethodType)(byte state, void* data, struct CommandEngine* commandEngine);

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

// Application state lifecycle
typedef enum {
    InitializeStatus = 0x00,
    ReadyForInputStatus,
    LoopStatus,
    WriteStatus, // TODO
    ParseForCommandStatus,
    ExecuteCommandStatus,
    ExecuteApplicationStatus,
    ExecuteServicesStatus,
    ExecuteServicesBetweenParseAndExecuteStatus,
} CommandEngineStatus;

typedef enum {
    ReadyForKeyInputStatus = 0x00,
    ReadyToParseStatus,
    ReadyToShowPromptStatus,
} KeyInputStatus;

typedef struct Command {
    const char * Name;
    CommandExecuteMethodType Execute;
    const char * HelpText;
} Command;

typedef struct Application {
    const char * Name;
    const char * HelpText;
    ApplicationOnInputMethodType OnInput;
    ApplicationOnStartMethodType OnStart;
    ApplicationOnCloseMethodType OnClose;
    ApplicationOnStateExecuteMethodType Run;
    // Private fields
    byte State;
} Application;

//Service wellknow lifecycle states
typedef enum {
    Starting = 0x00,
    Stopped = 0xFF,
} ServiceStatus;

typedef struct Service {
    const char * Name;
    const char * HelpText;
    ServiceStateExecuteMethodType Run;
    byte State;
    void * Data;
} Service;

typedef struct CommandEngine {
    byte *CommandBuffer;
    const unsigned int CommandBufferSize;
    const Command** RegisteredCommands;
    Application** RegisteredApplications;
    Service** RegisteredServices;
    WriterMethodType WriteToOutput;
    WriterMethodType WriteError;
    const char* Prompt;
    const char* Intro;
    // Private fields
    unsigned short BufferPosition;
    CommandEngineStatus Status;
    KeyInputStatus KeyInputStatus;
    Application* RunningApplication;
    byte ServiceCount;
    byte ServiceRunning;
    const Command * ParsedCommand;
    byte KeystrokeReceived : 1;
} CommandEngine;

////////////////////////////////////////////////////////////////////////////////
// Public methods
////////////////////////////////////////////////////////////////////////////////

// Basic command engine API
void DoTasks(CommandEngine* commandEngine);
void AddKeystroke(CommandEngine* commandEngine, unsigned char keystroke);

// Application API
void CloseApplication(CommandEngine* commandEngine);

#ifdef	__cplusplus
}
#endif

#endif	/* COMMANDS_H */

