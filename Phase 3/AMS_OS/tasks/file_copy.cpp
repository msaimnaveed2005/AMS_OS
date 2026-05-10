#include <iostream>
#include <fstream>
#include <string>
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

    cout << "\n========== FILE COPY TASK ==========\n";
    cout << "File Copy started as separate executable.\n";

    cout << "Enter source file path: ";
    cin >> sourcePath;

    cout << "Enter destination file path: ";
    cin >> destinationPath;

    ifstream sourceFile(sourcePath);
    ofstream destinationFile(destinationPath);

    if (!sourceFile) {
        cout << "Error: Source file could not be opened.\n";
        return 1;
    }

    if (!destinationFile) {
        cout << "Error: Destination file could not be created.\n";
        return 1;
    }

    while (getline(sourceFile, line)) {
        destinationFile << line << endl;
    }

    sourceFile.close();
    destinationFile.close();

    cout << "File copied successfully.\n";
    cout << "File Copy task completed.\n";

    return 0;
}