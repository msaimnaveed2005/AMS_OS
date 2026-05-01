#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/*
Function: main
Purpose: Runs notepad task as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string line;
    ofstream file;

    cout << "\n========== NOTEPAD TASK ==========\n";
    cout << "Notepad started as separate executable.\n";
    cout << "Write your text below.\n";
    cout << "Type SAVE to save and exit.\n\n";

    file.open("data/virtual_disk/notepad.txt", ios::app);

    if (!file) {
        cout << "Error: Could not open notepad file.\n";
        return 1;
    }

    cin.ignore();

    while (true) {
        getline(cin, line);

        if (line == "SAVE") {
            break;
        }

        file << line << endl;
    }

    file.close();

    cout << "Notepad content saved to data/virtual_disk/notepad.txt\n";
    cout << "Notepad task completed.\n";

    return 0;
}