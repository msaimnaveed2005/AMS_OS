#include <iostream>
#include <limits>
#include <unistd.h>
#include "../kernel/ui.h"
using namespace std;

/*
Function: main
Purpose: Runs music player simulation as a separate executable loaded through exec.
         Simulates background music playback with visual progress bars and sound cues.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Music Player", "Background task");
    UI::taskControlHint(getpid());

    UI::sectionBanner("Track List", UI::BRIGHT_MAGENTA);
    UI::menuItem(1, "Calm Breeze",   "Ambient  | 8s");
    UI::menuItem(2, "Night Drive",   "Lo-fi    | 10s");
    UI::menuItem(3, "Retro Waves",   "Synthwave| 12s");
    UI::menuItem(4, "Focus Mode",    "Minimal  | 6s");
    UI::menuItem(5, "Sunrise Theme", "Upbeat   | 8s");

    int selectedTrack = 0;
    while (true) {
        cout << "\n  Enter track number (1-5): ";
        cin >> selectedTrack;

        if (!cin.fail() && selectedTrack >= 1 && selectedTrack <= 5) {
            break;
        }

        UI::errorLine("Invalid selection. Please choose a number from 1 to 5.");
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    string trackName;
    int trackLength;
    string genre;

    switch (selectedTrack) {
        case 1: trackName = "Calm Breeze";   trackLength = 8;  genre = "Ambient";   break;
        case 2: trackName = "Night Drive";   trackLength = 10; genre = "Lo-fi";     break;
        case 3: trackName = "Retro Waves";   trackLength = 12; genre = "Synthwave"; break;
        case 4: trackName = "Focus Mode";    trackLength = 6;  genre = "Minimal";   break;
        case 5: trackName = "Sunrise Theme"; trackLength = 8;  genre = "Upbeat";    break;
        default: trackName = "Calm Breeze";  trackLength = 8;  genre = "Ambient";
    }

    UI::sectionBanner("Now Playing", UI::BRIGHT_GREEN);
    UI::keyValue("Track", trackName);
    UI::keyValue("Genre", genre);
    UI::keyValue("Duration", to_string(trackLength) + " seconds");
    cout << "\n";

    for (int i = 1; i <= trackLength; i++) {
        int elapsed = i;
        int remaining = trackLength - i;

        cout << "  " << UI::paint(trackName, UI::WHITE + UI::BOLD)
             << "  " << UI::usageBar(i, trackLength)
             << "  " << UI::paint(to_string(elapsed) + "s/" + to_string(trackLength) + "s", UI::DIM);

        if (remaining > 0) {
            cout << "  " << UI::paint(to_string(remaining) + "s left", UI::YELLOW);
        } else {
            cout << "  " << UI::paint("Finished", UI::GREEN + UI::BOLD);
        }
        cout << "\n";

        /*
        Play a beep sound at the start and every 4 seconds to simulate
        music beats running in the background.
        */
        if (i == 1 || i % 4 == 0) {
            UI::playCue("tick");
        }

        sleep(1);
    }

    cout << "\n";
    UI::successLine("Track playback completed.");
    UI::keyValue("Played", trackName + " (" + to_string(trackLength) + "s)");
    UI::panelFooter();
    return 0;
}
