#ifndef COMMANDS_H
#define	COMMANDS_H

#ifdef	__cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define MAX_CMD_ARGS 5

#define ESC_ASCII (0x1b)
#define TAB_ASCII (0x09)
#define RETURN_ASCII (0x0D)
#define BACKSPACE_ASCII (0x7F)
#define CTRL_C_ASCII (0x03)         // ETX, End Of Text

struct CommandEngine;

typedef unsigned char byte;

typedef void (*WriterMethodType)(const char *string);
typedef byte* (*CommandExecuteMethodType)(const char* args[], struct CommandEngine* commandEngine);
typedef void (*ApplicationLoadMethodType)(const char* args[], struct CommandEngine* commandEngine);
typedef void (*ExecuteApplicationMethodType)(const char input, struct CommandEngine* commandEngine);
typedef void (*ApplicationMethodType)(struct CommandEngine* commandEngine);

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////
extern const char LineFeed;
extern const char* Crlf;
extern const char* ClearScreen;
extern const char* MakeWhite;
extern const char* MakeGreen;
extern const char* MakeRed;
extern const char* MakeYellow;
extern const char* MakeBold;
extern const char* ClearAttributes;

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
    const unsigned short RegisteredCommandsCount;
    const Application** RegisteredApplications;
    const unsigned short RegisteredApplicationsCount;
    WriterMethodType WriteToOutput;
    WriterMethodType WriteError;
    const char* Prompt;
    unsigned short BufferPosition;
    CommandEngineStatus Status;
    const Application* RunningApplication;
} CommandEngine;

////////////////////////////////////////////////////////////////////////////////
// Public methods
////////////////////////////////////////////////////////////////////////////////

void DoTasks(CommandEngine* commandEngine);
void AddKeystroke(CommandEngine* commandEngine, unsigned char keystroke);
void CloseApplication(CommandEngine* commandEngine);

////////////////////////////////////////////////////////////////////////////////
// Private methods
////////////////////////////////////////////////////////////////////////////////

static void ProcessBuffer(CommandEngine* commandEngine);
static const Command* CheckCommand(struct CommandEngine* commandEngine);
static void ExecuteCommand(struct CommandEngine* commandEngine, const Command* command);
static int StringToArgs(char *pRawString, const char *argv[]);

#ifdef	__cplusplus
}
#endif

#endif	/* COMMANDS_H */
