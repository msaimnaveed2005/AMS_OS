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
    UI::infoLine("Tick mode enabled for demo visibility.");

    for (int i = 1; i <= 12; i++) {
        time_t now = time(0);
        tm *currentTime = localtime(&now);
        char formatted[32];
        strftime(formatted, sizeof(formatted), "%H:%M:%S", currentTime);

        cout << "  " << UI::paint("Current Time: ", UI::BOLD)
             << UI::paint(formatted, UI::WHITE + UI::BOLD)
             << "  "
             << UI::paint("tick tick", UI::YELLOW + UI::BOLD)
             << "\n";
        UI::playCue("tick");
        sleep(1);
    }

    cout << UI::paint("Digital Clock task completed.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}
