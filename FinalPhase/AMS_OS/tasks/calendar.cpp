#include <iostream>
#include <ctime>
#include <iomanip>
#include <string>
#include <vector>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: isLeapYear
Purpose: Determines whether a given year is a leap year.
Parameters: year - the year to check.
Returns: true if the year is a leap year, false otherwise.
*/
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*
Function: daysInMonth
Purpose: Returns the number of days in a given month of a given year.
Parameters: month - month number (1-12), year - the full year.
Returns: Number of days in that month.
*/
int daysInMonth(int month, int year) {
    int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month == 2 && isLeapYear(year)) {
        return 29;
    }

    return days[month - 1];
}

/*
Function: firstDayOfMonth
Purpose: Calculates the weekday (0=Sun, 1=Mon, ..., 6=Sat) of the first
         day of a given month and year using Tomohiko Sakamoto's algorithm.
Parameters: month - month number (1-12), year - the full year.
Returns: Weekday index (0=Sunday through 6=Saturday).
*/
int firstDayOfMonth(int month, int year) {
    static int offset[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

    if (month < 3) {
        year--;
    }

    return (year + year / 4 - year / 100 + year / 400 + offset[month - 1] + 1) % 7;
}

/*
Function: monthName
Purpose: Returns the full name of a month given its number.
Parameters: month - month number (1-12).
Returns: Month name as a string.
*/
string monthName(int month) {
    string names[] = {
        "January", "February", "March",     "April",   "May",      "June",
        "July",    "August",   "September", "October", "November", "December"
    };
    return names[month - 1];
}

/*
Function: printCalendar
Purpose: Prints a formatted monthly calendar grid with ANSI color styling.
Parameters: month - month number (1-12), year - the full year,
            todayDay - current day to highlight (0 if not current month).
Returns: Nothing.
*/
void printCalendar(int month, int year, int todayDay) {
    int totalDays = daysInMonth(month, year);
    int startDay  = firstDayOfMonth(month, year);

    string header = monthName(month) + " " + to_string(year);

    cout << "\n";
    cout << "    " << UI::paint(header, UI::ROYAL_BLUE + UI::BOLD) << "\n";
    cout << "    " << UI::paint("----------------------------", UI::DIM) << "\n";
    cout << "    " << UI::paint(" Su", UI::RED)
         << UI::paint("  Mo  Tu  We  Th  Fr", UI::LIGHT_BLUE)
         << UI::paint("  Sa", UI::RED) << "\n";
    cout << "    " << UI::paint("----------------------------", UI::DIM) << "\n";

    cout << "    ";
    int col = 0;

    for (int i = 0; i < startDay; i++) {
        cout << "    ";
        col++;
    }

    for (int day = 1; day <= totalDays; day++) {
        if (day == todayDay) {
            cout << UI::paint("[" + string(day < 10 ? " " : "") + to_string(day) + "]", UI::GREEN + UI::BOLD);
        } else if (col == 0 || col == 6) {
            cout << " " << UI::paint((day < 10 ? " " : "") + to_string(day), UI::YELLOW) << " ";
        } else {
            cout << " " << setw(2) << day << " ";
        }

        col++;

        if (col == 7) {
            cout << "\n    ";
            col = 0;
        }
    }

    if (col != 0) {
        cout << "\n";
    }

    cout << "    " << UI::paint("----------------------------", UI::DIM) << "\n";
}

/*
Function: showCurrentMonth
Purpose: Reads the system clock and displays the calendar for the current month,
         highlighting today's date.
Parameters: None.
Returns: Nothing.
*/
void showCurrentMonth() {
    time_t now  = time(0);
    tm*    info = localtime(&now);

    int todayDay   = info->tm_mday;
    int thisMonth  = info->tm_mon + 1;
    int thisYear   = info->tm_year + 1900;

    UI::sectionBanner("Current Month", UI::BRIGHT_CYAN);
    cout << "    " << UI::paint("Today's date is marked with [ ]", UI::DIM) << "\n";

    printCalendar(thisMonth, thisYear, todayDay);

    char timeBuffer[64];
    strftime(timeBuffer, sizeof(timeBuffer), "%A, %B %d, %Y  %H:%M:%S", info);
    UI::keyValue("Today", string(timeBuffer));
}

/*
Function: showArbitraryMonth
Purpose: Prompts the user to enter a month and year, then displays that
         month's calendar with no day highlighted.
Parameters: None.
Returns: Nothing.
*/
void showArbitraryMonth() {
    int month, year;

    cout << "\n  Enter month (1-12): ";
    cin  >> month;

    if (month < 1 || month > 12) {
        UI::errorLine("Month must be between 1 and 12.");
        return;
    }

    cout << "  Enter year (e.g. 2026): ";
    cin  >> year;

    if (year < 1) {
        UI::errorLine("Year must be a positive number.");
        return;
    }

    printCalendar(month, year, 0);
}

/*
Function: showYearSummary
Purpose: Prompts the user for a year and prints a compact two-column
         summary showing every month name and its day count.
Parameters: None.
Returns: Nothing.
*/
void showYearSummary() {
    int year;

    cout << "\n  Enter year for summary: ";
    cin  >> year;

    if (year < 1) {
        UI::errorLine("Year must be a positive number.");
        return;
    }

    UI::sectionBanner("Year Summary", UI::BRIGHT_GREEN);
    cout << "  " << UI::paint(to_string(year), UI::WHITE + UI::BOLD);
    cout << (isLeapYear(year) ? UI::paint("  (Leap Year)", UI::YELLOW + UI::BOLD) : "") << "\n";
    cout << "  " << UI::paint(UI::repeat('-', 32), UI::DIM) << "\n";
    cout << "  " << left << setw(14) << UI::paint("Month", UI::LIGHT_BLUE + UI::BOLD)
         << setw(6)  << UI::paint("Days", UI::LIGHT_BLUE + UI::BOLD) << "\n";
    cout << "  " << UI::paint(UI::repeat('-', 32), UI::DIM) << "\n";

    int totalDays = 0;
    for (int m = 1; m <= 12; m++) {
        int d = daysInMonth(m, year);
        totalDays += d;
        cout << "  " << left << setw(14) << monthName(m)
             << setw(6)  << d << "\n";
    }

    cout << "  " << UI::paint(UI::repeat('-', 32), UI::DIM) << "\n";
    UI::keyValue("Total days", to_string(totalDays));
}

/*
Function: main
Purpose: Runs the Calendar task as a separate executable loaded through exec.
         Offers the user a menu to view the current month, browse any month,
         or view a full-year day summary.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Calendar", "AMS OS task executable");
    UI::taskControlHint(getpid());

    while (true) {
        UI::sectionBanner("Calendar Menu", UI::BRIGHT_BLUE);
        UI::menuItem(1, "Current Month", "Today's calendar with date highlighted");
        UI::menuItem(2, "Browse Month", "View any month/year");
        UI::menuItem(3, "Year Summary", "Day count for all 12 months");
        UI::menuItem(0, "Exit Calendar");

        int choice = 0;
        cout << "\n  Enter choice: ";
        cin  >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(256, '\n');
            UI::errorLine("Invalid input. Enter a number.");
            continue;
        }

        switch (choice) {
            case 1:
                showCurrentMonth();
                break;
            case 2:
                showArbitraryMonth();
                break;
            case 3:
                showYearSummary();
                break;
            case 0:
                UI::successLine("Calendar task completed.");
                UI::panelFooter();
                return 0;
            default:
                UI::warnLine("Invalid option. Choose 0-3.");
                break;
        }
    }

    return 0;
}