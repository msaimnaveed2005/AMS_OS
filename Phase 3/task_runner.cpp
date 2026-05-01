#include <iostream>
#include <unistd.h>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[]) {
    string taskName = "Unknown Task";
    int duration = 5;

    if (argc >= 2) {
        taskName = argv[1];
    }

    if (argc >= 3) {
        duration = atoi(argv[2]);
    }

    cout << "\n[EXEC LOADED] Task started: " << taskName << endl;
    cout << "[PID " << getpid() << "] Running for " << duration << " seconds.\n";

    for (int i = 1; i <= duration; i++) {
        cout << "[PID " << getpid() << "] " << taskName
             << " executing step " << i << "/" << duration << endl;
        sleep(1);
    }

    cout << "[PID " << getpid() << "] " << taskName << " completed.\n";
    return 0;
}