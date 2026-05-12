#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include <string>
#include "../kernel/ui.h"

using namespace std;

struct Download {
    string name;
    int progress;
    int speed;
};

/*
Function: main
Purpose: Simulates a file download task running in the background.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    srand(time(0));
    
    vector<Download> downloads = {
        {"ubuntu-26.04-desktop-amd64.iso", 0, rand() % 5 + 3},
        {"linux-kernel-6.8.tar.xz", 0, rand() % 8 + 4},
        {"ams-os-update-v2.deb", 0, rand() % 12 + 6}
    };

    bool allDone = false;

    while (!allDone) {
        UI::clearScreen();
        UI::panelHeader("Download Simulator", "Background task");
        UI::taskControlHint(getpid());
        cout << "  Active Downloads:\n\n";

        allDone = true;
        for (auto &dl : downloads) {
            dl.progress += dl.speed;
            if (dl.progress > 100) dl.progress = 100;
            if (dl.progress < 100) allDone = false;

            cout << "  " << UI::paint(dl.name, UI::WHITE + UI::BOLD) << "\n";
            cout << "  " << UI::usageBar(dl.progress, 100, 50) << "\n\n";
        }

        usleep(500000); // 0.5s updates for smooth animation
    }

    UI::clearScreen();
    UI::panelHeader("Download Simulator", "Background task");
    for (auto &dl : downloads) {
        cout << "  " << UI::paint(dl.name, UI::WHITE + UI::BOLD) << "\n";
        cout << "  " << UI::usageBar(100, 100, 50) << "\n\n";
    }
    cout << UI::paint("All downloads completed successfully.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();

    return 0;
}
