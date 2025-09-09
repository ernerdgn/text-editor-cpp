/*
text editor in c++
*/

// include libs
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>   // atexit()
#include <cctype>    // iscntrl()

#ifdef _WIN32      // if working on winOS
#include <windows.h> // winAPI
#include <conio.h>   // _getch()
#else              // posix systems
#include <unistd.h>  // read(), isatty(), STDIN_FILENO
#include <termios.h> // terminal settings
#include <cerrno>    // For errno
#endif

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

int main()
{
    enableRawMode();
    std::cout << "Starting raw input mode, press 'q' to quit.\r\n";

    while (true)
    {
        char c = '\0';
#ifdef _WIN32
        c = _getch();
#else
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
#endif
        if (c == '\0') continue;

        if (iscntrl(c))
        {
            std::cout << static_cast<int>(c) << "\r\n";
        }
        else
        {
            std::cout << c << " (" << static_cast<int>(c) << ")\r\n";
        }

        if (c == 'q')
        {
            break;
        }
    }

    return 0;
}