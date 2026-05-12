#include <iostream>
#include <ctime>
#include <unistd.h>
#include <vector>
#include <string>
#include "../kernel/ui.h"
using namespace std;

const vector<string> digitMap[10] = {
    {"  ___  ", " / _ \\ ", "| | | |", "| |_| |", " \\___/ "}, // 0
    {" __ ", "/_ |", " | |", " | |", " |_|"}, // 1
    {" ___  ", "|__ \\ ", "   ) |", "  / / ", " |____|"}, // 2
    {" ____  ", "|___ \\ ", "  __) |", " |__ < ", " |___/ "}, // 3
    {" _  _   ", "| || |  ", "| || |_ ", "|__   _|", "   |_|  "}, // 4
    {" _____ ", "| ____|", "| |__  ", "|___ \\ ", " |___/ "}, // 5
    {"  __  ", " / /  ", "/ /_  ", "| '_ \\", "|___/ "}, // 6
    {" ____  ", "|____| ", "    / /", "   / / ", "  /_/  "}, // 7
    {"  ___  ", " / _ \\ ", "| (_) |", " > _ < ", " \\___/ "}, // 8
    {"  ___  ", " / _ \\ ", "| (_) |", " \\__, |", "   /_/ "}  // 9
};

const vector<string> colonMap = {
    "   ", " _ ", "(_)", " _ ", "(_)"
};

/*
Function: main
Purpose: Runs digital clock task as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Digital Clock", "AMS OS task executable");
    UI::taskControlHint(getpid(), true);
    UI::infoLine("Big Clock mode enabled.");

    for (int i = 1; i <= 60; i++) {
        time_t now = time(0);
        tm *currentTime = localtime(&now);
        
        int h1 = currentTime->tm_hour / 10;
        int h2 = currentTime->tm_hour % 10;
        int m1 = currentTime->tm_min / 10;
        int m2 = currentTime->tm_min % 10;
        int s1 = currentTime->tm_sec / 10;
        int s2 = currentTime->tm_sec % 10;

        UI::clearScreen();
        UI::panelHeader("Digital Clock", "AMS OS task executable");
        UI::taskControlHint(getpid(), true);
        cout << "\n";
        
        for (int row = 0; row < 5; row++) {
            cout << "  " 
                 << UI::paint(digitMap[h1][row], UI::BRIGHT_CYAN + UI::BOLD) << "  "
                 << UI::paint(digitMap[h2][row], UI::BRIGHT_CYAN + UI::BOLD) << "  "
                 << UI::paint(colonMap[row], UI::DIM) << "  "
                 << UI::paint(digitMap[m1][row], UI::BRIGHT_GREEN + UI::BOLD) << "  "
                 << UI::paint(digitMap[m2][row], UI::BRIGHT_GREEN + UI::BOLD) << "  "
                 << UI::paint(colonMap[row], UI::DIM) << "  "
                 << UI::paint(digitMap[s1][row], UI::YELLOW + UI::BOLD) << "  "
                 << UI::paint(digitMap[s2][row], UI::YELLOW + UI::BOLD) << "\n";
        }
        cout << "\n";
        UI::playCue("tick");
        sleep(1);
    }

    cout << UI::paint("Digital Clock task completed.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}

