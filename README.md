# Command Engine
An implementation of a terminal-like parser and execution engine for C based microcontrollers.

## Why
I had a need of terminal provider for software debugging and probing of microcontroller services through a protocol.

So I have made this small engine for custom predefined commands that you can easily extend.

## Testing
I have been using PIC32 for testing this project. The family of microcontrollers must support pointers to functions (older 8-bit PIC families do not :-( ).

## Features
The command-engine includes
* Command parser and executioner
* Process executioner (full-focus commands)
* Background services

The services and the command executioner are been implemented as a state-machine and can share CPU time. The same happens when a process executes and has the terminal focus.

## Making it run
The example/CommandEngine.[h/c] are the files that you want to reuse/customise.

I will evolve the examples and the library in time.

The software is provided under The MIT License.
