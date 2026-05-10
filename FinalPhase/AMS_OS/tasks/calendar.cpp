#include <iostream>
#include <ctime>
#include <iomanip>
#include <string>
#include <vector>
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
Purpose: Prints a formatted monthly calendar grid to the terminal.
Parameters: month - month number (1-12), year - the full year,
            todayDay - current day to highlight (0 if not current month).
Returns: Nothing.
*/
void printCalendar(int month, int year, int todayDay) {
    int totalDays = daysInMonth(month, year);
    int startDay  = firstDayOfMonth(month, year);

    string header = monthName(month) + " " + to_string(year);

    int padding = (28 - (int)header.size()) / 2;

    cout << string(padding, ' ') << header << "\n";
    cout << "----------------------------\n";
    cout << " Su  Mo  Tu  We  Th  Fr  Sa\n";
    cout << "----------------------------\n";

    int col = 0;

    for (int i = 0; i < startDay; i++) {
        cout << "    ";
        col++;
    }

    for (int day = 1; day <= totalDays; day++) {
        if (day == todayDay) {
            cout << "[" << setw(2) << day << "]";
        } else {
            cout << " " << setw(2) << day << " ";
        }

        col++;

        if (col == 7) {
            cout << "\n";
            col = 0;
        }
    }

    if (col != 0) {
        cout << "\n";
    }

    cout << "----------------------------\n";
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

    cout << "\n  Showing current month (today marked with [ ]):\n\n";

    printCalendar(thisMonth, thisYear, todayDay);

    char timeBuffer[64];
    strftime(timeBuffer, sizeof(timeBuffer), "%A, %B %d, %Y", info);
    cout << "\n  Today: " << timeBuffer << "\n";
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
        cout << "  Error: Month must be between 1 and 12.\n";
        return;
    }

    cout << "  Enter year (e.g. 2025): ";
    cin  >> year;

    if (year < 1) {
        cout << "  Error: Year must be a positive number.\n";
        return;
    }

    cout << "\n";
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
        cout << "  Error: Year must be a positive number.\n";
        return;
    }

    cout << "\n  Year Summary for " << year;
    cout << (isLeapYear(year) ? " (Leap Year)" : "") << ":\n";
    cout << "  --------------------------------\n";
    cout << "  " << left << setw(14) << "Month"
         << setw(6)  << "Days" << "\n";
    cout << "  --------------------------------\n";

    for (int m = 1; m <= 12; m++) {
        cout << "  " << left << setw(14) << monthName(m)
             << setw(6)  << daysInMonth(m, year) << "\n";
    }

    cout << "  --------------------------------\n";
    cout << "  Total days: " << (isLeapYear(year) ? 366 : 365) << "\n";
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
    cout << "\n========== CALENDAR TASK ==========\n";
    cout << "Calendar started as separate executable.\n";

    int choice = 0;

    cout << "\n1. Show current month\n";
    cout << "2. Browse a specific month\n";
    cout << "3. Year day-count summary\n";
    cout << "Enter choice: ";
    cin  >> choice;

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

        default:
            cout << "\nInvalid option. Showing current month by default.\n";
            showCurrentMonth();
            break;
    }

    cout << "\nCalendar task completed.\n";
    return 0;
}