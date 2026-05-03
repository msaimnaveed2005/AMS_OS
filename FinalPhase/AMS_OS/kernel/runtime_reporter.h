#ifndef RUNTIME_REPORTER_H
#define RUNTIME_REPORTER_H

#include <string>

namespace RuntimeReport {
    void reset();
    void event(const std::string &category, const std::string &message);
}

#endif
