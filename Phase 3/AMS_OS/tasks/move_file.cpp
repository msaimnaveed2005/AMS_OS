#include <iostream>
#include <cstdio>
#include <string>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

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

    cout << "Enter source file name: ";
    cin >> sourceFile;

    cout << "Enter destination file name: ";
    cin >> destinationFile;

    string sourcePath = "data/virtual_disk/" + sourceFile;
    string destinationPath = "data/virtual_disk/" + destinationFile;

    if (rename(sourcePath.c_str(), destinationPath.c_str()) == 0) {
        cout << UI::paint("File moved/renamed successfully.\n", UI::GREEN + UI::BOLD);
        UI::keyValue("From", sourcePath);
        UI::keyValue("To", destinationPath);
    } else {
        cout << UI::paint("Error: File could not be moved. Check if source exists.\n", UI::RED + UI::BOLD);
        return 1;
    }

    UI::panelFooter();
    return 0;
}
