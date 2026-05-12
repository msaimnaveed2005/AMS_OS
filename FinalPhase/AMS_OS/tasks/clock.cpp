#include <iostream>
#include <ctime>
#include <unistd.h>
#include "../kernel/ui.h"
using namespace std;

/*
Function: main
Purpose: Runs digital clock task as a separate executable loaded through exec.
         Displays a large ASCII-art time display that updates every second.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Digital Clock", "AMS OS task executable");
    UI::taskControlHint(getpid(), true);
    UI::infoLine("Live clock running for 60 ticks. Auto-task will finish automatically.");

    for (int i = 1; i <= 60; i++) {
        time_t now = time(0);
        tm *currentTime = localtime(&now);
        char timeFormatted[32];
        char dateFormatted[64];
        strftime(timeFormatted, sizeof(timeFormatted), "%H:%M:%S", currentTime);
        strftime(dateFormatted, sizeof(dateFormatted), "%A, %B %d, %Y", currentTime);

        string timeStr(timeFormatted);

        cout << "\r  "
             << UI::paint("[", UI::DIM)
             << UI::paint(timeStr, UI::WHITE + UI::BOLD)
             << UI::paint("]", UI::DIM)
             << "  "
             << UI::paint(dateFormatted, UI::LIGHT_BLUE)
             << "  "
             << UI::paint("tick " + to_string(i) + "/60", UI::YELLOW)
             << "      ";
        cout.flush();

        sleep(1);
    }

    cout << "\n\n";
    UI::successLine("Digital Clock task completed (60 ticks).");
    UI::panelFooter();
    return 0;
}
