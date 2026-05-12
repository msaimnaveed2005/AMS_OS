#include <iostream>
#include <fstream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../kernel/ui.h"

using namespace std;

/*
Function: main
Purpose: Displays task manager information showing running process list
         from /proc and virtual disk file inventory.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    UI::panelHeader("Task Manager", "Process monitor");
    UI::taskControlHint(getpid());

    UI::sectionBanner("AMS OS Process Info", UI::BRIGHT_CYAN);
    UI::keyValue("Current Task PID", to_string(getpid()));
    UI::keyValue("Parent Kernel PID", to_string(UI::parentProcessID()));
    UI::keyValue("Process Group", to_string(getpgrp()));

    /*
    List files in the virtual disk to show active storage state.
    This gives the user a real snapshot of the simulated HDD.
    */
    UI::sectionBanner("Virtual Disk Contents", UI::BRIGHT_GREEN);
    DIR *dir = opendir("data/virtual_disk");
    if (dir != nullptr) {
        struct dirent *entry;
        int fileCount = 0;

        while ((entry = readdir(dir)) != nullptr) {
            string name(entry->d_name);
            if (name == "." || name == "..") continue;

            string fullPath = "data/virtual_disk/" + name;
            struct stat info;
            if (stat(fullPath.c_str(), &info) == 0) {
                string sizeText = to_string(info.st_size) + " bytes";
                UI::keyValue(name, sizeText);
                fileCount++;
            }
        }
        closedir(dir);

        if (fileCount == 0) {
            UI::infoLine("Virtual disk is empty. Create files using task 1.");
        }

        UI::keyValue("Total Files", to_string(fileCount));
    } else {
        UI::warnLine("Could not open virtual disk directory.");
    }

    /*
    Show a sample of active system processes using /proc.
    This demonstrates awareness of the Linux process model.
    */
    UI::sectionBanner("System Process Sample", UI::BRIGHT_BLUE);
    UI::infoLine("Showing first 10 running processes from /proc:");
    cout << "\n";

    DIR *procDir = opendir("/proc");
    if (procDir != nullptr) {
        struct dirent *entry;
        int shown = 0;

        while ((entry = readdir(procDir)) != nullptr && shown < 10) {
            string name(entry->d_name);

            /* Only process numeric directories (PIDs) */
            bool isNumeric = true;
            for (char ch : name) {
                if (ch < '0' || ch > '9') { isNumeric = false; break; }
            }
            if (!isNumeric) continue;

            string commPath = "/proc/" + name + "/comm";
            ifstream commFile(commPath);
            if (commFile) {
                string processName;
                getline(commFile, processName);
                commFile.close();

                cout << "    " << UI::paint("PID " + name, UI::LIGHT_BLUE + UI::BOLD)
                     << "  " << UI::paint(processName, UI::WHITE)
                     << "\n";
                shown++;
            }
        }
        closedir(procDir);
    } else {
        UI::warnLine("Cannot read /proc. Process listing unavailable.");
    }

    cout << "\n";
    UI::infoLine("For full PCB table and process states, use AMS OS kernel menu option 7.");
    UI::panelFooter();
    return 0;
}
