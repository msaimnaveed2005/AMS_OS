#include "runtime_reporter.h"
#include <fstream>
#include <ctime>

using namespace std;

static const string REPORT_PATH = "data/runtime_insights.log";

static string nowStamp() {
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", lt);
    return string(buffer);
}

void RuntimeReport::reset() {
    ofstream out(REPORT_PATH, ios::trunc);
    out << "=== AMS OS Runtime Insights ===\n";
    out << "Generated at " << nowStamp() << "\n\n";
}

void RuntimeReport::event(const string &category, const string &message) {
    ofstream out(REPORT_PATH, ios::app);
    out << "[" << nowStamp() << "] "
        << "[" << category << "] "
        << message << "\n";
}
