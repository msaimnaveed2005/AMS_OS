#include <iostream>
#include <ctime>
#include <unistd.h>
#include "../kernel/ui.h"
using namespace std;

/*
Function: main
Purpose: Runs digital clock task as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Digital Clock", "AMS OS task executable");
    UI::taskControlHint(getpid(), true);

    for (int i = 1; i <= 5; i++) {
        time_t now = time(0);
        char* currentTime = ctime(&now);

        cout << "  " << UI::paint("Current Time: ", UI::BOLD) << currentTime;
        sleep(1);
    }

    cout << UI::paint("Digital Clock task completed.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}
