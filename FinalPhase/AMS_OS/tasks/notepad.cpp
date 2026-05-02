#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

/*
Function: main
Purpose: Runs notepad task as a separate executable with auto-save.
         Every line typed by the user is immediately written and flushed to disk.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    string line;
    string fileName;

    cout << "\n========== NOTEPAD WITH AUTO-SAVE TASK ==========\n";
    cout << "Notepad started as a separate executable.\n";
    cout << "Every line you type will be auto-saved immediately.\n";
    cout << "Type EXIT to close Notepad.\n\n";

    cout << "Enter file name for notes: ";
    cin >> fileName;

    string filePath = "data/virtual_disk/" + fileName;

    ofstream file(filePath, ios::app);

    if (!file) {
        cout << "Error: Could not open or create notepad file.\n";
        return 1;
    }

    cin.ignore();

    cout << "\nStart writing below:\n";

    while (true) {
        cout << "> ";
        getline(cin, line);

        if (line == "EXIT") {
            break;
        }

        file << line << endl;

        /*
        Auto-save happens here.
        flush() forces the data to be written immediately instead of waiting
        for the file to close.
        */
        file.flush();

        cout << "[AUTO-SAVE] Line saved successfully.\n";
    }

    file.close();

    cout << "\nNotepad closed.\n";
    cout << "File saved at: " << filePath << "\n";

    return 0;
}