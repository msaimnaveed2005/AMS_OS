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
#include <unistd.h>

using namespace std;

namespace UI {

    /* ANSI reset and attribute codes */
    const string RESET  = "\033[0m";
    const string BOLD   = "\033[1m";
    const string DIM    = "\033[2m";

    /* Primary palette: royal-blue accent with a neutral GNOME-like base */
    const string ROYAL_BLUE  = "\033[38;5;33m";
    const string LIGHT_BLUE  = "\033[38;5;75m";
    const string SLATE_GRAY  = "\033[38;5;245m";
    const string LIGHT_GRAY  = "\033[38;5;252m";
    const string WHITE       = "\033[38;5;255m";

    /* Reserved semantic colours for state and alert feedback */
    const string GREEN       = "\033[38;5;35m";
    const string YELLOW      = "\033[38;5;178m";
    const string RED         = "\033[38;5;160m";
    const string CYAN        = LIGHT_BLUE;
    const string MAGENTA     = SLATE_GRAY;

    /* Convenience aliases used throughout the codebase */
    const string BLUE           = ROYAL_BLUE;
    const string BRIGHT_CYAN    = LIGHT_BLUE;
    const string BRIGHT_BLUE    = ROYAL_BLUE;
    const string BRIGHT_MAGENTA = MAGENTA;
    const string BRIGHT_GREEN   = GREEN;
    const string BRIGHT_YELLOW  = YELLOW;

    /* Inline helper utilities */

    inline string paint(const string &text, const string &style) {
        return style + text + RESET;
    }

    inline string repeat(char value, int count) {
        return string(max(0, count), value);
    }

    inline string fit(const string &text, int width) {
        if (width <= 0) return "";
        if ((int)text.length() <= width) return text;
        if (width <= 3) return text.substr(0, width);
        return text.substr(0, width - 3) + "...";
    }

    /* Panel rendering */

    inline void panelHeader(const string &title, const string &subtitle = "", int width = 78) {
        cout << "\n" << paint("+" + repeat('-', width - 2) + "+\n", ROYAL_BLUE + BOLD);

        string content = " " + title;

        if (!subtitle.empty()) {
            int gap = width - 2 - (int)content.length() - (int)subtitle.length() - 1;
            content += repeat(' ', max(1, gap)) + subtitle;
        }

        cout << paint("|", ROYAL_BLUE + BOLD)
             << left << setw(width - 2)
             << fit(content, width - 2)
             << paint("|\n", ROYAL_BLUE + BOLD);

        cout << paint("+" + repeat('-', width - 2) + "+\n", SLATE_GRAY);
    }

    inline void panelFooter(int width = 78) {
        cout << paint("+" + repeat('-', width - 2) + "+\n", ROYAL_BLUE + BOLD);
    }

    /* Section headings */

    inline void sectionTitle(const string &title, int width = 78) {
        cout << "\n" << paint("* " + title, ROYAL_BLUE + BOLD) << "\n";
        cout << paint(repeat('-', min(width, 78)) + "\n", DIM);
    }

    inline void sectionBanner(const string &title, const string &accent = ROYAL_BLUE) {
        cout << "\n  "
             << paint("*", accent + BOLD) << " "
             << paint(title, accent + BOLD)
             << paint(" " + repeat('.', max(0, 46 - (int)title.length())), DIM)
             << "\n";
    }

    /* Key-value output */

    inline void keyValue(const string &key, const string &value, int keyWidth = 24) {
        cout << "  "
             << left << setw(keyWidth)
             << paint(key, LIGHT_BLUE)
             << paint(value, WHITE + BOLD)
             << "\n";
    }

    inline string statusPill(const string &text, const string &style) {
        return paint("[ " + text + " ]", style + BOLD);
    }

    /* Terminal control */

    inline void clearScreen() {
        cout << "\033[2J\033[H";
    }

    inline int parentProcessID() {
        return getppid();
    }

    /* Dashboard widgets */

    inline void metric(const string &label, const string &value, const string &hint = "") {
        cout << "  "
             << left << setw(18)
             << paint(label, LIGHT_BLUE)
             << setw(16)
             << paint(value, WHITE + BOLD);

        if (!hint.empty()) {
            cout << paint(hint, DIM);
        }

        cout << "\n";
    }

    inline string usageBar(int used, int total, int width = 28) {
        if (total <= 0) {
            return "[" + repeat('-', width) + "] 0%";
        }

        used = max(0, min(used, total));

        int filled = (int)((double)used / total * width + 0.5);
        filled = max(0, min(filled, width));

        int percent = (int)((double)used / total * 100 + 0.5);

        string barStyle = GREEN;

        if (percent >= 80) {
            barStyle = RED;
        } else if (percent >= 55) {
            barStyle = YELLOW;
        }

        ostringstream output;

        output << "["
               << paint(repeat('#', filled), barStyle + BOLD)
               << paint(repeat('-', width - filled), DIM)
               << "] "
               << setw(3) << percent << "%";

        return output.str();
    }

    /* Menu rendering */

    inline void menuItem(int id, const string &label, const string &hint = "") {
        ostringstream idText;
        idText << right << setw(2) << id;

        cout << "  "
             << paint("[" + idText.str() + "]", ROYAL_BLUE + BOLD)
             << "  "
             << left << setw(30)
             << paint(label, LIGHT_GRAY);

        if (!hint.empty()) {
            cout << paint(hint, DIM);
        }

        cout << "\n";
    }

    inline void modeSplash(const string &primary, const string &secondary) {
        cout << "  "
             << statusPill(primary, ROYAL_BLUE)
             << "  "
             << statusPill(secondary, SLATE_GRAY)
             << "\n";
    }

    /* Status line helpers */

    inline void infoLine(const string &message) {
        cout << "  " << paint("i", LIGHT_BLUE + BOLD) << " "
             << paint(message, LIGHT_GRAY) << "\n";
    }

    inline void successLine(const string &message) {
        cout << "  " << paint("OK", GREEN + BOLD) << " "
             << paint(message, GREEN) << "\n";
    }

    inline void warnLine(const string &message) {
        cout << "  " << paint("!", YELLOW + BOLD) << " "
             << paint(message, YELLOW) << "\n";
    }

    inline void errorLine(const string &message) {
        cout << "  " << paint("ERR", RED + BOLD) << " "
             << paint(message, RED + BOLD) << "\n";
    }

    /* ASCII boot logo */

    inline void asciiLogo() {
        const vector<pair<string, string> > lines = {
            {"        _    __  __ ____       ___  ____  ", ROYAL_BLUE},
            {"       / \\  |  \\/  / ___|     / _ \\/ ___| ", ROYAL_BLUE},
            {"      / _ \\ | |\\/| \\___ \\    | | | \\___ \\ ", LIGHT_BLUE},
            {"     / ___ \\| |  | |___) |   | |_| |___) |", LIGHT_BLUE},
            {"    /_/   \\_\\_|  |_|____/     \\___/|____/ ", ROYAL_BLUE}
        };

        cout << "\n";

        for (const auto &line : lines) {
            cout << paint(line.first, line.second + BOLD) << "\n";
        }

        cout << "\n";
        cout << paint("    AMS OS - Atomic Management System\n", ROYAL_BLUE + BOLD);
        cout << paint("    Ubuntu Terminal Edition\n", SLATE_GRAY);
        cout << "\n";
    }

    inline void bootStep(const string &label, const string &message) {
        cout << "  "
             << statusPill(label, ROYAL_BLUE)
             << " "
             << paint(message, LIGHT_GRAY)
             << "\n";
    }

    /* Ubuntu-style command prompt */

    inline void commandPrompt(const string &text) {
        cout << paint("\nams", GREEN + BOLD)
             << paint("@", LIGHT_GRAY)
             << paint("ubuntu", ROYAL_BLUE + BOLD)
             << paint(":~/AMS_OS$ ", LIGHT_BLUE + BOLD)
             << paint(text, WHITE + BOLD);
    }

    /* Empty-state placeholder */

    inline void emptyState(const string &message, int width = 78) {
        cout << "  " << paint(message, DIM) << "\n";
        panelFooter(width);
    }

    /* Task control hint for child executables */

    inline void taskControlHint(int pid, bool autoManaged = false) {
        const char* amsOSPID = getenv("AMS_OS_PID");
        string displayedPID = to_string(pid);

        if (amsOSPID != nullptr && string(amsOSPID).length() > 0) {
            displayedPID = amsOSPID;
        }

        keyValue("AMS OS PID", displayedPID);

        if (autoManaged) {
            cout << "  "
                 << paint("Auto task: finishes automatically or can be closed from Kernel Mode.\n", DIM);
        } else {
            cout << "  "
                 << paint("Task controls: close from menu 21 or minimize from menu 17 using this PID.\n", DIM);
            cout << "  "
                 << paint("Ubuntu terminal controls: Ctrl+C closes task, Ctrl+Z pauses task.\n", DIM);
        }
    }
}

#endif
