/*
text editor in c++
*/
#include "editor.h"

int main(int argc, char* argv[])
{
    enableRawMode();

    editorState state;

    if (argc >= 2) editorOpen(argv[1], state);
    else state.lines.push_back("");

    // state.lines.push_back("Hello, World!");
    // state.lines.push_back("This is the second line.");
    // state.lines.push_back("Shorter third line.");
    editorSetStatusMessage(state, "HELP: ctrl+s to save | q to quit");

    while (true)
    {
        refreshScreen(state);
        state.status_msg.clear();
        editorProcessKeypress(state);
    }

    return 0;
}