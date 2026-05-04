#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

struct TaskEvent {
    string taskName;
    string event;
    string timestamp;
};

vector<TaskEvent> eventLog;

/*
Function: loadEventLog
Purpose: Loads the event log from a file.
Parameters: None.
Returns: A vector of task events.
*/
vector<TaskEvent> loadEventLog() {
    vector<TaskEvent> events;
    ifstream logFile("data/scheduler_event_log.txt");

    string line;

    while (getline(logFile, line)) {
        TaskEvent event;
        stringstream ss(line);
        getline(ss, event.taskName, '|');
        getline(ss, event.event, '|');
        getline(ss, event.timestamp);

        events.push_back(event);
    }

    logFile.close();
    return events;
}

/*
Function: drawTaskLog
Purpose: Draws the task event log on the window.
Parameters: SFML RenderWindow, font, event log.
Returns: Nothing.
*/
void drawTaskLog(sf::RenderWindow &window, sf::Font &font, const vector<TaskEvent> &events) {
    float x = 50, y = 50;
    for (const auto &event : events) {
        sf::Text text;
        text.setFont(font);
        text.setString(event.taskName + " | " + event.event + " | " + event.timestamp);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);
        text.setPosition(x, y);
        window.draw(text);
        y += 30;
    }
}

/*
Function: main
Purpose: Runs the scheduling terminal to show real-time event log, scheduling metrics, etc.
Parameters: None.
Returns: Program exit status.
*/
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Scheduler Terminal");

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        cout << "Error loading font\n";
        return -1;
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear(sf::Color::Black);

        // Load and display task events
        vector<TaskEvent> events = loadEventLog();
        drawTaskLog(window, font, events);

        window.display();
    }

    return 0;
}