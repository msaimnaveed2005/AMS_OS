#include <iostream>
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

    cout << "\n========== CALCULATOR TASK ==========\n";
    cout << "Calculator started as separate executable.\n";

    cout << "\n1. Addition\n";
    cout << "2. Subtraction\n";
    cout << "3. Multiplication\n";
    cout << "4. Division\n";
    cout << "Enter choice: ";
    cin >> choice;

    cout << "Enter first number: ";
    cin >> num1;

    cout << "Enter second number: ";
    cin >> num2;

    switch (choice) {
        case 1:
            cout << "Result: " << num1 + num2 << endl;
            break;

        case 2:
            cout << "Result: " << num1 - num2 << endl;
            break;

        case 3:
            cout << "Result: " << num1 * num2 << endl;
            break;

        case 4:
            if (num2 == 0) {
                cout << "Error: Division by zero is not allowed.\n";
            } else {
                cout << "Result: " << num1 / num2 << endl;
            }
            break;

        default:
            cout << "Invalid calculator option.\n";
    }

    cout << "Calculator task completed.\n";
    return 0;
}