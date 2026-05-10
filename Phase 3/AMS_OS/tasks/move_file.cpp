#include <iostream>
#include <cstdio>
#include <string>

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

    cout << "\n========== MOVE FILE TASK ==========\n";

    cout << "Enter source file name: ";
    cin >> sourceFile;

    cout << "Enter destination file name: ";
    cin >> destinationFile;

    string sourcePath = "data/virtual_disk/" + sourceFile;
    string destinationPath = "data/virtual_disk/" + destinationFile;

    if (rename(sourcePath.c_str(), destinationPath.c_str()) == 0) {
        cout << "File moved/renamed successfully.\n";
        cout << "From: " << sourcePath << "\n";
        cout << "To: " << destinationPath << "\n";
    } else {
        cout << "Error: File could not be moved. Check if source exists.\n";
        return 1;
    }

    return 0;
}