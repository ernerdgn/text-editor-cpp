#pragma once

// include libs
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib> // atexit()
#include <cctype>  // iscntrl()
#include <vector>
#include <fstream> // file i/o

#ifdef _WIN32 // if working on winOS
#include <windows.h> // winAPI
#include <conio.h> // _getch()
#else // posix systems
#include <unistd.h> // read(), isatty(), STDIN_FILENO
#include <termios.h> // terminal settings
#include <cerrno>  // For errno
#endif

// es
struct editorState {
    int cursorX = 0;
    int cursorY = 0;
    std::vector<std::string> lines;
    std::string file_name;
    std::string status_msg;
};

// ek enum
enum editorKey
{
    BACKSPACE = 127,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DELETE_KEY
};

void die(const char *s);
void disableRawMode();
void enableRawMode();
void refreshScreen(const editorState& state);
void editorSetStatusMessage(editorState& state, const std::string& msg);
int editorReadKey();
void editorMoveCursor(int key, editorState& state);
void editorInsertChar(char c, editorState& state);
void editorDeleteChar(editorState& state);
void editorInsertNewLine(editorState& state);
void editorOpen(const char* file_name, editorState& state);
void editorSave(editorState& state);
void editorProcessKeypress(editorState& state);
