#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

const int GRID_SIZE = 5;
const int NUM_MINES = 4;

int countAdjacentMines(const vector<vector<bool>> &mines, int r, int c) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            int nr = r + i, nc = c + j;
            if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                if (mines[nr][nc]) count++;
            }
        }
    }
    return count;
}

/*
Function: main
Purpose: Runs a simple text-based minesweeper simulation.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    int row, col;
    int attempts = 5;
    vector<vector<char>> board(GRID_SIZE, vector<char>(GRID_SIZE, '?'));
    vector<vector<bool>> mines(GRID_SIZE, vector<bool>(GRID_SIZE, false));

    srand(time(0));

    // Place mines randomly
    int placed = 0;
    while (placed < NUM_MINES) {
        int r = rand() % GRID_SIZE;
        int c = rand() % GRID_SIZE;
        if (!mines[r][c]) {
            mines[r][c] = true;
            placed++;
        }
    }

    UI::panelHeader("Minesweeper", "5x5 text simulation");
    UI::taskControlHint(getpid());
    cout << "  Enter row and column values between 0 and " << (GRID_SIZE - 1) << ".\n";
    cout << "  Clear " << attempts << " safe cells to win.\n";

    while (attempts > 0) {
        cout << "\n    0   1   2   3   4\n";
        cout << "  +" << string(GRID_SIZE * 4 - 1, '-') << "+\n";
        for (int r = 0; r < GRID_SIZE; r++) {
            cout << r << " | ";
            for (int c = 0; c < GRID_SIZE; c++) {
                if (board[r][c] == '?') cout << board[r][c] << " | ";
                else if (board[r][c] == '0') cout << UI::paint(" ", UI::DIM) << " | ";
                else cout << UI::paint(string(1, board[r][c]), UI::BRIGHT_CYAN + UI::BOLD) << " | ";
            }
            cout << "\n  +" << string(GRID_SIZE * 4 - 1, '-') << "+\n";
        }
        
        UI::keyValue("Remaining safe cells needed", to_string(attempts));
        cout << "Enter row: ";
        cin >> row;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        cout << "Enter column: ";
        cin >> col;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
            cout << UI::paint("Invalid cell.\n", UI::YELLOW + UI::BOLD);
            continue;
        }

        if (board[row][col] != '?') {
            cout << UI::paint("Cell already revealed.\n", UI::YELLOW + UI::BOLD);
            continue;
        }

        if (mines[row][col]) {
            board[row][col] = 'X';
            
            // Reveal board
            cout << "\n    0   1   2   3   4\n";
            cout << "  +" << string(GRID_SIZE * 4 - 1, '-') << "+\n";
            for (int r = 0; r < GRID_SIZE; r++) {
                cout << r << " | ";
                for (int c = 0; c < GRID_SIZE; c++) {
                    if (mines[r][c]) cout << UI::paint("X", UI::RED + UI::BOLD) << " | ";
                    else if (board[r][c] != '?') cout << board[r][c] << " | ";
                    else cout << "? | ";
                }
                cout << "\n  +" << string(GRID_SIZE * 4 - 1, '-') << "+\n";
            }
            
            cout << "\n" << UI::paint("Boom! You hit a mine. Game Over.\n", UI::RED + UI::BOLD);
            UI::panelFooter();
            return 0;
        }

        int adj = countAdjacentMines(mines, row, col);
        board[row][col] = '0' + adj;
        cout << UI::paint("Safe cell.\n", UI::GREEN + UI::BOLD);
        attempts--;
    }

    cout << UI::paint("\nCongratulations! You survived the Minesweeper simulation.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}
