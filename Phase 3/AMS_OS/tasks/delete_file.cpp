#include <iostream>
#include <cstdio>
#include <string>

using namespace std;

/*
Function: main
Purpose: Deletes a file from the virtual disk.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string fileName;

    cout << "\n========== DELETE FILE TASK ==========\n";
    cout << "Enter file name to delete: ";
    cin >> fileName;

    string filePath = "data/virtual_disk/" + fileName;

    if (remove(filePath.c_str()) == 0) {
        cout << "File deleted successfully: " << filePath << "\n";
    } else {
        cout << "Error: File could not be deleted or does not exist.\n";
        return 1;
    }

    return 0;
}