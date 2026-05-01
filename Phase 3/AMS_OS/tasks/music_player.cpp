#include <iostream>
#include <unistd.h>
using namespace std;

/*
Function: main
Purpose: Runs music player simulation as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    cout << "\n========== MUSIC PLAYER TASK ==========\n";
    cout << "Music Player started as separate executable.\n";
    cout << "Playing background beep simulation.\n";

    for (int i = 1; i <= 5; i++) {
        cout << "Beep playing... " << i << "/5\n";
        cout << "\a";
        sleep(1);
    }

    cout << "Music Player task completed.\n";
    return 0;
}