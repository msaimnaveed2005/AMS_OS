#ifndef CONSOLE_COLORS_H
#define CONSOLE_COLORS_H
#include <iostream>
#include <string>

using namespace std;

namespace Color {
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";

    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string BLUE = "\033[34m";
    const string MAGENTA = "\033[35m";
    const string CYAN = "\033[36m";
    const string WHITE = "\033[37m";
    const string GRAY = "\033[90m";

    const string BRIGHT_RED = "\033[91m";
    const string BRIGHT_GREEN = "\033[92m";
    const string BRIGHT_YELLOW = "\033[93m";
    const string BRIGHT_BLUE = "\033[94m";
    const string BRIGHT_MAGENTA = "\033[95m";
    const string BRIGHT_CYAN = "\033[96m";
    const string BRIGHT_WHITE = "\033[97m";  // Added BRIGHT_WHITE

    /*
    Function: paint
    Purpose: Applies ANSI color formatting to text.
    Parameters: Text and color code.
    Returns: Colored text.
    */
    inline string paint(string text, string color) {
        return color + text + RESET;
    }

    /*
    Function: kernel
    Purpose: Colors kernel messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string kernel(string text) {
        return BOLD + BRIGHT_MAGENTA + text + RESET;
    }

    /*
    Function: parent
    Purpose: Colors parent process messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string parent(string text) {
        return BOLD + BRIGHT_BLUE + text + RESET;
    }

    /*
    Function: child
    Purpose: Colors child process messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string child(string text) {
        return BOLD + BRIGHT_GREEN + text + RESET;
    }

    /*
    Function: process
    Purpose: Colors process manager messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string process(string text) {
        return BOLD + CYAN + text + RESET;
    }

    /*
    Function: resource
    Purpose: Colors resource manager messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string resource(string text) {
        return BOLD + YELLOW + text + RESET;
    }

    /*
    Function: memory
    Purpose: Colors memory manager messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string memory(string text) {
        return BOLD + BRIGHT_CYAN + text + RESET;
    }

    /*
    Function: ready
    Purpose: Colors ready queue messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string ready(string text) {
        return BOLD + BRIGHT_BLUE + text + RESET;
    }

    /*
    Function: scheduler
    Purpose: Colors scheduler messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string scheduler(string text) {
        return BOLD + BRIGHT_YELLOW + text + RESET;
    }

    /*
    Function: deadlock
    Purpose: Colors deadlock messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string deadlock(string text) {
        return BOLD + BRIGHT_RED + text + RESET;
    }

    /*
    Function: success
    Purpose: Colors success messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string success(string text) {
        return BOLD + BRIGHT_GREEN + text + RESET;
    }

    /*
    Function: error
    Purpose: Colors error messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string error(string text) {
        return BOLD + BRIGHT_RED + text + RESET;
    }

    /*
    Function: warning
    Purpose: Colors warning messages.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string warning(string text) {
        return BOLD + BRIGHT_YELLOW + text + RESET;
    }

    /*
    Function: table
    Purpose: Colors table headings.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string table(string text) {
        return BOLD + BRIGHT_CYAN + text + RESET;
    }

    /*
    Function: section
    Purpose: Colors section banners.
    Parameters: Text.
    Returns: Colored text.
    */
    inline string section(string text) {
        return BOLD + WHITE + text + RESET;
    }

/*
Function: cell
Purpose: Prints a colored table cell without breaking alignment.
Parameters: Text, column width, and color code.
Returns: Nothing.
*/
inline void cell(string text, int width, string color) {
    cout << color << text << RESET;

    int padding = width - static_cast<int>(text.length());

    if (padding > 0) {
        cout << string(padding, ' ');
    }
}

/*
Function: line
Purpose: Prints a colored horizontal line.
Parameters: Character, count, and color code.
Returns: Nothing.
*/
inline void line(char character, int count, string color) {
    cout << color;

    for (int i = 0; i < count; i++) {
        cout << character;
    }

    cout << RESET << "\n";
}

/*
Function: number
Purpose: Colors menu option numbers.
Parameters: Text.
Returns: Colored text.
*/
inline string number(string text) {
    return BOLD + BRIGHT_CYAN + text + RESET;
}

/*
Function: label
Purpose: Colors labels in menu and tables.
Parameters: Text.
Returns: Colored text.
*/
inline string label(string text) {
    return BOLD + WHITE + text + RESET;
}
}

#endif