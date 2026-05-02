#include <iostream>
#include <unistd.h>

using namespace std;

/*
Function: main
Purpose: Simulates a file download task running in the background.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    cout << "\n========== FILE DOWNLOAD SIMULATOR TASK ==========\n";
    cout << "Download started.\n";

    for (int progress = 20; progress <= 100; progress += 20) {
        cout << "Downloading... " << progress << "% completed\n";
        sleep(1);
    }

    cout << "Download completed successfully.\n";

    return 0;
}