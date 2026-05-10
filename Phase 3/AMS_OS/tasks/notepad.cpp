#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Runs notepad task as a separate executable with auto-save.
         Every line typed by the user is immediately written and flushed to disk.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string line;
    string fileName;

    UI::panelHeader("Notepad", "Auto-save editor");
    UI::taskControlHint(getpid());
    cout << "  Every line you type is auto-saved immediately.\n";
    cout << "  Commands: /help  /status  /exit\n\n";

    cout << "  Enter file name for notes (e.g. notes.txt): ";
    cin >> fileName;

    if (cin.fail() || fileName.empty()) {
        UI::errorLine("Invalid file name.");
        return 1;
    }

    if (fileName.find('/') != string::npos || fileName.find('\\') != string::npos) {
        UI::errorLine("Use file name only, not a path.");
        return 1;
    }

    string filePath = "data/virtual_disk/" + fileName;

    ofstream file(filePath, ios::app);

    if (!file) {
        cout << UI::paint("Error: Could not open or create notepad file.\n", UI::RED + UI::BOLD);
        return 1;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    int lineCount = 0;
    UI::sectionBanner("Editor", UI::BRIGHT_BLUE);
    cout << "  Start writing below:\n";
    cout << "  > ";

    while (true) {
        getline(cin, line);

        if (line == "/exit" || line == "EXIT") {
            break;
        }

        if (line == "/help") {
            UI::infoLine("Commands: /help shows commands, /status shows save info, /exit closes editor.");
            cout << "  > ";
            continue;
        }

        if (line == "/status") {
            UI::keyValue("Current file", filePath);
            UI::keyValue("Saved lines", to_string(lineCount));
            cout << "  > ";
            continue;
        }

        if (line.empty()) {
            UI::warnLine("Empty line skipped.");
            cout << "  > ";
            continue;
        }

        file << line << endl;

        /*
        Auto-save happens here.
        flush() forces the data to be written immediately instead of waiting
        for the file to close.
        */
        file.flush();
        lineCount++;

        UI::successLine("Auto-save complete.");
        UI::playCue("tick");
        cout << "  > ";
    }

    file.close();

    cout << "\n";
    UI::successLine("Notepad closed.");
    UI::keyValue("File saved at", filePath);
    UI::keyValue("Total lines saved", to_string(lineCount));
    UI::panelFooter();

    return 0;
}
