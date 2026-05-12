#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <limits>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

static const int GRID_SIZE = 5;
static const int MINE_COUNT = 4;

/*
Function: main
Purpose: Runs a text-based minesweeper simulation with a 5x5 grid and
         4 hidden mines. Uses ANSI colors for visual feedback.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    srand(static_cast<unsigned int>(time(0)));

    vector<vector<int> > mines(GRID_SIZE, vector<int>(GRID_SIZE, 0));
    vector<vector<char> > board(GRID_SIZE, vector<char>(GRID_SIZE, '.'));

    int placed = 0;
    while (placed < MINE_COUNT) {
        int r = rand() % GRID_SIZE;
        int c = rand() % GRID_SIZE;
        if (mines[r][c] == 0) {
            mines[r][c] = 1;
            placed++;
        }
    }

    int safeCells = GRID_SIZE * GRID_SIZE - MINE_COUNT;
    int revealed = 0;
    bool alive = true;

    UI::panelHeader("Minesweeper", "5x5 grid simulation");
    UI::taskControlHint(getpid());
    UI::keyValue("Grid Size", to_string(GRID_SIZE) + " x " + to_string(GRID_SIZE));
    UI::keyValue("Hidden Mines", to_string(MINE_COUNT));
    UI::keyValue("Safe Cells", to_string(safeCells));
    UI::infoLine("Enter row and column (0 to " + to_string(GRID_SIZE - 1) + ") to reveal a cell.");

    while (alive && revealed < safeCells) {
        cout << "\n";

        /* Column headers */
        cout << "      ";
        for (int c = 0; c < GRID_SIZE; c++) {
            cout << UI::paint(" " + to_string(c) + " ", UI::LIGHT_BLUE + UI::BOLD) << " ";
        }
        cout << "\n";
        cout << "      " << UI::paint(string(GRID_SIZE * 4, '-'), UI::DIM) << "\n";

        for (int r = 0; r < GRID_SIZE; r++) {
            cout << "  " << UI::paint(to_string(r), UI::LIGHT_BLUE + UI::BOLD) << " | ";
            for (int c = 0; c < GRID_SIZE; c++) {
                char cell = board[r][c];
                if (cell == '.') {
                    cout << UI::paint(" . ", UI::SLATE_GRAY);
                } else if (cell == 'X') {
                    cout << UI::paint(" X ", UI::RED + UI::BOLD);
                } else if (cell == '0') {
                    cout << UI::paint(" 0 ", UI::DIM);
                } else {
                    cout << UI::paint(" " + string(1, cell) + " ", UI::YELLOW + UI::BOLD);
                }
                cout << " ";
            }
            cout << "\n";
        }

        UI::keyValue("Revealed", to_string(revealed) + "/" + to_string(safeCells));

        int row, col;
        cout << "\n  Enter row: ";
        cin >> row;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            UI::errorLine("Invalid input.");
            continue;
        }
        cout << "  Enter col: ";
        cin >> col;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            UI::errorLine("Invalid input.");
            continue;
        }

        if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
            UI::warnLine("Cell out of bounds. Try again.");
            continue;
        }

        if (board[row][col] != '.') {
            UI::warnLine("Cell already revealed. Choose another.");
            continue;
        }

        if (mines[row][col] == 1) {
            board[row][col] = 'X';
            alive = false;

            /* Reveal all mines */
            for (int r = 0; r < GRID_SIZE; r++) {
                for (int c = 0; c < GRID_SIZE; c++) {
                    if (mines[r][c] == 1) board[r][c] = 'X';
                }
            }

            cout << "\n";
            cout << "      ";
            for (int c = 0; c < GRID_SIZE; c++) {
                cout << UI::paint(" " + to_string(c) + " ", UI::LIGHT_BLUE + UI::BOLD) << " ";
            }
            cout << "\n";
            cout << "      " << UI::paint(string(GRID_SIZE * 4, '-'), UI::DIM) << "\n";
            for (int r = 0; r < GRID_SIZE; r++) {
                cout << "  " << UI::paint(to_string(r), UI::LIGHT_BLUE + UI::BOLD) << " | ";
                for (int c = 0; c < GRID_SIZE; c++) {
                    char cell = board[r][c];
                    if (cell == 'X') {
                        cout << UI::paint(" X ", UI::RED + UI::BOLD);
                    } else if (cell == '.') {
                        cout << UI::paint(" . ", UI::SLATE_GRAY);
                    } else {
                        cout << UI::paint(" " + string(1, cell) + " ", UI::YELLOW + UI::BOLD);
                    }
                    cout << " ";
                }
                cout << "\n";
            }

            UI::errorLine("BOOM! You hit a mine. Game Over.");
            UI::playCue("error");
        } else {
            /* Count adjacent mines */
            int adjacent = 0;
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    int nr = row + dr;
                    int nc = col + dc;
                    if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                        adjacent += mines[nr][nc];
                    }
                }
            }
            board[row][col] = static_cast<char>('0' + adjacent);
            revealed++;
            UI::successLine("Safe! Adjacent mines: " + to_string(adjacent));
            UI::playCue("tick");
        }
    }

    if (alive && revealed >= safeCells) {
        UI::successLine("Congratulations! You cleared all safe cells!");
        UI::playCue("granted");
    }

    cout << "\n";
    UI::keyValue("Cells Revealed", to_string(revealed) + "/" + to_string(safeCells));
    UI::panelFooter();
    return 0;
}
