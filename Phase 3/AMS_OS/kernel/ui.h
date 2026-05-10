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
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string DIM = "\033[2m";

    const string UBUNTU_ORANGE = "\033[38;5;208m";
    const string UBUNTU_PURPLE = "\033[38;5;91m";
    const string UBUNTU_LIGHT = "\033[38;5;253m";
    const string UBUNTU_GREEN = "\033[38;5;40m";
    const string UBUNTU_RED = "\033[38;5;196m";
    const string UBUNTU_YELLOW = "\033[38;5;220m";
    const string UBUNTU_BLUE = "\033[38;5;39m";

    const string CYAN = UBUNTU_BLUE;
    const string GREEN = UBUNTU_GREEN;
    const string YELLOW = UBUNTU_YELLOW;
    const string RED = UBUNTU_RED;
    const string BLUE = UBUNTU_BLUE;
    const string MAGENTA = UBUNTU_PURPLE;
    const string WHITE = UBUNTU_LIGHT;
    const string BRIGHT_CYAN = UBUNTU_ORANGE;
    const string BRIGHT_BLUE = UBUNTU_BLUE;
    const string BRIGHT_MAGENTA = UBUNTU_PURPLE;
    const string BRIGHT_GREEN = UBUNTU_GREEN;
    const string BRIGHT_YELLOW = UBUNTU_YELLOW;

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

    inline void panelHeader(const string &title, const string &subtitle = "", int width = 78) {
        cout << "\n" << paint("+" + repeat('-', width - 2) + "+\n", UBUNTU_ORANGE + BOLD);

        string content = " " + title;

        if (!subtitle.empty()) {
            int gap = width - 2 - (int)content.length() - (int)subtitle.length() - 1;
            content += repeat(' ', max(1, gap)) + subtitle;
        }

        cout << paint("|", UBUNTU_ORANGE + BOLD)
             << left << setw(width - 2)
             << fit(content, width - 2)
             << paint("|\n", UBUNTU_ORANGE + BOLD);

        cout << paint("+" + repeat('-', width - 2) + "+\n", UBUNTU_PURPLE);
    }

    inline void panelFooter(int width = 78) {
        cout << paint("+" + repeat('-', width - 2) + "+\n", UBUNTU_ORANGE + BOLD);
    }

    inline void sectionTitle(const string &title, int width = 78) {
        cout << "\n" << paint("* " + title, UBUNTU_ORANGE + BOLD) << "\n";
        cout << paint(repeat('-', min(width, 78)) + "\n", DIM);
    }

    inline void sectionBanner(const string &title, const string &accent = UBUNTU_ORANGE) {
        cout << "\n  "
             << paint("*", accent + BOLD) << " "
             << paint(title, accent + BOLD)
             << paint(" " + repeat('.', max(0, 46 - (int)title.length())), DIM)
             << "\n";
    }

    inline void keyValue(const string &key, const string &value, int keyWidth = 24) {
        cout << "  "
             << left << setw(keyWidth)
             << paint(key, UBUNTU_ORANGE)
             << paint(value, UBUNTU_LIGHT + BOLD)
             << "\n";
    }

    inline string statusPill(const string &text, const string &style) {
        return paint("[ " + text + " ]", style + BOLD);
    }

    inline void clearScreen() {
        cout << "\033[2J\033[H";
    }

    inline int parentProcessID() {
        return getppid();
    }

    inline void metric(const string &label, const string &value, const string &hint = "") {
        cout << "  "
             << left << setw(18)
             << paint(label, UBUNTU_ORANGE)
             << setw(16)
             << paint(value, UBUNTU_LIGHT + BOLD);

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

        string barStyle = UBUNTU_GREEN;

        if (percent >= 80) {
            barStyle = UBUNTU_RED;
        } else if (percent >= 55) {
            barStyle = UBUNTU_YELLOW;
        }

        ostringstream output;

        output << "["
               << paint(repeat('#', filled), barStyle + BOLD)
               << paint(repeat('-', width - filled), DIM)
               << "] "
               << setw(3) << percent << "%";

        return output.str();
    }

    inline void menuItem(int id, const string &label, const string &hint = "") {
        ostringstream idText;
        idText << right << setw(2) << id;

        cout << "  "
             << paint("[" + idText.str() + "]", UBUNTU_ORANGE + BOLD)
             << "  "
             << left << setw(30)
             << paint(label, UBUNTU_LIGHT);

        if (!hint.empty()) {
            cout << paint(hint, DIM);
        }

        cout << "\n";
    }

    inline void modeSplash(const string &primary, const string &secondary) {
        cout << "  "
             << statusPill(primary, UBUNTU_ORANGE)
             << "  "
             << statusPill(secondary, UBUNTU_PURPLE)
             << "\n";
    }

    inline void infoLine(const string &message) {
        cout << "  " << paint("i", UBUNTU_BLUE + BOLD) << " "
             << paint(message, UBUNTU_LIGHT) << "\n";
    }

    inline void successLine(const string &message) {
        cout << "  " << paint("OK", UBUNTU_GREEN + BOLD) << " "
             << paint(message, UBUNTU_GREEN) << "\n";
    }

    inline void warnLine(const string &message) {
        cout << "  " << paint("!", UBUNTU_YELLOW + BOLD) << " "
             << paint(message, UBUNTU_YELLOW) << "\n";
    }

    inline void errorLine(const string &message) {
        cout << "  " << paint("ERR", UBUNTU_RED + BOLD) << " "
             << paint(message, UBUNTU_RED + BOLD) << "\n";
    }

   inline void asciiLogo() {
    const vector<pair<string, string> > lines = {
        {"        _    __  __ ____       ___  ____  ", UBUNTU_ORANGE},
        {"       / \\  |  \\/  / ___|     / _ \\/ ___| ", UBUNTU_ORANGE},
        {"      / _ \\ | |\\/| \\___ \\    | | | \\___ \\ ", UBUNTU_PURPLE},
        {"     / ___ \\| |  | |___) |   | |_| |___) |", UBUNTU_PURPLE},
        {"    /_/   \\_\\_|  |_|____/     \\___/|____/ ", UBUNTU_ORANGE}
    };

    cout << "\n";

    for (const auto &line : lines) {
        cout << paint(line.first, line.second + BOLD) << "\n";
    }

    cout << "\n";
    cout << paint("    AMS OS Ubuntu Terminal Edition\n", UBUNTU_ORANGE + BOLD);
    cout << paint("    Atomic Management System Simulator\n", UBUNTU_LIGHT);
    cout << "\n";
}
    inline void bootStep(const string &label, const string &message) {
        cout << "  "
             << statusPill(label, UBUNTU_ORANGE)
             << " "
             << paint(message, UBUNTU_LIGHT)
             << "\n";
    }

    inline void commandPrompt(const string &text) {
        cout << paint("\nams", UBUNTU_GREEN + BOLD)
             << paint("@", UBUNTU_LIGHT)
             << paint("ubuntu", UBUNTU_ORANGE + BOLD)
             << paint(":~/AMS_OS$ ", UBUNTU_BLUE + BOLD)
             << paint(text, UBUNTU_LIGHT + BOLD);
    }

    inline void emptyState(const string &message, int width = 78) {
        cout << "  " << paint(message, DIM) << "\n";
        panelFooter(width);
    }

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