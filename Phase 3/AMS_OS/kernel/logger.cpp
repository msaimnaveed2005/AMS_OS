#include "logger.h"
#include <ctime>

/*
Function: Logger
Purpose: Opens log file in append mode.
*/
Logger::Logger() {
    logFile.open("data/system_log.txt", ios::app);

    if (!logFile) {
        cout << "[LOGGER] Error opening log file.\n";
    }
}

/*
Function: Destructor
*/
Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

/*
Function: logEvent
Purpose: Writes timestamp + message.
*/
void Logger::logEvent(string message) {
    if (!logFile) return;

    time_t now = time(0);
    char* dt = ctime(&now);

    logFile << "[" << dt;
    logFile.seekp(-1, ios_base::cur);
    logFile << "] " << message << "\n";
}

/*
Function: logProcessEvent
*/
void Logger::logProcessEvent(int pid, string processName, string message) {
    logEvent("PROCESS | PID: " + to_string(pid) + 
             " | " + processName + " | " + message);
}

/*
Function: logResourceEvent
*/
void Logger::logResourceEvent(string message) {
    logEvent("RESOURCE | " + message);
}

/*
Function: logSystemEvent
*/
void Logger::logSystemEvent(string message) {
    logEvent("SYSTEM | " + message);
}