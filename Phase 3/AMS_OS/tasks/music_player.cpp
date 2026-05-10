#include <iostream>
#include <limits>
#include <unistd.h>
#include "../kernel/ui.h"
using namespace std;

/*
Function: main
Purpose: Runs music player simulation as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Music Player", "Background task");
    UI::taskControlHint(getpid());
    cout << "  Choose a track to play:\n";
    cout << "  1. Calm Breeze\n";
    cout << "  2. Night Drive\n";
    cout << "  3. Retro Waves\n";
    cout << "  4. Focus Mode\n";
    cout << "  5. Sunrise Theme\n\n";

    int selectedTrack = 0;
    while (true) {
        cout << "  Enter track number (1-5): ";
        cin >> selectedTrack;

        if (!cin.fail() && selectedTrack >= 1 && selectedTrack <= 5) {
            break;
        }

        UI::errorLine("Invalid selection. Please choose a number from 1 to 5.");
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    string trackName;
    int trackLength = 5;

    switch (selectedTrack) {
        case 1:
            trackName = "Calm Breeze";
            break;
        case 2:
            trackName = "Night Drive";
            break;
        case 3:
            trackName = "Retro Waves";
            break;
        case 4:
            trackName = "Focus Mode";
            break;
        case 5:
            trackName = "Sunrise Theme";
            break;
        default:
            trackName = "Calm Breeze";
    }

    cout << "\n  Now playing: " << UI::paint(trackName, UI::WHITE + UI::BOLD) << "\n";

    for (int i = 1; i <= trackLength; i++) {
        cout << "  Playing " << UI::usageBar(i, trackLength) << "\n";
        if (selectedTrack % 2 == 0) {
            cout << "\a";
        } else {
            cout << "\a\a";
        }
        sleep(1);
    }

    cout << UI::paint("Music Player task completed.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}
