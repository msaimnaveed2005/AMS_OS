#ifndef UI_H
#define UI_H

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

namespace UI {
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";
    const std::string CYAN = "\033[36m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string RED = "\033[31m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string WHITE = "\033[37m";
    const std::string BRIGHT_CYAN = "\033[96m";
    const std::string BRIGHT_BLUE = "\033[94m";
    const std::string BRIGHT_MAGENTA = "\033[95m";

    /*
    Function: paint
    Purpose: Wraps text in ANSI terminal styling codes.
    Parameters: Text and ANSI style code.
    Returns: Styled terminal string.
    */
    inline std::string paint(const std::string &text, const std::string &style) {
        return style + text + RESET;
    }

    /*
    Function: repeat
    Purpose: Builds a repeated character string for console borders.
    Parameters: Character and repeat count.
    Returns: Repeated character string.
    */
    inline std::string repeat(char value, int count) {
        return std::string(std::max(0, count), value);
    }

    /*
    Function: fit
    Purpose: Truncates long text so it fits inside a fixed-width terminal column.
    Parameters: Text and maximum width.
    Returns: Original or truncated text.
    */
    inline std::string fit(const std::string &text, int width) {
        if (width <= 0) {
            return "";
        }

        if (static_cast<int>(text.length()) <= width) {
            return text;
        }

        if (width <= 3) {
            return text.substr(0, width);
        }

        return text.substr(0, width - 3) + "...";
    }

    /*
    Function: panelHeader
    Purpose: Prints a styled panel header for major console screens.
    Parameters: Title, optional subtitle, and panel width.
    Returns: Nothing.
    */
    inline void panelHeader(const std::string &title, const std::string &subtitle = "", int width = 78) {
        std::cout << "\n" << paint("╔" + repeat('=', width - 2) + "╗\n", BRIGHT_CYAN);
        std::string content = " " + title;

        if (!subtitle.empty()) {
            int gap = width - 2 - static_cast<int>(content.length()) - static_cast<int>(subtitle.length()) - 1;
            content += repeat(' ', std::max(1, gap)) + subtitle;
        }

        std::cout << paint("║", BRIGHT_CYAN) << std::left << std::setw(width - 2)
                  << fit(content, width - 2) << paint("║\n", BRIGHT_CYAN);
        std::cout << paint("╠" + repeat('=', width - 2) + "╣\n", BRIGHT_CYAN);
    }

    /*
    Function: panelFooter
    Purpose: Prints a styled panel footer line.
    Parameters: Panel width.
    Returns: Nothing.
    */
    inline void panelFooter(int width = 78) {
        std::cout << paint("╚" + repeat('=', width - 2) + "╝\n", BRIGHT_CYAN);
    }

    /*
    Function: sectionTitle
    Purpose: Prints a compact section title with divider.
    Parameters: Title and divider width.
    Returns: Nothing.
    */
    inline void sectionTitle(const std::string &title, int width = 78) {
        std::cout << "\n" << paint("[" + title + "]", BOLD) << "\n";
        std::cout << paint(repeat('-', std::min(width, 78)) + "\n", DIM);
    }

    /*
    Function: keyValue
    Purpose: Prints a readable key-value row.
    Parameters: Key, value, and key column width.
    Returns: Nothing.
    */
    inline void keyValue(const std::string &key, const std::string &value, int keyWidth = 24) {
        std::cout << "  " << std::left << std::setw(keyWidth) << key
                  << paint(value, BOLD) << "\n";
    }

    /*
    Function: statusPill
    Purpose: Creates a compact colored status label.
    Parameters: Label text and ANSI style.
    Returns: Styled status label string.
    */
    inline std::string statusPill(const std::string &text, const std::string &style) {
        return paint("[ " + text + " ]", style + BOLD);
    }

    /*
    Function: clearScreen
    Purpose: Clears the terminal screen using ANSI escape codes.
    Parameters: None.
    Returns: Nothing.
    */
    inline void clearScreen() {
        std::cout << "\033[2J\033[H";
    }

    /*
    Function: parentProcessID
    Purpose: Returns the parent process ID where supported by the target platform.
    Parameters: None.
    Returns: Parent process ID on Linux, or 0 on Windows syntax-check builds.
    */
    inline int parentProcessID() {
#ifdef _WIN32
        return 0;
#else
        return getppid();
#endif
    }

    /*
    Function: metric
    Purpose: Prints a dashboard metric row with optional hint text.
    Parameters: Label, value, and hint.
    Returns: Nothing.
    */
    inline void metric(const std::string &label, const std::string &value, const std::string &hint = "") {
        std::cout << "  " << std::left << std::setw(18) << label
                  << std::setw(16) << paint(value, BOLD);

        if (!hint.empty()) {
            std::cout << paint(hint, DIM);
        }

        std::cout << "\n";
    }

    /*
    Function: usageBar
    Purpose: Creates a colored usage bar for resource dashboards.
    Parameters: Used amount, total amount, and bar width.
    Returns: Styled usage bar string.
    */
    inline std::string usageBar(int used, int total, int width = 28) {
        if (total <= 0) {
            return "[" + repeat('-', width) + "] 0%";
        }

        used = std::max(0, std::min(used, total));
        int filled = static_cast<int>((static_cast<double>(used) / total) * width + 0.5);
        filled = std::max(0, std::min(filled, width));

        int percent = static_cast<int>((static_cast<double>(used) / total) * 100 + 0.5);
        std::string barStyle = GREEN;

        if (percent >= 80) {
            barStyle = RED;
        } else if (percent >= 55) {
            barStyle = YELLOW;
        }

        std::ostringstream output;
        output << "["
               << paint(repeat('#', filled), barStyle + BOLD)
               << paint(repeat('-', width - filled), DIM)
               << "] "
               << std::setw(3) << percent << "%";
        return output.str();
    }

    /*
    Function: menuItem
    Purpose: Prints one formatted menu command row.
    Parameters: Menu ID, label, and optional hint.
    Returns: Nothing.
    */
    inline void menuItem(int id, const std::string &label, const std::string &hint = "") {
        std::ostringstream idText;
        idText << std::right << std::setw(2) << id;

        std::cout << "  " << paint("[" + idText.str() + "]", BRIGHT_MAGENTA + BOLD) << "  "
                  << std::left << std::setw(30) << paint(label, WHITE);

        if (!hint.empty()) {
            std::cout << paint(hint, DIM);
        }

        std::cout << "\n";
    }

    /*
    Function: asciiLogo
    Purpose: Prints a colorful AMS OS startup logo for the terminal interface.
    Parameters: None.
    Returns: Nothing.
    */
    inline void asciiLogo() {
        const std::vector<std::pair<std::string, std::string>> lines = {
            {R"(        ___       __  ___   _____    ____  _____)", BRIGHT_CYAN},
            {R"(       /   |     /  |/  /  / ___/   / __ \/ ___/)", BRIGHT_BLUE},
            {R"(      / /| |    / /|_/ /   \__ \   / / / /\__ \ )", BRIGHT_MAGENTA},
            {R"(     / ___ |   / /  / /   ___/ /  / /_/ /___/ /)", BRIGHT_BLUE},
            {R"(    /_/  |_|  /_/  /_/   /____/   \____//____/ )", BRIGHT_CYAN}
        };

        std::cout << "\n";
        for (const auto &line : lines) {
            std::cout << paint(line.first, line.second + BOLD) << "\n";
        }
        std::cout << paint("        Atomic Management System Simulator", YELLOW + BOLD) << "\n";
    }

    /*
    Function: bootStep
    Purpose: Prints one colored boot-stage message.
    Parameters: Boot stage label and message.
    Returns: Nothing.
    */
    inline void bootStep(const std::string &label, const std::string &message) {
        std::cout << "  " << statusPill(label, GREEN) << " "
                  << paint(message, WHITE) << "\n";
    }

    /*
    Function: commandPrompt
    Purpose: Prints a colorful command prompt marker before user input.
    Parameters: Prompt text.
    Returns: Nothing.
    */
    inline void commandPrompt(const std::string &text) {
        std::cout << paint("\nams-os", BRIGHT_CYAN + BOLD)
                  << paint(" :: ", DIM)
                  << paint(text, YELLOW + BOLD);
    }

    /*
    Function: emptyState
    Purpose: Prints a styled empty-state message inside a panel.
    Parameters: Message and panel width.
    Returns: Nothing.
    */
    inline void emptyState(const std::string &message, int width = 78) {
        std::cout << "  " << paint(message, DIM) << "\n";
        panelFooter(width);
    }

    /*
    Function: taskControlHint
    Purpose: Prints AMS OS tracked PID and close/minimize instructions for task windows.
    Parameters: Task PID and whether the task is auto-managed.
    Returns: Nothing.
    */
    inline void taskControlHint(int pid, bool autoManaged = false) {
        const char* amsOSPID = std::getenv("AMS_OS_PID");
        std::string displayedPID = std::to_string(pid);

        if (amsOSPID != nullptr && std::string(amsOSPID).length() > 0) {
            displayedPID = amsOSPID;
        }

        keyValue("AMS OS PID", displayedPID);

        if (autoManaged) {
            std::cout << "  " << paint("Auto task: it finishes on its own, or can be closed from Kernel Mode.\n", DIM);
        } else {
            std::cout << "  " << paint("Close/Minimize: return to AMS OS and use menu options 21 or 17 with this PID.\n", DIM);
        }
    }
}

#endif
