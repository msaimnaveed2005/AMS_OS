#include <iostream>
#include <iomanip>
#include <ctime>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: daysInMonth
Purpose: Returns number of days in a selected month, including leap-year handling.
Parameters: Year and month.
Returns: Number of days.
*/
int daysInMonth(int year, int month) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    bool leapYear = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);

    if (month == 1 && leapYear) {
        return 29;
    }

    return days[month];
}

/*
Function: main
Purpose: Displays the current date and monthly calendar as an auto-start task.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    time_t now = time(0);
    tm currentDate = *localtime(&now);

    string monthNames[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    int year = currentDate.tm_year + 1900;
    int month = currentDate.tm_mon;
    int today = currentDate.tm_mday;
    int totalDays = daysInMonth(year, month);

    tm firstDay = currentDate;
    firstDay.tm_mday = 1;
    mktime(&firstDay);

    UI::panelHeader("Calendar", "Auto-start date task");
    UI::taskControlHint(getpid(), true);
    UI::keyValue("Today", monthNames[month] + " " + to_string(today) + ", " + to_string(year));

    cout << "\n  " << UI::paint(monthNames[month] + " " + to_string(year), UI::BOLD) << "\n";
    cout << "  Sun Mon Tue Wed Thu Fri Sat\n";
    cout << "  " << UI::paint(UI::repeat('-', 27) + "\n", UI::DIM);
    cout << "  ";

    for (int i = 0; i < firstDay.tm_wday; i++) {
        cout << "    ";
    }

    for (int day = 1; day <= totalDays; day++) {
        if (day == today) {
            cout << UI::paint((day < 10 ? "  " : " ") + to_string(day) + " ", UI::CYAN + UI::BOLD);
        } else {
            cout << setw(3) << day << " ";
        }

        if ((firstDay.tm_wday + day) % 7 == 0 && day != totalDays) {
            cout << "\n  ";
        }
    }

    cout << "\n";
    UI::panelFooter();

    return 0;
}
