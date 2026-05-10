#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

struct ProcessView {
    string pid;
    string name;
    string state;
    string priority;
    string ramBlock;
    int memoryStart;
    int memoryEnd;
};

struct GUIStatus {
    string osMode = "USER MODE";
    string taskMode = "Scheduler-Controlled Mode";

    int ramAvailable = 0;
    int ramTotal = 1;

    int hddAvailable = 0;
    int hddTotal = 1;

    int coresAvailable = 0;
    int coresTotal = 1;

    vector<ProcessView> processes;
};

/*
Function: split
Purpose: Splits a string using a delimiter.
Parameters: Text and delimiter.
Returns: Vector of string parts.
*/
vector<string> split(string text, char delimiter) {
    vector<string> parts;
    string item;
    stringstream ss(text);

    while (getline(ss, item, delimiter)) {
        parts.push_back(item);
    }

    return parts;
}

/*
Function: getValue
Purpose: Gets the value after '=' from a status file line.
Parameters: File line.
Returns: Extracted value.
*/
string getValue(string line) {
    size_t pos = line.find('=');

    if (pos == string::npos) {
        return "";
    }

    return line.substr(pos + 1);
}

/*
Function: toIntSafe
Purpose: Converts string to integer safely.
Parameters: String value.
Returns: Integer value or zero.
*/
int toIntSafe(string value) {
    try {
        return stoi(value);
    } catch (...) {
        return 0;
    }
}

/*
Function: parseRAMBlock
Purpose: Extracts memory start and end from RAM block text.
Parameters: RAM block string, start reference, end reference.
Returns: Nothing.
*/
void parseRAMBlock(string ramBlock, int &start, int &end) {
    start = -1;
    end = -1;

    size_t dashPos = ramBlock.find('-');

    if (dashPos == string::npos) {
        return;
    }

    string startText = ramBlock.substr(0, dashPos);
    string endText = ramBlock.substr(dashPos + 1);

    size_t mbPos = endText.find("MB");

    if (mbPos != string::npos) {
        endText = endText.substr(0, mbPos);
    }

    start = toIntSafe(startText);
    end = toIntSafe(endText);
}

/*
Function: loadStatus
Purpose: Loads AMS OS status from data/gui_status.txt.
Parameters: None.
Returns: GUIStatus object.
*/
GUIStatus loadStatus() {
    GUIStatus status;
    ifstream file("data/gui_status.txt");

    if (!file) {
        return status;
    }

    string line;

    while (getline(file, line)) {
        if (line.find("OS_MODE=") == 0) {
            status.osMode = getValue(line);
        }
        else if (line.find("TASK_MODE=") == 0) {
            status.taskMode = getValue(line);
        }
        else if (line.find("RAM_AVAILABLE=") == 0) {
            status.ramAvailable = toIntSafe(getValue(line));
        }
        else if (line.find("RAM_TOTAL=") == 0) {
            status.ramTotal = max(1, toIntSafe(getValue(line)));
        }
        else if (line.find("HDD_AVAILABLE=") == 0) {
            status.hddAvailable = toIntSafe(getValue(line));
        }
        else if (line.find("HDD_TOTAL=") == 0) {
            status.hddTotal = max(1, toIntSafe(getValue(line)));
        }
        else if (line.find("CORES_AVAILABLE=") == 0) {
            status.coresAvailable = toIntSafe(getValue(line));
        }
        else if (line.find("CORES_TOTAL=") == 0) {
            status.coresTotal = max(1, toIntSafe(getValue(line)));
        }
        else if (line.find("PROCESS=") == 0) {
            string value = getValue(line);
            vector<string> parts = split(value, '|');

            if (parts.size() >= 5) {
                ProcessView process;
                process.pid = parts[0];
                process.name = parts[1];
                process.state = parts[2];
                process.priority = parts[3];
                process.ramBlock = parts[4];

                parseRAMBlock(process.ramBlock, process.memoryStart, process.memoryEnd);

                status.processes.push_back(process);
            }
        }
    }

    file.close();

    return status;
}

/*
Function: loadLogLines
Purpose: Loads system log lines from data/system_log.txt.
Parameters: None.
Returns: Vector of log lines.
*/
vector<string> loadLogLines() {
    vector<string> lines;
    ifstream file("data/system_log.txt");

    if (!file) {
        lines.push_back("No system log file found.");
        return lines;
    }

    string line;

    while (getline(file, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    file.close();

    if (lines.empty()) {
        lines.push_back("System log is empty.");
    }

    return lines;
}

/*
Function: loadUIFont
Purpose: Loads a system font.
Parameters: Font reference.
Returns: true if font loaded.
*/
bool loadUIFont(sf::Font &font) {
    vector<string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf"
    };

    for (string path : fontPaths) {
        if (font.loadFromFile(path)) {
            return true;
        }
    }

    return false;
}

/*
Function: makeText
Purpose: Creates SFML text.
Parameters: Font, text, size, x, y, color.
Returns: SFML text object.
*/
sf::Text makeText(
    sf::Font &font,
    string value,
    unsigned int size,
    float x,
    float y,
    sf::Color color
) {
    sf::Text text;
    text.setFont(font);
    text.setString(value);
    text.setCharacterSize(size);
    text.setPosition(x, y);
    text.setFillColor(color);

    return text;
}

/*
Function: drawPanel
Purpose: Draws a hard technical panel.
Parameters: Window, x, y, width, height, title.
Returns: Nothing.
*/
void drawPanel(
    sf::RenderWindow &window,
    sf::Font &font,
    float x,
    float y,
    float width,
    float height,
    string title
) {
    sf::RectangleShape panel(sf::Vector2f(width, height));
    panel.setPosition(x, y);
    panel.setFillColor(sf::Color(18, 22, 31));
    panel.setOutlineColor(sf::Color(80, 95, 120));
    panel.setOutlineThickness(2);
    window.draw(panel);

    sf::RectangleShape header(sf::Vector2f(width, 34));
    header.setPosition(x, y);
    header.setFillColor(sf::Color(31, 39, 54));
    window.draw(header);

    sf::Text headerText = makeText(font, title, 16, x + 12, y + 8, sf::Color(225, 235, 245));
    headerText.setStyle(sf::Text::Bold);
    window.draw(headerText);
}

/*
Function: drawGridBackground
Purpose: Draws a technical grid background.
Parameters: Window.
Returns: Nothing.
*/
void drawGridBackground(sf::RenderWindow &window) {
    window.clear(sf::Color(9, 12, 18));

    for (int x = 0; x <= 1280; x += 40) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, 0), sf::Color(24, 30, 42)),
            sf::Vertex(sf::Vector2f(x, 760), sf::Color(24, 30, 42))
        };

        window.draw(line, 2, sf::Lines);
    }

    for (int y = 0; y <= 760; y += 40) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, y), sf::Color(24, 30, 42)),
            sf::Vertex(sf::Vector2f(1280, y), sf::Color(24, 30, 42))
        };

        window.draw(line, 2, sf::Lines);
    }
}

/*
Function: getStateColor
Purpose: Returns color according to process state.
Parameters: Process state.
Returns: SFML color.
*/
sf::Color getStateColor(string state) {
    if (state == "READY") {
        return sf::Color(70, 180, 255);
    }

    if (state == "RUNNING") {
        return sf::Color(80, 230, 140);
    }

    if (state == "BLOCKED") {
        return sf::Color(255, 185, 60);
    }

    if (state == "TERMINATED") {
        return sf::Color(255, 80, 90);
    }

    return sf::Color(210, 215, 225);
}

/*
Function: countState
Purpose: Counts processes by state.
Parameters: Process list and state name.
Returns: Count.
*/
int countState(vector<ProcessView> processes, string state) {
    int count = 0;

    for (ProcessView process : processes) {
        if (process.state == state) {
            count++;
        }
    }

    return count;
}

/*
Function: drawResourceBar
Purpose: Draws a technical resource usage bar.
Parameters: Window, font, label, available, total, position, color.
Returns: Nothing.
*/
void drawResourceBar(
    sf::RenderWindow &window,
    sf::Font &font,
    string label,
    int available,
    int total,
    float x,
    float y,
    float width,
    sf::Color color
) {
    int used = total - available;
    float ratio = 0.0f;

    if (total > 0) {
        ratio = static_cast<float>(used) / static_cast<float>(total);
    }

    ratio = max(0.0f, min(1.0f, ratio));

    window.draw(makeText(font, label, 14, x, y, sf::Color(220, 225, 235)));

    string value = to_string(used) + " / " + to_string(total);
    window.draw(makeText(font, value, 13, x + width - 140, y, sf::Color(160, 170, 185)));

    sf::RectangleShape outer(sf::Vector2f(width, 20));
    outer.setPosition(x, y + 25);
    outer.setFillColor(sf::Color(12, 16, 24));
    outer.setOutlineColor(sf::Color(75, 90, 112));
    outer.setOutlineThickness(1);
    window.draw(outer);

    sf::RectangleShape fill(sf::Vector2f(width * ratio, 20));
    fill.setPosition(x, y + 25);
    fill.setFillColor(color);
    window.draw(fill);

    sf::RectangleShape marker(sf::Vector2f(2, 24));
    marker.setPosition(x + width * ratio, y + 23);
    marker.setFillColor(sf::Color(235, 240, 245));
    window.draw(marker);
}

/*
Function: drawStatusHeader
Purpose: Draws main OS header and execution mode.
Parameters: Window, font, status.
Returns: Nothing.
*/
void drawStatusHeader(sf::RenderWindow &window, sf::Font &font, GUIStatus status) {
    sf::RectangleShape topBar(sf::Vector2f(1280, 82));
    topBar.setPosition(0, 0);
    topBar.setFillColor(sf::Color(15, 19, 28));
    topBar.setOutlineColor(sf::Color(75, 90, 112));
    topBar.setOutlineThickness(2);
    window.draw(topBar);

    sf::Text title = makeText(font, "AMS OS CONTROL PANEL", 30, 30, 18, sf::Color(235, 242, 250));
    title.setStyle(sf::Text::Bold);
    window.draw(title);

    window.draw(makeText(font, "Atomic Management System, FAST Style Engineering Dashboard", 14, 32, 53, sf::Color(145, 155, 170)));

    sf::RectangleShape modeBox(sf::Vector2f(380, 50));
    modeBox.setPosition(860, 16);
    modeBox.setFillColor(sf::Color(24, 31, 44));
    modeBox.setOutlineColor(sf::Color(90, 110, 140));
    modeBox.setOutlineThickness(2);
    window.draw(modeBox);

    window.draw(makeText(font, "OS MODE: " + status.osMode, 14, 875, 25, sf::Color(80, 230, 140)));
    window.draw(makeText(font, "TASK MODE: " + status.taskMode, 13, 875, 47, sf::Color(210, 218, 230)));
}

/*
Function: drawResourcePanel
Purpose: Draws system resource usage panel.
Parameters: Window, font, status.
Returns: Nothing.
*/
void drawResourcePanel(sf::RenderWindow &window, sf::Font &font, GUIStatus status) {
    drawPanel(window, font, 30, 105, 370, 130, "RESOURCE MONITOR");

    drawResourceBar(window, font, "RAM", status.ramAvailable, status.ramTotal, 50, 150, 330, sf::Color(60, 160, 240));
    drawResourceBar(window, font, "HDD", status.hddAvailable, status.hddTotal, 50, 205, 330, sf::Color(150, 105, 230));
}

/*
Function: drawCPUCorePanel
Purpose: Draws CPU core usage.
Parameters: Window, font, status.
Returns: Nothing.
*/
void drawCPUCorePanel(sf::RenderWindow &window, sf::Font &font, GUIStatus status) {
    drawPanel(window, font, 425, 105, 310, 130, "CPU CORE MAP");

    int usedCores = status.coresTotal - status.coresAvailable;

    for (int i = 0; i < status.coresTotal && i < 16; i++) {
        float x = 450 + (i % 8) * 32;
        float y = 152 + (i / 8) * 34;

        sf::RectangleShape core(sf::Vector2f(24, 24));
        core.setPosition(x, y);
        core.setOutlineColor(sf::Color(90, 105, 130));
        core.setOutlineThickness(1);

        if (i < usedCores) {
            core.setFillColor(sf::Color(80, 230, 140));
        } else {
            core.setFillColor(sf::Color(35, 42, 55));
        }

        window.draw(core);
        window.draw(makeText(font, to_string(i), 10, x + 7, y + 5, sf::Color(230, 235, 245)));
    }

    window.draw(makeText(font, "Used: " + to_string(usedCores), 13, 450, 205, sf::Color(220, 225, 235)));
    window.draw(makeText(font, "Free: " + to_string(status.coresAvailable), 13, 555, 205, sf::Color(160, 170, 185)));
}

/*
Function: drawProcessCounters
Purpose: Draws process state counters.
Parameters: Window, font, status.
Returns: Nothing.
*/
void drawProcessCounters(sf::RenderWindow &window, sf::Font &font, GUIStatus status) {
    drawPanel(window, font, 760, 105, 490, 130, "PROCESS STATE SUMMARY");

    vector<pair<string, sf::Color>> states = {
        {"READY", sf::Color(70, 180, 255)},
        {"RUNNING", sf::Color(80, 230, 140)},
        {"BLOCKED", sf::Color(255, 185, 60)},
        {"TERMINATED", sf::Color(255, 80, 90)}
    };

    for (size_t i = 0; i < states.size(); i++) {
        float x = 785 + i * 115;
        float y = 155;

        sf::RectangleShape box(sf::Vector2f(95, 52));
        box.setPosition(x, y);
        box.setFillColor(sf::Color(25, 31, 43));
        box.setOutlineColor(states[i].second);
        box.setOutlineThickness(2);
        window.draw(box);

        int value = countState(status.processes, states[i].first);

        window.draw(makeText(font, states[i].first, 12, x + 10, y + 8, states[i].second));
        sf::Text number = makeText(font, to_string(value), 24, x + 36, y + 24, sf::Color(235, 240, 245));
        number.setStyle(sf::Text::Bold);
        window.draw(number);
    }
}

/*
Function: drawProcessTable
Purpose: Draws scrollable process table.
Parameters: Window, font, status, scroll offset.
Returns: Nothing.
*/
void drawProcessTable(
    sf::RenderWindow &window,
    sf::Font &font,
    GUIStatus status,
    int scrollOffset
) {
    float x = 30;
    float y = 260;
    float width = 790;
    float height = 455;

    drawPanel(window, font, x, y, width, height, "SCROLLABLE PCB TABLE");

    window.draw(makeText(font, "Scroll rows, click READY process to dispatch", 12, x + 465, y + 10, sf::Color(160, 170, 185)));
    float headerY = y + 48;

    vector<string> headers = {"PID", "PROCESS", "STATE", "PRI", "RAM BLOCK"};
    vector<float> colX = {x + 20, x + 105, x + 385, x + 520, x + 600};

    for (size_t i = 0; i < headers.size(); i++) {
        sf::Text text = makeText(font, headers[i], 13, colX[i], headerY, sf::Color(85, 200, 255));
        text.setStyle(sf::Text::Bold);
        window.draw(text);
    }

    sf::RectangleShape line(sf::Vector2f(width - 40, 2));
    line.setPosition(x + 20, headerY + 28);
    line.setFillColor(sf::Color(80, 95, 120));
    window.draw(line);

    int rowHeight = 34;
    int visibleRows = 9;
    int startIndex = scrollOffset;
    int endIndex = min(static_cast<int>(status.processes.size()), startIndex + visibleRows);

    if (status.processes.empty()) {
        window.draw(makeText(font, "No active process records available.", 16, x + 25, y + 105, sf::Color(170, 180, 195)));
        return;
    }

    float rowY = headerY + 45;

    for (int i = startIndex; i < endIndex; i++) {
        sf::RectangleShape row(sf::Vector2f(width - 40, rowHeight));
        row.setPosition(x + 20, rowY - 7);

        if ((i - startIndex) % 2 == 0) {
            row.setFillColor(sf::Color(24, 30, 42));
        } else {
            row.setFillColor(sf::Color(30, 37, 50));
        }

        row.setOutlineColor(sf::Color(45, 55, 72));
        row.setOutlineThickness(1);
        window.draw(row);

        window.draw(makeText(font, status.processes[i].pid, 13, colX[0], rowY, sf::Color(225, 230, 238)));
        window.draw(makeText(font, status.processes[i].name, 13, colX[1], rowY, sf::Color(225, 230, 238)));
        window.draw(makeText(font, status.processes[i].state, 13, colX[2], rowY, getStateColor(status.processes[i].state)));
        window.draw(makeText(font, status.processes[i].priority, 13, colX[3], rowY, sf::Color(225, 230, 238)));
        window.draw(makeText(font, status.processes[i].ramBlock, 13, colX[4], rowY, sf::Color(225, 230, 238)));

        rowY += rowHeight + 7;
    }

    string scrollInfo = "Rows " + to_string(startIndex + 1) + " to " + to_string(endIndex) +
                        " of " + to_string(status.processes.size());

    window.draw(makeText(font, scrollInfo, 12, x + 20, y + height - 28, sf::Color(150, 160, 175)));
}

/*
Function: drawMemoryMap
Purpose: Draws RAM block map from PCB memory allocation.
Parameters: Window, font, status.
Returns: Nothing.
*/
void drawMemoryMap(sf::RenderWindow &window, sf::Font &font, GUIStatus status) {
    float x = 850;
    float y = 260;
    float width = 400;
    float height = 210;

    drawPanel(window, font, x, y, width, height, "RAM BLOCK MAP");

    sf::RectangleShape memoryBar(sf::Vector2f(40, 140));
    memoryBar.setPosition(x + 25, y + 52);
    memoryBar.setFillColor(sf::Color(28, 34, 46));
    memoryBar.setOutlineColor(sf::Color(100, 115, 140));
    memoryBar.setOutlineThickness(2);
    window.draw(memoryBar);

    vector<ProcessView> allocated;

    for (ProcessView process : status.processes) {
        if (process.memoryStart >= 0 && process.memoryEnd > process.memoryStart) {
            allocated.push_back(process);
        }
    }

    sort(allocated.begin(), allocated.end(), [](ProcessView a, ProcessView b) {
        return a.memoryStart < b.memoryStart;
    });

    for (ProcessView process : allocated) {
        float startRatio = static_cast<float>(process.memoryStart) / status.ramTotal;
        float endRatio = static_cast<float>(process.memoryEnd) / status.ramTotal;

        startRatio = max(0.0f, min(1.0f, startRatio));
        endRatio = max(0.0f, min(1.0f, endRatio));

        float blockY = y + 52 + startRatio * 140;
        float blockHeight = max(3.0f, (endRatio - startRatio) * 140);

        sf::RectangleShape block(sf::Vector2f(40, blockHeight));
        block.setPosition(x + 25, blockY);
        block.setFillColor(getStateColor(process.state));
        window.draw(block);
    }

    window.draw(makeText(font, "0 MB", 11, x + 72, y + 48, sf::Color(160, 170, 185)));
    window.draw(makeText(font, to_string(status.ramTotal) + " MB", 11, x + 72, y + 184, sf::Color(160, 170, 185)));

    float textY = y + 55;
    int shown = 0;

    for (ProcessView process : allocated) {
        if (shown >= 5) {
            break;
        }

        sf::RectangleShape legend(sf::Vector2f(12, 12));
        legend.setPosition(x + 125, textY + 3);
        legend.setFillColor(getStateColor(process.state));
        window.draw(legend);

        string label = process.pid + "  " + process.name + "  " + process.ramBlock;
        window.draw(makeText(font, label, 11, x + 145, textY, sf::Color(220, 225, 235)));

        textY += 24;
        shown++;
    }

    if (allocated.empty()) {
        window.draw(makeText(font, "RAM is currently free.", 13, x + 125, y + 65, sf::Color(170, 180, 195)));
    }
}

/*
Function: drawLogPreview
Purpose: Draws scrollable system log preview.
Parameters: Window, font, log lines, scroll offset.
Returns: Nothing.
*/
void drawLogPreview(
    sf::RenderWindow &window,
    sf::Font &font,
    vector<string> logLines,
    int logScrollOffset
) {
    float x = 850;
    float y = 495;
    float width = 400;
    float height = 220;

    drawPanel(window, font, x, y, width, height, "SYSTEM LOG PREVIEW");

    int visibleLines = 7;
    int startIndex = logScrollOffset;
    int endIndex = min(static_cast<int>(logLines.size()), startIndex + visibleLines);

    float lineY = y + 48;

    for (int i = startIndex; i < endIndex; i++) {
        string line = logLines[i];

        if (line.length() > 52) {
            line = line.substr(0, 52) + "...";
        }

        sf::Color color = sf::Color(190, 200, 212);

        if (line.find("PROCESS") != string::npos) {
            color = sf::Color(90, 190, 255);
        } else if (line.find("RESOURCE") != string::npos) {
            color = sf::Color(85, 230, 150);
        } else if (line.find("Deadlock") != string::npos || line.find("deadlock") != string::npos) {
            color = sf::Color(255, 90, 95);
        }

        window.draw(makeText(font, line, 11, x + 15, lineY, color));

        lineY += 22;
    }

    string info = "Log lines: " + to_string(logLines.size());
    window.draw(makeText(font, info, 11, x + 15, y + height - 24, sf::Color(150, 160, 175)));
}

/*
Function: drawFooter
Purpose: Draws keyboard shortcut footer.
Parameters: Window, font.
Returns: Nothing.
*/
void drawFooter(sf::RenderWindow &window, sf::Font &font) {
    sf::RectangleShape footer(sf::Vector2f(1280, 28));
    footer.setPosition(0, 732);
    footer.setFillColor(sf::Color(15, 19, 28));
    footer.setOutlineColor(sf::Color(70, 85, 110));
    footer.setOutlineThickness(1);
    window.draw(footer);

    string text = "Controls: R Refresh | UP/DOWN Scroll PCB | PAGE UP/PAGE DOWN Scroll Logs | ESC Close Dashboard";
    window.draw(makeText(font, text, 12, 30, 739, sf::Color(175, 185, 200)));
}
/*
Function: writeGUIRunCommand
Purpose: Writes a process dispatch command for AMS OS to read.
Parameters: PID string.
Returns: Nothing.
*/
void writeGUIRunCommand(string pid) {
    ofstream file("data/gui_command.txt");

    if (!file) {
        return;
    }

    file << "RUN_PID=" << pid << "\n";
    file.close();
}

/*
Function: main
Purpose: Runs the AMS OS graphical dashboard using SFML.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    sf::RenderWindow window(
        sf::VideoMode(1280, 760),
        "AMS OS Robust Graphical Dashboard",
        sf::Style::Close
    );

    window.setFramerateLimit(60);

    sf::Font font;

    if (!loadUIFont(font)) {
        cout << "Could not load system font.\n";
        return 1;
    }

    GUIStatus status = loadStatus();
    vector<string> logLines = loadLogLines();

    int processScrollOffset = 0;
    int logScrollOffset = 0;

    sf::Clock reloadClock;

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }

                if (event.key.code == sf::Keyboard::R) {
                    status = loadStatus();
                    logLines = loadLogLines();
                }

                if (event.key.code == sf::Keyboard::Down) {
                    processScrollOffset++;
                }

                if (event.key.code == sf::Keyboard::Up) {
                    processScrollOffset--;
                }

                if (event.key.code == sf::Keyboard::PageDown) {
                    logScrollOffset++;
                }

                if (event.key.code == sf::Keyboard::PageUp) {
                    logScrollOffset--;
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled) {
                sf::Vector2i mouse = sf::Mouse::getPosition(window);

                if (mouse.x >= 30 && mouse.x <= 820 && mouse.y >= 260 && mouse.y <= 715) {
                    if (event.mouseWheelScroll.delta < 0) {
                        processScrollOffset++;
                    } else {
                        processScrollOffset--;
                    }
                }

                if (mouse.x >= 850 && mouse.x <= 1250 && mouse.y >= 495 && mouse.y <= 715) {
                    if (event.mouseWheelScroll.delta < 0) {
                        logScrollOffset++;
                    } else {
                        logScrollOffset--;
                    }
                }
            }
if (event.type == sf::Event::MouseButtonPressed) {
    if (event.mouseButton.button == sf::Mouse::Left) {
        int mouseX = event.mouseButton.x;
        int mouseY = event.mouseButton.y;

        float tableX = 30;
        float tableY = 260;
        float headerY = tableY + 48;
        int rowHeight = 34;
        int visibleRows = 9;

        float firstRowY = headerY + 45;

        if (mouseX >= tableX && mouseX <= tableX + 790 &&
            mouseY >= firstRowY - 7 &&
            mouseY <= firstRowY + visibleRows * (rowHeight + 7)) {

            int rowIndex = static_cast<int>((mouseY - (firstRowY - 7)) / (rowHeight + 7));
            int processIndex = processScrollOffset + rowIndex;

            if (processIndex >= 0 && processIndex < static_cast<int>(status.processes.size())) {
                ProcessView selectedProcess = status.processes[processIndex];

                if (selectedProcess.state == "READY") {
                    writeGUIRunCommand(selectedProcess.pid);
                    cout << "GUI dispatch request sent for PID: " << selectedProcess.pid << endl;
                } else {
                    cout << "Only READY processes can be dispatched from GUI." << endl;
                }
            }
        }
    }
}
        }

        if (reloadClock.getElapsedTime().asSeconds() >= 1.0f) {
            status = loadStatus();
            logLines = loadLogLines();
            reloadClock.restart();
        }

        int maxProcessScroll = max(0, static_cast<int>(status.processes.size()) - 9);
        int maxLogScroll = max(0, static_cast<int>(logLines.size()) - 7);

        processScrollOffset = max(0, min(processScrollOffset, maxProcessScroll));
        logScrollOffset = max(0, min(logScrollOffset, maxLogScroll));

        drawGridBackground(window);
        drawStatusHeader(window, font, status);
        drawResourcePanel(window, font, status);
        drawCPUCorePanel(window, font, status);
        drawProcessCounters(window, font, status);
        drawProcessTable(window, font, status, processScrollOffset);
        drawMemoryMap(window, font, status);
        drawLogPreview(window, font, logLines, logScrollOffset);
        drawFooter(window, font);

        window.display();
    }

    return 0;
}
