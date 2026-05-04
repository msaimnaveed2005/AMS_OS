#include <iostream>
#include <unistd.h>
#include "../kernel/ui.h"
using namespace std;

/*
Function: main
Purpose: Runs calculator task as a separate executable loaded through exec.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    int choice;
    double num1, num2;

    UI::panelHeader("Calculator", "AMS OS task executable");
    UI::taskControlHint(getpid());

    UI::menuItem(1, "Addition");
    UI::menuItem(2, "Subtraction");
    UI::menuItem(3, "Multiplication");
    UI::menuItem(4, "Division");
    cout << "Enter choice: ";
    cin >> choice;

    cout << "Enter first number: ";
    cin >> num1;

    cout << "Enter second number: ";
    cin >> num2;

    switch (choice) {
        case 1:
            UI::keyValue("Result", to_string(num1 + num2));
            break;

        case 2:
            UI::keyValue("Result", to_string(num1 - num2));
            break;

        case 3:
            UI::keyValue("Result", to_string(num1 * num2));
            break;

        case 4:
            if (num2 == 0) {
                cout << UI::paint("Error: Division by zero is not allowed.\n", UI::RED + UI::BOLD);
            } else {
                UI::keyValue("Result", to_string(num1 / num2));
            }
            break;

        default:
            cout << UI::paint("Invalid calculator option.\n", UI::YELLOW + UI::BOLD);
    }

    cout << UI::paint("Calculator task completed.\n", UI::GREEN + UI::BOLD);
    UI::panelFooter();
    return 0;
}
