#include <iostream>
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
    cout << "  Playing background beep simulation.\n";

    for (int i = 1; i <= 5; i++) {
        cout << "  Beep playing " << UI::usageBar(i, 5) << "\n";
        cout << "\a";
        sleep(1);
    }

    cout << UI::paint("Music Player task completed.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}
