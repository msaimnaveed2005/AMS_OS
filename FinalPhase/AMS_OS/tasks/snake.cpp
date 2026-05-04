#include <iostream>
#include <cstdlib>
#include <ctime>

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

    cout << "\n========== SNAKE GAME TASK ==========\n";
    cout << "Simple Snake Simulation\n";
    cout << "Use W A S D to move. Press Q to quit.\n";

    srand(time(0));

    while (true) {
        cout << "\nSnake Position: [*]\n";
        cout << "Score: " << score << "\n";
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
                cout << "Food eaten. Score increased.\n";
            } else {
                cout << "Snake moved.\n";
            }
        } else {
            cout << "Invalid move.\n";
        }
    }

    cout << "Snake Game ended. Final Score: " << score << "\n";
    return 0;
}