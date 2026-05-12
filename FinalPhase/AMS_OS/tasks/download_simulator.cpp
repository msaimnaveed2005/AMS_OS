#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Simulates a file download task running in the background with
         animated progress bar, transfer speed, and file metadata.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    srand(static_cast<unsigned int>(time(0)));

    UI::panelHeader("Download Simulator", "Background task");
    UI::taskControlHint(getpid());

    string fileNames[] = {
        "ubuntu-22.04-desktop.iso",
        "kernel-headers-6.8.tar.gz",
        "vscode-linux-x64.deb",
        "gcc-13.2-source.tar.xz",
        "node-v20-linux-x64.tar.gz"
    };

    int fileSizes[] = { 3800, 1200, 580, 2400, 420 };
    int fileIndex = rand() % 5;

    string fileName = fileNames[fileIndex];
    int totalSizeMB = fileSizes[fileIndex];

    UI::sectionBanner("Download Details", UI::BRIGHT_BLUE);
    UI::keyValue("File", fileName);
    UI::keyValue("Size", to_string(totalSizeMB) + " MB");
    UI::keyValue("Server", "mirror.ams-os.local");
    cout << "\n";

    int steps = 10;
    int chunkSize = totalSizeMB / steps;

    for (int i = 1; i <= steps; i++) {
        int downloaded = chunkSize * i;
        if (i == steps) downloaded = totalSizeMB;

        int speed = 40 + (rand() % 80);

        cout << "  " << UI::usageBar(i, steps)
             << "  " << UI::paint(to_string(downloaded) + "/" + to_string(totalSizeMB) + " MB", UI::WHITE + UI::BOLD)
             << "  " << UI::paint(to_string(speed) + " MB/s", UI::LIGHT_BLUE)
             << "\n";

        sleep(1);
    }

    cout << "\n";
    UI::playCue("granted");
    UI::successLine("Download completed successfully.");
    UI::keyValue("Saved to", "data/virtual_disk/" + fileName);
    UI::keyValue("Total Size", to_string(totalSizeMB) + " MB");
    UI::panelFooter();

    return 0;
}
