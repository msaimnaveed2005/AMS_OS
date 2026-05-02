#include <iostream>
#include <ctime>
#include <unistd.h>
using namespace std;

/*
Function: main
Purpose: Runs digital clock task as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    cout << "\n========== DIGITAL CLOCK TASK ==========\n";
    cout << "Digital Clock started as separate executable.\n";

    for (int i = 1; i <= 5; i++) {
        time_t now = time(0);
        char* currentTime = ctime(&now);

        cout << "Current Time: " << currentTime;
        sleep(1);
    }

    cout << "Digital Clock task completed.\n";
    return 0;
}