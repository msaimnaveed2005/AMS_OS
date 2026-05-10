#include <iostream>
#include <fstream>
#include <string>

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

    cout << "\n========== CREATE FILE TASK ==========\n";
    cout << "Enter file name to create: ";
    cin >> fileName;

    string filePath = "data/virtual_disk/" + fileName;

    ofstream file(filePath);

    if (!file) {
        cout << "Error: Could not create file.\n";
        return 1;
    }

    cin.ignore();

    cout << "Enter content for the file: ";
    getline(cin, content);

    file << content << endl;
    file.close();

    cout << "File created successfully at: " << filePath << "\n";

    return 0;
}