#include <iostream>
#include <cstdlib>
#include <ctime>

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

    cout << "\n========== MINESWEEPER GAME TASK ==========\n";
    cout << "Simple 3x3 Minesweeper Simulation\n";
    cout << "Enter row and column values between 0 and 2.\n";

    while (attempts > 0) {
        cout << "\nAttempts left: " << attempts << "\n";
        cout << "Enter row: ";
        cin >> row;

        cout << "Enter column: ";
        cin >> col;

        if (row < 0 || row > 2 || col < 0 || col > 2) {
            cout << "Invalid cell.\n";
            continue;
        }

        if (row == mineRow && col == mineCol) {
            cout << "Boom. You hit a mine.\n";
            return 0;
        }

        cout << "Safe cell.\n";
        attempts--;
    }

    cout << "You survived the Minesweeper simulation.\n";
    return 0;
}