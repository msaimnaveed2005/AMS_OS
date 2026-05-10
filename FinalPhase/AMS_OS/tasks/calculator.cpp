#include <iostream>
#include <iomanip>
#include <limits>
#include <unistd.h>
#include "../kernel/ui.h"
using namespace std;

bool readMenuChoice(int &choice) {
    cout << "  Select operation [1-5]: ";
    cin >> choice;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }

    return true;
}

bool readNumber(const string &label, double &value) {
    cout << "  " << label;
    cin >> value;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }

    return true;
}

/*
Function: main
Purpose: Runs calculator task as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Calculator", "AMS OS task executable");
    UI::taskControlHint(getpid());
    UI::infoLine("Type values carefully. Invalid input will be rejected.");

    while (true) {
        UI::sectionBanner("Operations", UI::BRIGHT_BLUE);
        UI::menuItem(1, "Addition", "a + b");
        UI::menuItem(2, "Subtraction", "a - b");
        UI::menuItem(3, "Multiplication", "a x b");
        UI::menuItem(4, "Division", "a / b");
        UI::menuItem(5, "Exit Calculator");

        int choice = 0;

        if (!readMenuChoice(choice)) {
            UI::errorLine("Invalid menu input. Enter a number from 1 to 5.");
            continue;
        }

        if (choice == 5) {
            break;
        }

        if (choice < 1 || choice > 4) {
            UI::warnLine("Unknown option. Try again.");
            continue;
        }

        double num1 = 0.0;
        double num2 = 0.0;

        if (!readNumber("Enter first number: ", num1)) {
            UI::errorLine("Invalid number entered for first value.");
            continue;
        }

        if (!readNumber("Enter second number: ", num2)) {
            UI::errorLine("Invalid number entered for second value.");
            continue;
        }

        double result = 0.0;
        bool validOperation = true;
        string expression;

        switch (choice) {
            case 1:
                result = num1 + num2;
                expression = "a + b";
                break;
            case 2:
                result = num1 - num2;
                expression = "a - b";
                break;
            case 3:
                result = num1 * num2;
                expression = "a x b";
                break;
            case 4:
                if (num2 == 0) {
                    UI::errorLine("Division by zero is not allowed.");
                    validOperation = false;
                } else {
                    result = num1 / num2;
                    expression = "a / b";
                }
                break;
        }

        if (!validOperation) {
            continue;
        }

        ostringstream out;
        out << fixed << setprecision(4) << result;

        UI::sectionBanner("Result", UI::BRIGHT_GREEN);
        UI::keyValue("Operation", expression);
        UI::keyValue("Input A", to_string(num1));
        UI::keyValue("Input B", to_string(num2));
        UI::keyValue("Output", out.str());
        UI::playCue("tick");
    }

    UI::successLine("Calculator task completed.");
    UI::panelFooter();
    return 0;
}
