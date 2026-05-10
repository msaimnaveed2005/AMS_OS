#include <iostream>
#include <cstdio>
#include <string>
#include <cerrno>
#include <cstring>
#include <limits>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

bool isValidVirtualDiskFileName(const string &name) {
    return !name.empty() &&
           name.find('/') == string::npos &&
           name.find('\\') == string::npos;
}

/*
Function: main
Purpose: Moves or renames a file inside the virtual disk.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string sourceFile;
    string destinationFile;

    UI::panelHeader("Move File", "Virtual disk utility");
    UI::taskControlHint(getpid());
    UI::infoLine("Move or rename a file inside data/virtual_disk.");

    cout << "  Enter source file name: ";
    cin >> sourceFile;

    cout << "  Enter destination file name: ";
    cin >> destinationFile;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        UI::errorLine("Invalid input.");
        return 1;
    }

    if (!isValidVirtualDiskFileName(sourceFile) || !isValidVirtualDiskFileName(destinationFile)) {
        UI::errorLine("Use file names only (no folder separators).");
        return 1;
    }

    if (sourceFile == destinationFile) {
        UI::warnLine("Source and destination are same. Nothing to move.");
        return 1;
    }

    string sourcePath = "data/virtual_disk/" + sourceFile;
    string destinationPath = "data/virtual_disk/" + destinationFile;

    UI::sectionBanner("Move Operation", UI::BRIGHT_BLUE);
    UI::keyValue("From", sourcePath);
    UI::keyValue("To", destinationPath);

    if (rename(sourcePath.c_str(), destinationPath.c_str()) == 0) {
        UI::successLine("File moved/renamed successfully.");
        UI::playCue("granted");
    } else {
        UI::errorLine("File could not be moved.");
        UI::keyValue("System Error", strerror(errno));
        UI::playCue("error");
        return 1;
    }

    UI::panelFooter();
    return 0;
}
