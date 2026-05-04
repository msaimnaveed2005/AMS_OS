#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Runs a simple text-based minesweeper simulation.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    int mineRow;
    int mineCol;
    int row;
    int col;
    int attempts = 3;

    srand(time(0));

    mineRow = rand() % 3;
    mineCol = rand() % 3;

    UI::panelHeader("Minesweeper", "3x3 text simulation");
    UI::taskControlHint(getpid());
    cout << "  Enter row and column values between 0 and 2.\n";

    while (attempts > 0) {
        cout << "\n  0   1   2\n";
        cout << "  ? | ? | ?\n";
        cout << "  ? | ? | ?\n";
        cout << "  ? | ? | ?\n";
        UI::keyValue("Attempts left", to_string(attempts));
        cout << "Enter row: ";
        cin >> row;

        cout << "Enter column: ";
        cin >> col;

        if (row < 0 || row > 2 || col < 0 || col > 2) {
            cout << UI::paint("Invalid cell.\n", UI::YELLOW + UI::BOLD);
            continue;
        }

        if (row == mineRow && col == mineCol) {
            cout << UI::paint("Boom. You hit a mine.\n", UI::RED + UI::BOLD);
            UI::panelFooter();
            return 0;
        }

        cout << UI::paint("Safe cell.\n", UI::GREEN + UI::BOLD);
        attempts--;
    }

    cout << UI::paint("You survived the Minesweeper simulation.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}
