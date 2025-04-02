# Nano Shell
Nano Shell is a simple command-line shell implementation in C. It supports basic shell commands, local variables, and environment variable management.

## Features
**Basic Shell Commands**: Supports echo, pwd, cd, and exit.

**Local Variables**: Allows setting and retrieving local variables.

**Environment Variables**: Implements the export command to set environment variables.

**Command Execution**: Supports running external commands using execvp().

## How It Works
Uses fgets() to read user input.

Tokenizes input using strtok().

Identifies and executes built-in commands.

Uses fork() and execvp() for running external commands.

Manages local and environment variables with linked lists and setenv().

## Notes
Environment variables set with export will persist for child processes but not in the parent shell session.

If a command is not found, an error message is displayed.

Variable assignments must follow the format VAR=value without spaces.
