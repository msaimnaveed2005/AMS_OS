#include <iostream>
#include <fstream>
#include <string>
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
Purpose: Runs file copy task as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string sourceFileName;
    string destinationFileName;
    string line;

    UI::panelHeader("File Copy", "Virtual disk utility");
    UI::taskControlHint(getpid());
    UI::infoLine("Copy one file to another inside data/virtual_disk.");

    cout << "  Enter source file name: ";
    cin >> sourceFileName;

    cout << "  Enter destination file name: ";
    cin >> destinationFileName;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        UI::errorLine("Invalid input.");
        return 1;
    }

    if (!isValidVirtualDiskFileName(sourceFileName) || !isValidVirtualDiskFileName(destinationFileName)) {
        UI::errorLine("Use file names only (no folder separators).");
        return 1;
    }

    if (sourceFileName == destinationFileName) {
        UI::warnLine("Source and destination are same. Nothing to copy.");
        return 1;
    }

    string sourcePath = "data/virtual_disk/" + sourceFileName;
    string destinationPath = "data/virtual_disk/" + destinationFileName;

    ifstream sourceFile(sourcePath);
    ofstream destinationFile(destinationPath);

    if (!sourceFile) {
        UI::errorLine("Source file could not be opened.");
        return 1;
    }

    if (!destinationFile) {
        UI::errorLine("Destination file could not be created.");
        return 1;
    }

    int copiedLines = 0;
    UI::sectionBanner("Copy Progress", UI::BRIGHT_BLUE);

    while (getline(sourceFile, line)) {
        destinationFile << line << endl;
        copiedLines++;

        if (copiedLines % 10 == 0) {
            UI::infoLine("Copied " + to_string(copiedLines) + " lines...");
        }
    }

    sourceFile.close();
    destinationFile.close();

    UI::successLine("File copied successfully.");
    UI::keyValue("Source", sourcePath);
    UI::keyValue("Destination", destinationPath);
    UI::keyValue("Lines Copied", to_string(copiedLines));
    UI::playCue("granted");
    UI::panelFooter();

    return 0;
}
