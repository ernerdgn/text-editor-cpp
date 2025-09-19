#include "editor.h"

// holding original console state
#ifdef _WIN32
HANDLE hStdin;
DWORD dwOriginalMode;
#else
struct termios orig_termios;
#endif

// error & exit
void die(const char *s)
{
    std::cerr << s << ": " << strerror(errno) << std::endl;
    exit(1);
}

// restoring original console settings
void disableRawMode()
{
#ifdef _WIN32
    if (!SetConsoleMode(hStdin, dwOriginalMode))
    {
        std::cerr << "Failed to restore console mode!" << std::endl;
    }
#else
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
#endif
}

// enable raw mode
void enableRawMode()
{
#ifdef _WIN32
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        die("GetStdHandle");
    if (!GetConsoleMode(hStdin, &dwOriginalMode))
        die("GetConsoleMode");

    atexit(disableRawMode); // register dRM for exit

    DWORD dwNewMode = dwOriginalMode;
    // ENABLE_PROCESSED_INPUT to prevent CTRL+C from exiting
    dwNewMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

    if (!SetConsoleMode(hStdin, dwNewMode))
        die("SetConsoleMode");
#else
    if (!isatty(STDIN_FILENO))
    {
        std::cerr << "ERROR: Not running in a terminal" << std::endl;
        exit(1);
    }

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");

    atexit(disableRawMode);

    struct termios raw = orig_termios;
    // standard set of flags for enabling raw mode on POSIX
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
#endif
}

void refreshScreen(const editorState& state)
{
    // clear using an ANSI escape code
    std::cout << "\x1b[2J";
    
    // pos cursor top left
    std::cout << "\x1b[H";

    // draw lines
    for (const auto& line : state.lines) {
        std::cout << line << "\r\n";
    }

    std::cout << "\x1b[7m";  // color invert
    std::cout << state.status_msg;

    for (size_t i = state.status_msg.length(); i < 80; ++i) std::cout << "";  // 80 columns terminal
    std::cout << "\x1b[m";  // turn off color invert

    // move cursor
    // ANSI escape code to move cursor rowY,columnX
    // terminal rows/cols are 1-based(some index shit), so add 1
    std::cout << "\x1b[" << (state.cursorY + 1) << ";" << (state.cursorX + 1) << "H";

    // flush to see real time
    std::cout.flush();
}

void editorSetStatusMessage(editorState& state, const std::string& msg)
{
    state.status_msg = msg;
}

int editorReadKey()
{
#ifdef _WIN32
    int c = _getch();
    if (c == 0xE0 || c == 0)  // spec keys
    {
        switch (_getch())
        {
            case 72: return ARROW_UP;
            case 75: return ARROW_LEFT;
            case 77: return ARROW_RIGHT;
            case 80: return ARROW_DOWN;
            case 83: return DELETE_KEY;
            default: return -1;  // unknown spec key
        }
    }

    else if (c == 8) return BACKSPACE;
    
    else return c; // regular key
#else
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    { if (nread == -1 && errno != EAGAIN) die("read"); }

    if (c == '\x1b')
    {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[')
        {
            if (seq[1] >= '0' && seq[1] <= '9')
            {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~')
                {
                    switch (seq[1])
                    {
                        case '3': return DELETE_KEY;
                    }
                }
            }

            else
            {
                switch (seq[1])
                {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                }
            }
        }

        return '\x1b';
    }
    else return c;  // posix backspace ascii 127
#endif
}

void editorMoveCursor(int key, editorState& state)
{
    switch (key)
    {
        case ARROW_LEFT:
            if (state.cursorX > 0) state.cursorX--;  // move left in line
            else if (state.cursorX <= 0 && state.cursorY > 0)
            {
                state.cursorY--;
                state.cursorX = state.lines[state.cursorY].length();  // move previous lines end if exists
            }
            break;
        case ARROW_RIGHT:
            if (state.cursorY < state.lines.size() && state.cursorX < state.lines[state.cursorY].length()) state.cursorX++;
            else if (state.cursorX >= state.lines[state.cursorY].length() && state.cursorY < state.lines.size() - 1)
            {
                state.cursorY++;
                state.cursorX = 0;
            }
            break;
        case ARROW_UP:
            if (state.cursorY > 0) state.cursorY--;
            break;
        case ARROW_DOWN:
            if (state.cursorY < state.lines.size() - 1) state.cursorY++;
            break;
    }
    // send cursor at the end of the line if new line is shorter
    if (state.cursorY < state.lines.size() && state.cursorX > state.lines[state.cursorY].length()) state.cursorX = state.lines[state.cursorY].length();
}

void editorInsertChar(char c, editorState& state)
{
    if (state.cursorY < state.lines.size())
    {
        state.lines[state.cursorY].insert(state.cursorX, 1, c);
        state.cursorX++;
    }
}

void editorDeleteChar(editorState& state)
{
    if (state.cursorY >= state.lines.size()) return;  // cannot delete past buffer
    if (state.cursorX == 0 && state.cursorY == 0) return;  // cannot delete at the start

    std::string& line = state.lines[state.cursorY];
    if (state.cursorX > 0)
    {
        // delete char to the left in the same line
        line.erase(state.cursorX - 1, 1);
        state.cursorX--;
    }
    else
    {
        // cursor at start of line, go line above
        state.cursorX = state.lines[state.cursorY - 1].length();
        state.lines[state.cursorY - 1] += line;
        state.lines.erase(state.lines.begin() + state.cursorY);
        state.cursorY--;
    }
}

void editorInsertNewLine(editorState& state)
{
    if (state.cursorY >= state.lines.size()) state.lines.push_back("");

    std::string& currentLine = state.lines[state.cursorY];
    // before current line
    if (state.cursorX == 0) state.lines.insert(state.lines.begin() + state.cursorY, "");
    // after current line
    else if (state.cursorX >= currentLine.length()) state.lines.insert(state.lines.begin() + state.cursorY + 1, "");
    // split line
    else
    {
        std::string newLine = currentLine.substr(state.cursorX);
        currentLine.erase(state.cursorX);
        state.lines.insert(state.lines.begin() + state.cursorY + 1, newLine);
    }

    //move cursor to the new line
    state.cursorY++;
    state.cursorX = 0;
}

void editorOpen(const char* file_name, editorState& state)
{
    state.file_name = file_name;

    std::fstream file(file_name);
    if (!file.is_open())  // if file doesnt exist
    {
        state.lines.push_back("");
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // handle carriage returns at the end of lines
        if (!line.empty() && line.back() == '\r') line.pop_back();
        state.lines.push_back(line);
    }

    // if the file is empty, start with blank line
    if (state.lines.empty()) state.lines.push_back("");
}

void editorSave(editorState& state)
{
    if (state.file_name.empty())
    {
        editorSetStatusMessage(state, "ERROR (1): No filename to save to");
        return;
    }

    std::ofstream file(state.file_name);
    if (!file.is_open())
    {
        editorSetStatusMessage(state, "ERROR (2): Can't open file for writing");
        return;
    }

    long long total_bytes = 0;
    for (const auto& line: state.lines)
    {
        file << line << "\n";
        total_bytes += line.length() + 1;  // +1 for new line
    }

    file.close();

    editorSetStatusMessage(state, std::to_string(total_bytes) + " bytes written to disk");
}

void editorProcessKeypress(editorState& state)
{
    int c = editorReadKey();

    switch (c)
    {
        case ('q'): //TODO: ctrl+q
            std::cout << "\x1b[2J";
            std::cout << "\x1b[H";
            std::cout.flush();
            exit(0);
            break;
        case '\r':  // enter
            editorInsertNewLine(state);
            break;
        case BACKSPACE:
            editorDeleteChar(state);
            break;
        case DELETE_KEY:
            // change cursor position to right and apply delete
            if(state.cursorX >= state.lines[state.cursorY].length() && state.cursorY == state.lines.size() - 1) break;  // dont work if there is nothing to the right
            editorMoveCursor(ARROW_RIGHT, state);
            editorDeleteChar(state);
            break;
        case ('s' & 0x1f):  // ctrl+s to save
            editorSave(state);
            break;
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c, state);
            break;
        default:
            if (isprint(c)) editorInsertChar(c, state);  // printable
            break;
    }
}