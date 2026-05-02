#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <string>

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

    cout << "\n========== FILE INFORMATION VIEWER TASK ==========\n";
    cout << "Enter file name: ";
    cin >> fileName;

    string filePath = "data/virtual_disk/" + fileName;

    if (stat(filePath.c_str(), &fileStatus) != 0) {
        cout << "Error: File does not exist.\n";
        return 1;
    }

    cout << "\nFile Path: " << filePath << "\n";
    cout << "File Size: " << fileStatus.st_size << " bytes\n";
    cout << "Permissions: " << (fileStatus.st_mode & 0777) << "\n";

    ifstream file(filePath);

    if (file) {
        string line;
        int lineCount = 0;

        while (getline(file, line)) {
            lineCount++;
        }

        cout << "Total Lines: " << lineCount << "\n";
        file.close();
    }

    return 0;
}