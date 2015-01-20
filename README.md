# Command Engine
An implementation of a terminal parser and execution engine for C based microcontrollers.

## Why
For debugging, updating on the field or running your own background applications.

I had a need of terminal provider for software debugging and probing of microcontroller services through a serial protocol.

So I have made this small engine for custom commands that you can easily extend.

## Testing
I have been testing on PIC32/PIC16. The compiler for the family of microcontrollers you select must support pointers to functions.

## Features
The command-engine includes
* Command parser and executioner
* Process executioner (full-focus commands)
* Background services

The services and the command executioner are been implemented as a state-machine and can share CPU time. The same happens when a process executes and has the terminal focus.

## Try it!
Follow the instructions on https://github.com/phaetto/PICTerminalExample

The software is provided under The MIT License.
