#include <iostream>
#include <limits>
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include <string>
#include "../kernel/ui.h"
using namespace std;

string getVisualizerBar(int height, int maxHeight) {
    string bar = "";
    for (int i = 0; i < maxHeight; i++) {
        if (i < height) {
            bar += "|";
        } else {
            bar += " ";
        }
    }
    return bar;
}

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
    int trackLength = 20;

    switch (selectedTrack) {
        case 1: trackName = "Calm Breeze"; break;
        case 2: trackName = "Night Drive"; break;
        case 3: trackName = "Retro Waves"; break;
        case 4: trackName = "Focus Mode"; break;
        case 5: trackName = "Sunrise Theme"; break;
        default: trackName = "Calm Breeze";
    }

    srand(time(0));
    const int NUM_BARS = 20;
    const int MAX_HEIGHT = 8;
    vector<int> heights(NUM_BARS, 0);

    for (int i = 1; i <= trackLength; i++) {
        UI::clearScreen();
        UI::panelHeader("Music Player", "Background task");
        UI::taskControlHint(getpid());
        cout << "\n  Now playing: " << UI::paint(trackName, UI::BRIGHT_CYAN + UI::BOLD) << "\n\n";
        
        // Generate random heights for visualizer
        for (int j = 0; j < NUM_BARS; j++) {
            heights[j] = rand() % MAX_HEIGHT + 1;
        }

        // Print visualizer from top to bottom
        for (int row = MAX_HEIGHT; row > 0; row--) {
            cout << "      ";
            for (int j = 0; j < NUM_BARS; j++) {
                if (heights[j] >= row) {
                    if (row > 6) cout << UI::paint("[]", UI::RED + UI::BOLD);
                    else if (row > 3) cout << UI::paint("[]", UI::YELLOW + UI::BOLD);
                    else cout << UI::paint("[]", UI::GREEN + UI::BOLD);
                } else {
                    cout << "  ";
                }
                cout << " ";
            }
            cout << "\n";
        }
        cout << "\n  Progress: " << UI::usageBar(i, trackLength, 40) << "\n";
        
        if (selectedTrack % 2 == 0) {
            cout << "\a";
        } else {
            cout << "\a\a";
        }
        
        usleep(800000); // slightly faster than 1 sec to make it feel more dynamic
    }

    UI::clearScreen();
    UI::panelHeader("Music Player", "Background task");
    cout << UI::paint("\n  Music Player task completed.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}
