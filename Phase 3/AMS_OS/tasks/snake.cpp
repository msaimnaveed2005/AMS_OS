#include <iostream>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <vector>
#include <utility>
#include <algorithm>
#include <limits>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

static const int GRID_ROWS = 10;
static const int GRID_COLS = 20;

bool isSnakeCell(const deque<pair<int, int> > &snake, int row, int col) {
    for (const auto &segment : snake) {
        if (segment.first == row && segment.second == col) {
            return true;
        }
    }
    return false;
}

pair<int, int> spawnFood(const deque<pair<int, int> > &snake) {
    vector<pair<int, int> > freeCells;

    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            if (!isSnakeCell(snake, row, col)) {
                freeCells.push_back(make_pair(row, col));
            }
        }
    }

    if (freeCells.empty()) {
        return make_pair(-1, -1);
    }

    return freeCells[rand() % freeCells.size()];
}

void renderBoard(const deque<pair<int, int> > &snake, pair<int, int> food, int score) {
    UI::clearScreen();
    UI::panelHeader("Snake Game", "Grid simulation");
    UI::taskControlHint(getpid());
    UI::keyValue("Controls", "W A S D + Enter | Q to quit");
    UI::keyValue("Score", to_string(score));

    cout << "\n";
    cout << "  +" << string(GRID_COLS, '-') << "+\n";

    for (int row = 0; row < GRID_ROWS; row++) {
        cout << "  |";

        for (int col = 0; col < GRID_COLS; col++) {
            if (snake.front().first == row && snake.front().second == col) {
                cout << UI::paint("@", UI::GREEN + UI::BOLD);
            } else if (isSnakeCell(snake, row, col)) {
                cout << UI::paint("o", UI::LIGHT_BLUE + UI::BOLD);
            } else if (food.first == row && food.second == col) {
                cout << UI::paint("*", UI::YELLOW + UI::BOLD);
            } else {
                cout << " ";
            }
        }

        cout << "|\n";
    }

    cout << "  +" << string(GRID_COLS, '-') << "+\n";
}

/*
Function: main
Purpose: Runs a simple text-based snake game simulation.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    srand(static_cast<unsigned int>(time(0)));

    deque<pair<int, int> > snake;
    snake.push_front(make_pair(GRID_ROWS / 2, GRID_COLS / 2));
    snake.push_back(make_pair(GRID_ROWS / 2, GRID_COLS / 2 - 1));
    snake.push_back(make_pair(GRID_ROWS / 2, GRID_COLS / 2 - 2));

    pair<int, int> food = spawnFood(snake);
    char direction = 'D';
    int score = 0;

    while (true) {
        renderBoard(snake, food, score);
        cout << "\n  Enter move: ";

        char move;
        cin >> move;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (move == 'q' || move == 'Q') {
            break;
        }

        move = static_cast<char>(toupper(move));

        if (move == 'W' && direction != 'S') direction = 'W';
        else if (move == 'S' && direction != 'W') direction = 'S';
        else if (move == 'A' && direction != 'D') direction = 'A';
        else if (move == 'D' && direction != 'A') direction = 'D';
        else if (move != 'W' && move != 'A' && move != 'S' && move != 'D') {
            UI::warnLine("Invalid move key.");
            sleep(1);
            continue;
        }

        pair<int, int> nextHead = snake.front();

        if (direction == 'W') nextHead.first--;
        else if (direction == 'S') nextHead.first++;
        else if (direction == 'A') nextHead.second--;
        else if (direction == 'D') nextHead.second++;

        if (
            nextHead.first < 0 || nextHead.first >= GRID_ROWS ||
            nextHead.second < 0 || nextHead.second >= GRID_COLS ||
            isSnakeCell(snake, nextHead.first, nextHead.second)
        ) {
            renderBoard(snake, food, score);
            UI::errorLine("Game over: collision detected.");
            break;
        }

        snake.push_front(nextHead);

        if (nextHead == food) {
            score += 10;
            food = spawnFood(snake);
            UI::successLine("Food eaten. Snake length increased.");

            if (food.first == -1) {
                UI::successLine("You filled the board. Perfect game.");
                break;
            }
        } else {
            snake.pop_back();
        }
    }

    cout << UI::paint("Snake Game ended.\n", UI::GREEN + UI::BOLD);
    UI::keyValue("Final Score", to_string(score));
    UI::panelFooter();
    return 0;
}
