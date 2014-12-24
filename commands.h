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
#define COMMANDS_BUFFER_SIZE 0x3F
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

struct CommandEngine;

typedef unsigned char byte;

typedef void (*WriterMethodType)(const char *string);
typedef byte* (*CommandExecuteMethodType)(const char* args[], struct CommandEngine* commandEngine);
typedef void (*ApplicationLoadMethodType)(const char* args[], struct CommandEngine* commandEngine);
typedef void (*ExecuteApplicationMethodType)(const char input, struct CommandEngine* commandEngine);
typedef void (*ApplicationMethodType)(struct CommandEngine* commandEngine);

////////////////////////////////////////////////////////////////////////////////
// Default Commands
////////////////////////////////////////////////////////////////////////////////

extern const struct Command ClearCommand;
extern const struct Command HelpCommand;

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

// Application state lifecycle
typedef enum {
    Initialize = 0x00,
    CanRead = 0x01,
    CanWrite = 0x02, // TODO
    CanParseForCommand = 0x04,
    CanExecuteCommand = 0x08,
} CommandEngineStatus;

typedef struct Command {
    const char * Name;
    CommandExecuteMethodType Execute;
    const char * HelpText;
} Command;

typedef struct Application {
    const char * Name;
    const char * HelpText;
    ExecuteApplicationMethodType Execute;
    ApplicationLoadMethodType OnStart;
    ApplicationMethodType OnClose;
} Application;

typedef struct CommandEngine {
    byte *CommandBuffer;
    const unsigned int CommandBufferSize;
    const Command** RegisteredCommands;
    const Application** RegisteredApplications;
    WriterMethodType WriteToOutput;
    WriterMethodType WriteError;
    const char* Prompt;
    unsigned short BufferPosition;
    CommandEngineStatus Status;
    const Application* RunningApplication;
    const Command * ParsedCommand;
} CommandEngine;

////////////////////////////////////////////////////////////////////////////////
// Public methods
////////////////////////////////////////////////////////////////////////////////

void DoTasks(CommandEngine* commandEngine);
void AddKeystroke(CommandEngine* commandEngine, unsigned char keystroke);
void CloseApplication(CommandEngine* commandEngine);

#ifdef	__cplusplus
}
#endif

#endif	/* COMMANDS_H */

