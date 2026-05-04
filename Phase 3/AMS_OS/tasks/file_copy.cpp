#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "../kernel/ui.h"
using namespace std;

/*
Function: main
Purpose: Runs file copy task as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string sourcePath;
    string destinationPath;
    string line;

    UI::panelHeader("File Copy", "Virtual disk utility");
    UI::taskControlHint(getpid());

    cout << "Enter source file path: ";
    cin >> sourcePath;

    cout << "Enter destination file path: ";
    cin >> destinationPath;

    ifstream sourceFile(sourcePath);
    ofstream destinationFile(destinationPath);

    if (!sourceFile) {
        cout << UI::paint("Error: Source file could not be opened.\n", UI::RED + UI::BOLD);
        return 1;
    }

    if (!destinationFile) {
        cout << UI::paint("Error: Destination file could not be created.\n", UI::RED + UI::BOLD);
        return 1;
    }

    while (getline(sourceFile, line)) {
        destinationFile << line << endl;
    }

    sourceFile.close();
    destinationFile.close();

    cout << UI::paint("File copied successfully.\n", UI::GREEN + UI::BOLD);
    UI::keyValue("Source", sourcePath);
    UI::keyValue("Destination", destinationPath);
    UI::panelFooter();

    return 0;
}
