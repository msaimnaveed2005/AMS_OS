/*
    SNAKE GAME IN C++ FOR UBUNTU / LINUX TERMINAL

    Refined from Windows version:
    - Removed windows.h
    - Removed conio.h
    - Replaced Sleep() with this_thread::sleep_for()
    - Replaced system("cls") with ANSI clear screen
    - Replaced _kbhit() and _getch() with Linux terminal input using termios
    - Added better score sorting
    - Added smoother terminal rendering

    Controls:
    - Arrow keys or W A S D to move
    - ESC to exit during game
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <limits>

using namespace std;

// Game size
const int width = 30;
const int height = 20;

// Game variables
int X, Y, fruitX, fruitY;
int score, highestScore;
bool gameover = false;
string playerName;

vector<pair<int, int>> tail;

enum Direction {
    STOP = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

Direction dir = STOP;

chrono::steady_clock::time_point startTime;

// Terminal settings
termios originalTerminal;

// ANSI colors
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string PURPLE = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";
const string BOLD = "\033[1m";

// Function prototypes
void enableRawMode();
void disableRawMode();
bool keyPressed();
int getKey();

void clearScreen();
void moveCursor(int row, int col);
void hideCursor();
void showCursor();

void setup();
void draw();
void input();
void logic();
void updateDifficulty();

void loadHighestScore();
void saveHighestScore();
void saveScoreToFile(const string& name, int score);
void showTopScores();

void mainMenu();
void startGame();
void gameOverMenu();
void waitForEnter();

bool fruitOnSnake(int fx, int fy);

// Terminal raw mode
void enableRawMode() {
    tcgetattr(STDIN_FILENO, &originalTerminal);

    termios raw = originalTerminal;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTerminal);
}

// Check keyboard input
bool keyPressed() {
    timeval tv{};
    fd_set readfds;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv) > 0;
}

// Read keyboard input
int getKey() {
    char ch;

    if (read(STDIN_FILENO, &ch, 1) <= 0) {
        return 0;
    }

    // Arrow keys start with ESC [ A/B/C/D
    if (ch == 27) {
        char seq[2];

        this_thread::sleep_for(chrono::milliseconds(2));

        if (read(STDIN_FILENO, &seq[0], 1) <= 0) {
            return 27;
        }

        if (read(STDIN_FILENO, &seq[1], 1) <= 0) {
            return 27;
        }

        if (seq[0] == '[') {
            switch (seq[1]) {
            case 'A':
                return UP;
            case 'B':
                return DOWN;
            case 'C':
                return RIGHT;
            case 'D':
                return LEFT;
            }
        }

        return 27;
    }

    // WASD support
    if (ch == 'w' || ch == 'W') return UP;
    if (ch == 's' || ch == 'S') return DOWN;
    if (ch == 'a' || ch == 'A') return LEFT;
    if (ch == 'd' || ch == 'D') return RIGHT;

    return ch;
}

// Terminal utilities
void clearScreen() {
    cout << "\033[2J\033[H";
}

void moveCursor(int row, int col) {
    cout << "\033[" << row << ";" << col << "H";
}

void hideCursor() {
    cout << "\033[?25l";
}

void showCursor() {
    cout << "\033[?25h";
}

// Check fruit collision with snake body
bool fruitOnSnake(int fx, int fy) {
    for (auto segment : tail) {
        if (segment.first == fx && segment.second == fy) {
            return true;
        }
    }
    return false;
}

// Game setup
void setup() {
    X = width / 2;
    Y = height / 2;

    tail.clear();

    do {
        fruitX = rand() % width;
        fruitY = rand() % height;
    } while ((fruitX == X && fruitY == Y) || fruitOnSnake(fruitX, fruitY));

    score = 0;
    dir = STOP;
    gameover = false;

    startTime = chrono::steady_clock::now();
    loadHighestScore();
}

// Draw game screen
void draw() {
    moveCursor(1, 1);

    cout << PURPLE << BOLD;
    for (int i = 0; i < width + 2; i++) cout << "#";
    cout << RESET << "\n";

    for (int i = 0; i < height; i++) {
        cout << PURPLE << BOLD << "#" << RESET;

        for (int j = 0; j < width; j++) {
            if (i == Y && j == X) {
                cout << YELLOW << BOLD << "O" << RESET;
            }
            else if (i == fruitY && j == fruitX) {
                cout << RED << BOLD << "F" << RESET;
            }
            else {
                bool printedTail = false;

                for (auto segment : tail) {
                    if (segment.first == j && segment.second == i) {
                        cout << GREEN << "o" << RESET;
                        printedTail = true;
                        break;
                    }
                }

                if (!printedTail) {
                    cout << " ";
                }
            }
        }

        cout << PURPLE << BOLD << "#" << RESET << "\n";
    }

    cout << PURPLE << BOLD;
    for (int i = 0; i < width + 2; i++) cout << "#";
    cout << RESET << "\n";

    auto currentTime = chrono::steady_clock::now();
    int elapsedSeconds = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();

    cout << CYAN << BOLD;
    cout << "Player: " << playerName
         << " | Score: " << score
         << " | Highest Score: " << highestScore
         << " | Time: " << elapsedSeconds << "s";
    cout << RESET << "\n";

    cout << WHITE << "Controls: Arrow Keys / W A S D | ESC to quit" << RESET << "\n";
}

// Input handling
void input() {
    if (keyPressed()) {
        int key = getKey();

        switch (key) {
        case UP:
            if (dir != DOWN) dir = UP;
            break;

        case DOWN:
            if (dir != UP) dir = DOWN;
            break;

        case LEFT:
            if (dir != RIGHT) dir = LEFT;
            break;

        case RIGHT:
            if (dir != LEFT) dir = RIGHT;
            break;

        case 27:
            gameover = true;
            break;
        }
    }
}

// Game logic
void logic() {
    if (dir == STOP) {
        return;
    }

    int previousX = X;
    int previousY = Y;

    switch (dir) {
    case UP:
        Y--;
        break;

    case DOWN:
        Y++;
        break;

    case LEFT:
        X--;
        break;

    case RIGHT:
        X++;
        break;

    default:
        break;
    }

    // Wall collision
    if (X < 0 || X >= width || Y < 0 || Y >= height) {
        gameover = true;
        return;
    }

    // Self collision
    for (auto segment : tail) {
        if (segment.first == X && segment.second == Y) {
            gameover = true;
            return;
        }
    }

    // Move tail
    if (!tail.empty()) {
        for (int i = tail.size() - 1; i > 0; i--) {
            tail[i] = tail[i - 1];
        }

        tail[0] = { previousX, previousY };
    }

    // Fruit collection
    if (X == fruitX && Y == fruitY) {
        score += 10;

        tail.push_back({ previousX, previousY });

        do {
            fruitX = rand() % width;
            fruitY = rand() % height;
        } while ((fruitX == X && fruitY == Y) || fruitOnSnake(fruitX, fruitY));

        if (score > highestScore) {
            highestScore = score;
            saveHighestScore();
        }
    }
}

// Difficulty adjustment
void updateDifficulty() {
    int delay = 180 - (score / 10) * 8;

    if (delay < 50) {
        delay = 50;
    }

    this_thread::sleep_for(chrono::milliseconds(delay));
}

// Load highest score
void loadHighestScore() {
    ifstream file("highestscore.txt");

    if (file.is_open()) {
        file >> highestScore;
        file.close();
    }
    else {
        highestScore = 0;
    }
}

// Save highest score
void saveHighestScore() {
    ofstream file("highestscore.txt");

    if (file.is_open()) {
        file << highestScore;
        file.close();
    }
}

// Save score
void saveScoreToFile(const string& name, int score) {
    ofstream file("scores.txt", ios::app);

    if (file.is_open()) {
        file << score << " " << name << "\n";
        file.close();
    }
}

// Show top scores
void showTopScores() {
    clearScreen();

    ifstream file("scores.txt");
    vector<pair<int, string>> scores;

    int tempScore;
    string tempName;

    while (file >> tempScore) {
        getline(file, tempName);

        if (!tempName.empty() && tempName[0] == ' ') {
            tempName.erase(0, 1);
        }

        scores.push_back({ tempScore, tempName });
    }

    file.close();

    sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    cout << CYAN << BOLD;
    cout << "==============================\n";
    cout << "          TOP SCORES          \n";
    cout << "==============================\n";
    cout << RESET;

    if (scores.empty()) {
        cout << "No scores saved yet.\n";
    }
    else {
        int limit = min(5, (int)scores.size());

        for (int i = 0; i < limit; i++) {
            cout << i + 1 << ". " << scores[i].second
                 << " - " << scores[i].first << "\n";
        }
    }

    cout << CYAN << "==============================\n" << RESET;

    waitForEnter();
}

// Wait for user
void waitForEnter() {
    cout << "\nPress Enter to continue...";
    cin.get();
}

// Game over menu
void gameOverMenu() {
    int choice;

    while (true) {
        clearScreen();

        cout << RED << BOLD;
        cout << "==============================\n";
        cout << "           GAME OVER          \n";
        cout << "==============================\n";
        cout << RESET;

        cout << "Final Score: " << score << "\n\n";

        cout << "1. Retry\n";
        cout << "2. Main Menu\n";
        cout << "3. Exit\n\n";

        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            startGame();
            return;
        }
        else if (choice == 2) {
            return;
        }
        else if (choice == 3) {
            clearScreen();
            exit(0);
        }
        else {
            cout << "Invalid choice.\n";
            waitForEnter();
        }
    }
}

// Start game
void startGame() {
    setup();

    clearScreen();
    hideCursor();
    enableRawMode();

    while (!gameover) {
        draw();
        input();
        logic();
        updateDifficulty();
    }

    disableRawMode();
    showCursor();

    saveScoreToFile(playerName, score);
    gameOverMenu();
}

// Main menu
void mainMenu() {
    int choice;

    while (true) {
        clearScreen();

        cout << YELLOW << BOLD;
        cout << "==============================\n";
        cout << "          SNAKE GAME          \n";
        cout << "==============================\n";
        cout << RESET;

        cout << "1. Enter Player Name\n";
        cout << "2. View Top Scores\n";
        cout << "3. Start Game\n";
        cout << "4. Exit Game\n\n";

        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
        case 1:
            cout << "Enter your name: ";
            getline(cin, playerName);

            if (playerName.empty()) {
                playerName = "Player";
            }
            break;

        case 2:
            showTopScores();
            break;

        case 3:
            if (playerName.empty()) {
                cout << "\nPlease enter your name first.\n";
                waitForEnter();
            }
            else {
                startGame();
            }
            break;

        case 4:
            clearScreen();
            cout << "Game closed successfully.\n";
            return;

        default:
            cout << "\nInvalid choice. Try again.\n";
            waitForEnter();
            break;
        }
    }
}

// Main function
int main() {
    srand(time(nullptr));

    mainMenu();

    return 0;
}