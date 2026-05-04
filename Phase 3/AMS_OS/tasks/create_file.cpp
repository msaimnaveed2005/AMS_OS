#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Creates a new file inside the virtual disk.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string fileName;
    string content;

    UI::panelHeader("Create File", "Virtual disk utility");
    UI::taskControlHint(getpid());
    cout << "Enter file name to create: ";
    cin >> fileName;

    string filePath = "data/virtual_disk/" + fileName;

    ofstream file(filePath);

    if (!file) {
        cout << UI::paint("Error: Could not create file.\n", UI::RED + UI::BOLD);
        return 1;
    }

    cin.ignore();

    cout << "Enter content for the file: ";
    getline(cin, content);

    file << content << endl;
    file.close();

    cout << UI::paint("File created successfully.\n", UI::GREEN + UI::BOLD);
    UI::keyValue("Path", filePath);
    UI::panelFooter();

    return 0;
}
