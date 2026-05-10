#include <iostream>
#include <cstdio>
#include <string>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Deletes a file from the virtual disk.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string fileName;

    UI::panelHeader("Delete File", "Virtual disk utility");
    UI::taskControlHint(getpid());
    cout << "Enter file name to delete: ";
    cin >> fileName;

    string filePath = "data/virtual_disk/" + fileName;

    if (remove(filePath.c_str()) == 0) {
        cout << UI::paint("File deleted successfully.\n", UI::GREEN + UI::BOLD);
        UI::keyValue("Path", filePath);
    } else {
        cout << UI::paint("Error: File could not be deleted or does not exist.\n", UI::RED + UI::BOLD);
        return 1;
    }

    UI::panelFooter();
    return 0;
}
