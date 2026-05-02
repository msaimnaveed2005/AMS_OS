#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

struct ProcessView {
    string pid;
    string name;
    string state;
    string priority;
    string ramBlock;
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
Parameters: Input string and delimiter character.
Returns: Vector of separated string parts.
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
Purpose: Extracts value after '=' from a status file line.
Parameters: Line from file.
Returns: Value as string.
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
Purpose: Converts a string to integer safely.
Parameters: Text value.
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

                status.processes.push_back(process);
            }
        }
    }

    file.close();

    return status;
}

/*
Function: loadUIFont
Purpose: Loads a system font for the dashboard.
Parameters: Font reference.
Returns: true if font is loaded, otherwise false.
*/
bool loadUIFont(sf::Font &font) {
    vector<string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
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
Purpose: Creates SFML text with standard settings.
Parameters: Font, string, size, position, and color.
Returns: SFML Text object.
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
Function: drawRoundedRect
Purpose: Draws a rounded rectangle using rectangles and circles.
Parameters: Window, position, size, radius, and color.
Returns: Nothing.
*/
void drawRoundedRect(
    sf::RenderWindow &window,
    float x,
    float y,
    float width,
    float height,
    float radius,
    sf::Color color
) {
    sf::RectangleShape center(sf::Vector2f(width - 2 * radius, height));
    center.setPosition(x + radius, y);
    center.setFillColor(color);

    sf::RectangleShape middle(sf::Vector2f(width, height - 2 * radius));
    middle.setPosition(x, y + radius);
    middle.setFillColor(color);

    sf::CircleShape corner(radius);
    corner.setFillColor(color);

    corner.setPosition(x, y);
    window.draw(corner);

    corner.setPosition(x + width - 2 * radius, y);
    window.draw(corner);

    corner.setPosition(x, y + height - 2 * radius);
    window.draw(corner);

    corner.setPosition(x + width - 2 * radius, y + height - 2 * radius);
    window.draw(corner);

    window.draw(center);
    window.draw(middle);
}

/*
Function: drawGlowCircle
Purpose: Draws soft glow circles in the background.
Parameters: Window, position, radius, and base color.
Returns: Nothing.
*/
void drawGlowCircle(
    sf::RenderWindow &window,
    float x,
    float y,
    float radius,
    sf::Color baseColor
) {
    for (int i = 8; i >= 1; i--) {
        float r = radius * i / 8.0f;
        sf::CircleShape circle(r);
        circle.setPosition(x - r, y - r);
        circle.setFillColor(sf::Color(
            baseColor.r,
            baseColor.g,
            baseColor.b,
            static_cast<sf::Uint8>(12)
        ));

        window.draw(circle);
    }
}

/*
Function: drawProgressBar
Purpose: Draws a stylish progress bar for resource usage.
Parameters: Window, font, label, position, size, available amount, total amount.
Returns: Nothing.
*/
void drawProgressBar(
    sf::RenderWindow &window,
    sf::Font &font,
    string label,
    float x,
    float y,
    float width,
    float height,
    int available,
    int total,
    sf::Color accent
) {
    int used = total - available;
    float percentage = 0.0f;

    if (total > 0) {
        percentage = static_cast<float>(used) / static_cast<float>(total);
    }

    if (percentage < 0) {
        percentage = 0;
    }

    if (percentage > 1) {
        percentage = 1;
    }

    sf::Text title = makeText(font, label, 18, x, y - 30, sf::Color(230, 235, 245));
    window.draw(title);

    string usage = to_string(used) + " / " + to_string(total);
    sf::Text usageText = makeText(font, usage, 14, x + width - 120, y - 28, sf::Color(170, 180, 195));
    window.draw(usageText);

    drawRoundedRect(window, x, y, width, height, 10, sf::Color(36, 42, 58));

    float fillWidth = width * percentage;

    if (fillWidth > 0) {
        drawRoundedRect(window, x, y, fillWidth, height, 10, accent);
    }

    sf::RectangleShape shine(sf::Vector2f(fillWidth, height / 2));
    shine.setPosition(x, y);
    shine.setFillColor(sf::Color(255, 255, 255, 25));
    window.draw(shine);
}

/*
Function: drawProcessTable
Purpose: Draws the active process table.
Parameters: Window, font, process list, position and size.
Returns: Nothing.
*/
void drawProcessTable(
    sf::RenderWindow &window,
    sf::Font &font,
    vector<ProcessView> processes,
    float x,
    float y,
    float width,
    float height
) {
    drawRoundedRect(window, x, y, width, height, 22, sf::Color(22, 27, 40, 235));

    sf::Text title = makeText(font, "Active Processes", 24, x + 28, y + 22, sf::Color(245, 247, 255));
    window.draw(title);

    sf::Text sub = makeText(font, "PCB table snapshot with state, priority, and RAM block", 14, x + 28, y + 55, sf::Color(150, 160, 178));
    window.draw(sub);

    float headerY = y + 95;

    vector<string> headers = {"PID", "PROCESS", "STATE", "PRIORITY", "RAM BLOCK"};
    vector<float> columnX = {x + 28, x + 125, x + 430, x + 590, x + 720};

    for (int i = 0; i < headers.size(); i++) {
        sf::Text h = makeText(font, headers[i], 13, columnX[i], headerY, sf::Color(105, 210, 255));
        h.setStyle(sf::Text::Bold);
        window.draw(h);
    }

    sf::RectangleShape line(sf::Vector2f(width - 56, 1));
    line.setPosition(x + 28, headerY + 28);
    line.setFillColor(sf::Color(70, 80, 105));
    window.draw(line);

    if (processes.empty()) {
        sf::Text empty = makeText(font, "No active process records available.", 18, x + 28, y + 155, sf::Color(160, 170, 185));
        window.draw(empty);
        return;
    }

    int maxRows = min(static_cast<int>(processes.size()), 8);
    float rowY = headerY + 50;

    for (int i = 0; i < maxRows; i++) {
        sf::Color rowColor = i % 2 == 0 ? sf::Color(28, 34, 49, 160) : sf::Color(32, 39, 56, 120);
        drawRoundedRect(window, x + 20, rowY - 8, width - 40, 38, 10, rowColor);

        sf::Color stateColor = sf::Color(230, 235, 245);

        if (processes[i].state == "READY") {
            stateColor = sf::Color(105, 210, 255);
        } else if (processes[i].state == "RUNNING") {
            stateColor = sf::Color(110, 255, 175);
        } else if (processes[i].state == "BLOCKED") {
            stateColor = sf::Color(255, 196, 87);
        } else if (processes[i].state == "TERMINATED") {
            stateColor = sf::Color(255, 105, 125);
        }

        window.draw(makeText(font, processes[i].pid, 15, columnX[0], rowY, sf::Color(230, 235, 245)));
        window.draw(makeText(font, processes[i].name, 15, columnX[1], rowY, sf::Color(230, 235, 245)));
        window.draw(makeText(font, processes[i].state, 15, columnX[2], rowY, stateColor));
        window.draw(makeText(font, processes[i].priority, 15, columnX[3], rowY, sf::Color(230, 235, 245)));
        window.draw(makeText(font, processes[i].ramBlock, 15, columnX[4], rowY, sf::Color(230, 235, 245)));

        rowY += 44;
    }
}

/*
Function: main
Purpose: Runs the AMS OS graphical dashboard using SFML.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 760), "AMS OS Graphical Dashboard", sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;

    if (!loadUIFont(font)) {
        cout << "Could not load system font.\n";
        return 1;
    }

    GUIStatus status = loadStatus();

    sf::Clock animationClock;
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
                }
            }
        }

        if (reloadClock.getElapsedTime().asSeconds() >= 1.0f) {
            status = loadStatus();
            reloadClock.restart();
        }

        float t = animationClock.getElapsedTime().asSeconds();

        window.clear(sf::Color(10, 14, 25));

        for (int i = 0; i < 760; i++) {
            float ratio = static_cast<float>(i) / 760.0f;

            sf::RectangleShape line(sf::Vector2f(1280, 1));
            line.setPosition(0, i);
            line.setFillColor(sf::Color(
                static_cast<sf::Uint8>(10 + ratio * 16),
                static_cast<sf::Uint8>(14 + ratio * 18),
                static_cast<sf::Uint8>(25 + ratio * 32)
            ));

            window.draw(line);
        }

        drawGlowCircle(window, 1100 + sin(t) * 20, 115 + cos(t) * 10, 170, sf::Color(38, 169, 255));
        drawGlowCircle(window, 170 + cos(t * 0.8f) * 18, 650 + sin(t) * 10, 150, sf::Color(130, 86, 255));
        drawGlowCircle(window, 640 + sin(t * 0.5f) * 25, 40, 120, sf::Color(52, 255, 181));

        sf::Text title = makeText(font, "AMS OS", 48, 54, 35, sf::Color(250, 252, 255));
        title.setStyle(sf::Text::Bold);
        window.draw(title);

        sf::Text subtitle = makeText(font, "Atomic Management System, Graphical Control Dashboard", 18, 58, 92, sf::Color(150, 165, 185));
        window.draw(subtitle);

        drawRoundedRect(window, 890, 38, 320, 62, 20, sf::Color(22, 27, 40, 220));
        window.draw(makeText(font, status.osMode, 17, 915, 56, sf::Color(110, 255, 175)));
        window.draw(makeText(font, status.taskMode, 13, 915, 78, sf::Color(160, 170, 190)));

        drawRoundedRect(window, 55, 135, 360, 170, 24, sf::Color(22, 27, 40, 230));
        drawRoundedRect(window, 460, 135, 360, 170, 24, sf::Color(22, 27, 40, 230));
        drawRoundedRect(window, 865, 135, 360, 170, 24, sf::Color(22, 27, 40, 230));

        drawProgressBar(window, font, "RAM Usage", 85, 215, 300, 22, status.ramAvailable, status.ramTotal, sf::Color(70, 178, 255));
        drawProgressBar(window, font, "HDD Usage", 490, 215, 300, 22, status.hddAvailable, status.hddTotal, sf::Color(142, 105, 255));
        drawProgressBar(window, font, "CPU Core Usage", 895, 215, 300, 22, status.coresAvailable, status.coresTotal, sf::Color(70, 230, 170));

        window.draw(makeText(font, "Memory Manager", 20, 85, 160, sf::Color(245, 247, 255)));
        window.draw(makeText(font, "Storage Manager", 20, 490, 160, sf::Color(245, 247, 255)));
        window.draw(makeText(font, "CPU Scheduler", 20, 895, 160, sf::Color(245, 247, 255)));

        drawProcessTable(window, font, status.processes, 55, 335, 1170, 340);

        sf::Text footer = makeText(
            font,
            "Press R to refresh manually, Press ESC to close dashboard",
            14,
            55,
            705,
            sf::Color(145, 155, 175)
        );

        window.draw(footer);

        window.display();
    }

    return 0;
}