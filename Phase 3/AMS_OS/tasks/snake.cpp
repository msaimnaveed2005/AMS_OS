#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Runs a simple text-based snake game simulation.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    int score = 0;
    char move;

    UI::panelHeader("Snake Game", "Text simulation");
    UI::taskControlHint(getpid());
    cout << "  Use W A S D to move. Press Q to quit.\n";

    srand(time(0));

    while (true) {
        cout << "\n  " << UI::paint("Snake", UI::BOLD) << "  [*]\n";
        UI::keyValue("Score", to_string(score), 8);
        cout << "Enter move: ";
        cin >> move;

        if (move == 'q' || move == 'Q') {
            break;
        }

        if (
            move == 'w' || move == 'W' ||
            move == 'a' || move == 'A' ||
            move == 's' || move == 'S' ||
            move == 'd' || move == 'D'
        ) {
            int food = rand() % 2;

            if (food == 1) {
                score += 10;
                cout << UI::paint("Food eaten. Score increased.\n", UI::GREEN + UI::BOLD);
            } else {
                cout << "Snake moved.\n";
            }
        } else {
            cout << UI::paint("Invalid move.\n", UI::YELLOW + UI::BOLD);
        }
    }

    cout << UI::paint("Snake Game ended.\n", UI::GREEN + UI::BOLD);
    UI::keyValue("Final Score", to_string(score));
    UI::panelFooter();
    return 0;
}
