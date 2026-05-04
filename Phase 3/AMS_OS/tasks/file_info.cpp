#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Displays basic information about a file in the virtual disk.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string fileName;
    struct stat fileStatus;

    UI::panelHeader("File Information", "Virtual disk utility");
    UI::taskControlHint(getpid());
    cout << "Enter file name: ";
    cin >> fileName;

    string filePath = "data/virtual_disk/" + fileName;

    if (stat(filePath.c_str(), &fileStatus) != 0) {
        cout << UI::paint("Error: File does not exist.\n", UI::RED + UI::BOLD);
        return 1;
    }

    UI::sectionTitle("File Summary");
    UI::keyValue("File Path", filePath);
    UI::keyValue("File Size", to_string(fileStatus.st_size) + " bytes");
    UI::keyValue("Permissions", to_string(fileStatus.st_mode & 0777));

    ifstream file(filePath);

    if (file) {
        string line;
        int lineCount = 0;

        while (getline(file, line)) {
            lineCount++;
        }

        UI::keyValue("Total Lines", to_string(lineCount));
        file.close();
    }

    UI::panelFooter();
    return 0;
}
