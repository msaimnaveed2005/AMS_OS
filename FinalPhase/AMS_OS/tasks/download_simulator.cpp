#include <iostream>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Simulates a file download task running in the background.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Download Simulator", "Background task");
    UI::taskControlHint(getpid());
    cout << "  Download started.\n";

    for (int progress = 20; progress <= 100; progress += 20) {
        cout << "  Downloading " << UI::usageBar(progress, 100) << "\n";
        sleep(1);
    }

    cout << UI::paint("Download completed successfully.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();

    return 0;
}
