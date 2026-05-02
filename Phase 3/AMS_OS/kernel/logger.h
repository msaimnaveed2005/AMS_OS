#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Logger {
private:
    ofstream logFile;

public:
    /*
    Function: Logger
    Purpose: Opens log file for writing logs.
    */
    Logger();

    /*
    Function: ~Logger
    Purpose: Closes log file when program ends.
    */
    ~Logger();

    /*
    Function: logEvent
    Purpose: Writes a log message into system log file.
    */
    void logEvent(string message);

    /*
    Function: logProcessEvent
    Purpose: Logs process-specific events.
    */
    void logProcessEvent(int pid, string processName, string message);

    /*
    Function: logResourceEvent
    Purpose: Logs resource allocation or release events.
    */
    void logResourceEvent(string message);

    /*
    Function: logSystemEvent
    Purpose: Logs system-level events.
    */
    void logSystemEvent(string message);
};

#endif