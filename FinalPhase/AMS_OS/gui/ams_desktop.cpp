/*
AMS OS Desktop — Graphical Operating System Interface
GTK3 desktop with login screen, boot splash, app launcher, taskbar, and shutdown animation.
All tasks launch as native GTK3 GUI apps (not in terminals).
Dynamic app registry loaded from data/desktop_apps.txt.
*/

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h>

/* ═══════════════════════════════════════════════════════
   Icon Mapping Helper
   ═══════════════════════════════════════════════════════ */
static const char* get_icon_for_app(const std::string& name) {
    if (name == "Create File") return "document-new";
    if (name == "Delete File") return "edit-delete";
    if (name == "Copy File") return "edit-copy";
    if (name == "Move File") return "go-next";
    if (name == "File Info") return "dialog-information";
    if (name == "Notepad") return "text-editor";
    if (name == "Calculator") return "accessories-calculator";
    if (name == "Digital Clock") return "preferences-system-time";
    if (name == "System Info") return "utilities-system-monitor";
    if (name == "Snake Game") return "face-cool";
    if (name == "Minesweeper") return "dialog-warning";
    if (name == "Music Player") return "media-playback-start";
    if (name == "Downloads") return "emblem-downloads";
    if (name == "Task Manager") return "utilities-system-monitor";
    if (name == "Process Killer") return "process-stop";
    if (name == "Calendar") return "x-office-calendar";
    if (name == "AI Copilot") return "face-smile";
    if (name == "Sudoku") return "view-grid";
    if (name == "Chess") return "system-users";
    if (name == "Tic Tac Toe") return "format-justify-center";
    if (name == "Flappy Bird") return "applications-games";
    
    if (name.find(".") != std::string::npos) return "text-x-generic"; // For files
    return "application-x-executable";
}

/* ═══════════════════════════════════════════════════════
   Task Registry — Dynamic, loaded from data/desktop_apps.txt
   ═══════════════════════════════════════════════════════ */

struct TaskEntry {
    int         id;
    std::string name;
    std::string emoji;
    std::string exec_path;
};

static std::vector<TaskEntry> TASKS;
static int TASK_COUNT = 0;

/* ── Default app definitions (written if registry file missing) ── */
static void write_default_tasks() {
    std::ofstream f("data/desktop_apps.txt");
    if (!f.is_open()) return;
    f << "1|Create File|📄|./build/gui_create_file\n";
    f << "2|Delete File|🗑️|./build/gui_delete_file\n";
    f << "3|Copy File|📋|./build/gui_file_copy\n";
    f << "4|Move File|📦|./build/gui_move_file\n";
    f << "5|File Info|ℹ️|./build/gui_file_info\n";
    f << "6|Notepad|📝|./build/gui_notepad\n";
    f << "7|Calculator|🧮|./build/gui_calculator\n";
    f << "8|Digital Clock|🕐|./build/gui_clock\n";
    f << "9|System Info|💻|./build/gui_system_info\n";
    f << "10|Snake Game|🐍|./build/gui_snake\n";
    f << "11|Minesweeper|💣|./build/gui_minesweeper\n";
    f << "12|Music Player|🎵|./build/gui_music_player\n";
    f << "13|Downloads|⬇️|./build/gui_download_simulator\n";
    f << "14|Task Manager|📊|./build/gui_task_manager\n";
    f << "15|Process Killer|⚡|./build/gui_process_killer\n";
    f << "16|Calendar|📅|./build/gui_calendar\n";
    f << "17|AI Copilot|✦|./build/gui_ai_copilot\n";
    f << "18|Sudoku|🔢|./build/gui_sudoku\n";
    f << "19|Chess|♟️|./build/gui_chess\n";
    f << "20|Terminal|💻|./build/gui_terminal\n";
    f << "21|File Explorer|📁|./build/gui_file_explorer\n";
    f << "22|Settings|⚙️|./build/gui_settings\n";
    f << "23|AMS Studio|💻|./build/gui_ams_studio\n";
    f << "24|Photo Viewer|🖼️|./build/gui_photo_viewer\n";
    f << "25|Flappy Bird|🐦|./build/gui_flappy_bird\n";
}

static void load_tasks() {
    TASKS.clear();

    /* Ensure data directory exists */
    int r1 = system("mkdir -p data 2>/dev/null");
    (void)r1;

    std::ifstream f("data/desktop_apps.txt");
    if (!f.is_open()) {
        write_default_tasks();
        f.open("data/desktop_apps.txt");
        if (!f.is_open()) return;
    }

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        /* Remove trailing \r if present (Windows line endings) */
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        /* Format: id|name|emoji|exec_path */
        size_t p1 = line.find('|');
        if (p1 == std::string::npos) continue;
        size_t p2 = line.find('|', p1 + 1);
        if (p2 == std::string::npos) continue;
        size_t p3 = line.find('|', p2 + 1);
        if (p3 == std::string::npos) continue;

        TaskEntry t;
        try { t.id = std::stoi(line.substr(0, p1)); } catch (...) { continue; }
        t.name      = line.substr(p1 + 1, p2 - p1 - 1);
        t.emoji     = line.substr(p2 + 1, p3 - p2 - 1);
        t.exec_path = line.substr(p3 + 1);

        TASKS.push_back(t);
    }
    
    /* Ensure desktop directory exists and load files from it */
    int r2 = system("mkdir -p data/desktop 2>/dev/null");
    (void)r2;
    DIR *dir = opendir("data/desktop");
    if (dir) {
        struct dirent *ent;
        int virtual_id = 1000;
        while ((ent = readdir(dir))) {
            if (ent->d_name[0] == '.') continue;
            std::string name = ent->d_name;
            std::string path = std::string("data/desktop/") + name;
            
            TaskEntry t;
            t.id = virtual_id++;
            t.name = name;
            t.emoji = ""; // Unused when using GTK icons
            
            if (ent->d_type == DT_DIR) {
                t.exec_path = std::string("xdg-open ") + path; // Simple fallback
            } else {
                t.exec_path = std::string("./build/gui_notepad ") + path;
            }
            TASKS.push_back(t);
        }
        closedir(dir);
    }
    
    TASK_COUNT = (int)TASKS.size();
}

/* ═══════════════════════════════════════════════════════
   Application State
   ═══════════════════════════════════════════════════════ */

struct AppState {
    GtkApplication *app;
    GtkWidget      *clock_label;
    GtkWidget      *progress_bar;
    GtkWidget      *splash_window;
    GtkWidget      *search_entry;
    GtkWidget      *flow_box;
    GtkWidget      *login_error;
    GtkWidget      *desktop_win;
    GtkWidget      *desktop_content;
    int             splash_step;
    int             shutdown_step;
    int             argc;
    char          **argv;
    GtkWidget      *cpu_label;
    GtkWidget      *ram_label;
};

static AppState S = {};

/* Forward declarations */
static void show_login();
static void show_splash();
static void show_desktop();
static void show_mode_selector();
static void populate_grid();
static void refresh_desktop();

/* Real-time Refresh via SIGUSR1 */
static volatile sig_atomic_t g_needs_refresh = 0;
static void handle_sigusr1(int) { g_needs_refresh = 1; }

static gboolean check_refresh_flag(gpointer) {
    if (g_needs_refresh) {
        g_needs_refresh = 0;
        refresh_desktop();
    }
    return G_SOURCE_CONTINUE;
}

/* ═══════════════════════════════════════════════════════
   CSS Theme — Premium dark desktop aesthetic
   ═══════════════════════════════════════════════════════ */

static const char *APP_CSS = R"CSS(

/* ── Login Screen ── */
window.login { background-color: #06060f; }
.login-avatar {
    font-size: 72px;
    margin-top: 20px;
}
.login-title {
    font-size: 22px;
    font-weight: 700;
    color: white;
    margin-top: 8px;
}
.login-entry {
    background-color: rgba(255,255,255,0.08);
    border: 1px solid rgba(255,255,255,0.1);
    border-radius: 10px;
    color: white;
    padding: 10px 16px;
    font-size: 14px;
    min-width: 260px;
    caret-color: white;
}
.login-entry:focus {
    border-color: rgba(139,92,246,0.6);
    background-color: rgba(255,255,255,0.12);
}
.login-btn {
    background-color: rgba(99,102,241,0.5);
    border: 1px solid rgba(99,102,241,0.6);
    border-radius: 10px;
    color: white;
    padding: 10px 40px;
    font-size: 14px;
    font-weight: 600;
    min-width: 260px;
    transition: all 150ms ease-in-out;
}
.login-btn:hover {
    background-color: rgba(99,102,241,0.7);
}
.login-hint {
    font-size: 11px;
    color: rgba(255,255,255,0.25);
    margin-top: 16px;
}
.login-error {
    font-size: 12px;
    color: #f87171;
    margin-top: 4px;
}

/* ── Mode Selector ── */
window.mode-select { background-color: #08081a; }
.mode-title {
    font-size: 48px; font-weight: 800; color: #a78bfa; margin-top: 36px;
}
.mode-subtitle {
    font-size: 13px; color: rgba(255,255,255,0.4); margin-bottom: 32px;
}
.mode-card {
    background-color: rgba(255,255,255,0.05);
    border: 1px solid rgba(255,255,255,0.08);
    border-radius: 22px; padding: 36px 52px;
    margin: 10px; min-width: 180px; min-height: 120px;
    transition: all 200ms ease-in-out;
}
.mode-card:hover {
    background-color: rgba(139,92,246,0.18);
    border-color: rgba(139,92,246,0.45);
    box-shadow: 0 8px 40px rgba(139,92,246,0.25);
}
.mode-emoji { font-size: 52px; }
.mode-label { font-size: 18px; font-weight: 700; color: white; margin-top: 10px; }
.mode-desc  { font-size: 11px; color: rgba(255,255,255,0.45); margin-top: 2px; }

/* ── Splash Screen ── */
window.splash {
    background-color: #06060f;
    animation: fade-in 1s ease-out;
}
.splash-logo {
    margin-top: 50px;
    animation: pulse-logo 2.5s infinite ease-in-out;
}
@keyframes pulse-logo {
    0% { opacity: 0.7; }
    50% { opacity: 1.0; }
    100% { opacity: 0.7; }
}
@keyframes fade-in {
    from { opacity: 0; }
    to { opacity: 1; }
}
.splash-name { font-size: 18px; font-weight: 700; color: rgba(255,255,255,0.75); letter-spacing: 8px; margin-top: 10px; }
.splash-ver  { font-size: 11px; color: rgba(255,255,255,0.25); margin-top: 20px; }
progressbar.splash-bar trough   { background-color: rgba(255,255,255,0.06); border-radius: 3px; min-height: 4px; }
progressbar.splash-bar progress { background-image: linear-gradient(to right, #6366f1, #a855f7, #ec4899); border-radius: 3px; min-height: 4px; }

/* ── Shutdown Screen ── */
window.shutdown {
    background-color: #06060f;
    animation: fade-in 0.8s ease-out;
}
.shutdown-icon { font-size: 56px; margin-bottom: 16px; opacity: 0.6; }
.shutdown-text { font-size: 16px; color: rgba(255,255,255,0.6); font-weight: 500; }
progressbar.shutdown-bar trough   { background-color: rgba(255,255,255,0.06); border-radius: 3px; min-height: 3px; }
progressbar.shutdown-bar progress { background-image: linear-gradient(to right, #6366f1, #a855f7); border-radius: 3px; min-height: 3px; }

/* ── Desktop Wallpaper — Ultra-dark cosmic gradient ── */
window.desktop {
    background-image: linear-gradient(
        145deg,
        #030014 0%,
        #0a0520 15%,
        #10082e 30%,
        #0d0628 45%,
        #120a35 55%,
        #1a0e3a 65%,
        #140830 80%,
        #0a0418 100%
    );
}

/* ── Top Bar — Frosted glass panel ── */
.top-bar {
    background-color: rgba(6, 3, 18, 0.82);
    border-bottom: 1px solid rgba(139, 92, 246, 0.12);
    padding: 4px 20px;
    min-height: 30px;
    box-shadow: 0 2px 20px rgba(0, 0, 0, 0.5);
}
.top-logo {
    color: rgba(255, 255, 255, 0.95);
    font-size: 13px;
    font-weight: 800;
    letter-spacing: 1px;
}
.top-clock {
    color: rgba(200, 180, 255, 0.9);
    font-size: 13px;
    font-weight: 600;
    letter-spacing: 0.5px;
}
.top-user {
    color: rgba(167, 139, 250, 0.7);
    font-size: 12px;
    font-weight: 500;
}

/* ── Search — Glassmorphic pill ── */
.search-box {
    background-color: rgba(255, 255, 255, 0.06);
    border: 1px solid rgba(139, 92, 246, 0.15);
    border-radius: 22px;
    color: white;
    padding: 10px 24px;
    font-size: 14px;
    min-width: 340px;
    margin-top: 28px;
    margin-bottom: 24px;
    caret-color: #a78bfa;
    box-shadow: 0 4px 24px rgba(0, 0, 0, 0.3),
                inset 0 1px 0 rgba(255, 255, 255, 0.04);
    transition: all 200ms ease-in-out;
}
.search-box:focus {
    background-color: rgba(255, 255, 255, 0.1);
    border-color: rgba(139, 92, 246, 0.45);
    box-shadow: 0 4px 30px rgba(139, 92, 246, 0.15),
                0 0 0 3px rgba(139, 92, 246, 0.08),
                inset 0 1px 0 rgba(255, 255, 255, 0.06);
}

/* ── App Icons — Glassmorphic hover with glow ── */
.app-btn {
    background-color: transparent;
    border: none;
    background-image: none;
    border-radius: 20px;
    padding: 8px;
    min-width: 108px;
    min-height: 120px;
    box-shadow: none;
}
.app-icon-wrapper {
    background-color: rgba(255, 255, 255, 0.03);
    border: 1px solid rgba(255, 255, 255, 0.08);
    border-radius: 20px;
    padding: 18px;
    min-width: 52px;
    min-height: 52px;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.2),
                inset 0 1px 0 rgba(255, 255, 255, 0.04);
    transition: all 250ms cubic-bezier(0.16, 1, 0.3, 1);
}
.app-btn:hover .app-icon-wrapper {
    background-color: rgba(139, 92, 246, 0.15);
    border-color: rgba(139, 92, 246, 0.45);
    box-shadow: 0 12px 32px rgba(139, 92, 246, 0.3),
                inset 0 1px 0 rgba(255, 255, 255, 0.08);
}
.app-btn:active .app-icon-wrapper {
    background-color: rgba(139, 92, 246, 0.28);
    border-color: rgba(139, 92, 246, 0.6);
    box-shadow: 0 6px 16px rgba(139, 92, 246, 0.2);
}
.app-icon-emoji {
    font-size: 38px;
}
.app-icon-name {
    color: rgba(255, 255, 255, 0.9);
    font-size: 11px;
    font-weight: 600;
    margin-top: 8px;
    letter-spacing: 0.3px;
    text-shadow: 0 2px 8px rgba(0, 0, 0, 0.8);
}
.app-btn:hover .app-icon-name {
    color: #c084fc;
}

/* ── Bottom Dock — Frosted glass bar ── */
.dock-outer {
    margin: 0px 0px 14px 0px;
}
.dock {
    background-color: rgba(8, 4, 22, 0.7);
    border: 1px solid rgba(139, 92, 246, 0.15);
    border-radius: 24px;
    padding: 8px 18px;
    box-shadow: 0 12px 40px rgba(0, 0, 0, 0.6),
                inset 0 1px 0 rgba(255, 255, 255, 0.05);
}
.dock-btn {
    background-color: transparent;
    border: none;
    background-image: none;
    border-radius: 16px;
    padding: 2px;
    min-width: 50px;
    min-height: 50px;
    box-shadow: none;
}
.dock-icon-wrapper {
    background-color: rgba(255, 255, 255, 0.04);
    border: 1px solid rgba(255, 255, 255, 0.08);
    border-radius: 14px;
    padding: 8px;
    min-width: 32px;
    min-height: 32px;
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2),
                inset 0 1px 0 rgba(255, 255, 255, 0.05);
    transition: all 250ms cubic-bezier(0.16, 1, 0.3, 1);
}
.dock-btn:hover .dock-icon-wrapper {
    background-color: rgba(139, 92, 246, 0.2);
    border-color: rgba(139, 92, 246, 0.45);
    box-shadow: 0 8px 24px rgba(139, 92, 246, 0.35),
                inset 0 1px 0 rgba(255, 255, 255, 0.1);
}
.dock-btn:active .dock-icon-wrapper {
    background-color: rgba(139, 92, 246, 0.3);
    border-color: rgba(139, 92, 246, 0.6);
}
.dock-emoji {
    font-size: 24px;
}

/* ── Power / Refresh — Pill buttons ── */
.power-btn {
    background-color: rgba(255, 60, 60, 0.08);
    border: 1px solid rgba(255, 60, 60, 0.12);
    padding: 3px 12px;
    border-radius: 10px;
    color: rgba(255, 120, 120, 0.8);
    font-size: 13px;
    transition: all 180ms ease-in-out;
}
.power-btn:hover {
    background-color: rgba(255, 60, 60, 0.25);
    border-color: rgba(255, 60, 60, 0.4);
    color: #ff9999;
    box-shadow: 0 2px 12px rgba(255, 60, 60, 0.2);
}
.refresh-btn {
    background-color: rgba(52, 211, 153, 0.06);
    border: 1px solid rgba(52, 211, 153, 0.1);
    padding: 3px 12px;
    border-radius: 10px;
    color: rgba(52, 211, 153, 0.7);
    font-size: 13px;
    transition: all 180ms ease-in-out;
}
.refresh-btn:hover {
    background-color: rgba(52, 211, 153, 0.2);
    border-color: rgba(52, 211, 153, 0.35);
    color: #6ee7b7;
    box-shadow: 0 2px 12px rgba(52, 211, 153, 0.15);
}

/* ── Shutdown / Power Menu Cards ── */
.shutdown-card {
    background-color: rgba(255, 255, 255, 0.03);
    border: 1px solid rgba(139, 92, 246, 0.1);
    border-radius: 18px;
    padding: 22px 36px;
    margin: 8px;
    min-width: 110px;
    transition: all 200ms ease-in-out;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
}
.shutdown-card:hover {
    background-color: rgba(139, 92, 246, 0.12);
    border-color: rgba(139, 92, 246, 0.4);
    box-shadow: 0 8px 36px rgba(139, 92, 246, 0.2);
}
.shutdown-card-emoji { font-size: 34px; }
.shutdown-card-label {
    font-size: 12px;
    color: rgba(255, 255, 255, 0.65);
    font-weight: 500;
    margin-top: 8px;
    letter-spacing: 0.3px;
}

)CSS";

/* ═══════════════════════════════════════════════════════
   Utilities
   ═══════════════════════════════════════════════════════ */

static std::string get_clock_text() {
    time_t now = time(NULL); struct tm *t = localtime(&now);
    char buf[64]; strftime(buf, sizeof(buf), "%a %b %d   %I:%M %p", t);
    return buf;
}

/* Launch a GUI task directly (no terminal wrapper needed) */
static void launch_gui_task(const char *path) {
    pid_t pid = fork();
    if (pid == 0) { setsid(); execlp(path, path, (char *)NULL); _exit(1); }
}

static GtkCssProvider *global_provider = NULL;

static void apply_css() {
    if (!global_provider) {
        global_provider = gtk_css_provider_new();
        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(global_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    std::ifstream in("data/theme.css");
    if (in.is_open()) {
        std::string css_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        gtk_css_provider_load_from_data(global_provider, css_str.c_str(), -1, NULL);
        in.close();
    } else {
        std::string dir_cmd = "mkdir -p data";
        int ret = system(dir_cmd.c_str());
        (void)ret;

        std::ofstream out("data/theme.css");
        out << APP_CSS;
        out.close();
        gtk_css_provider_load_from_data(global_provider, APP_CSS, -1, NULL);
    }
}

static void add_class(GtkWidget *w, const char *cls) {
    gtk_style_context_add_class(gtk_widget_get_style_context(w), cls);
}

/* ═══════════════════════════════════════════════════════
   Clock tick
   ═══════════════════════════════════════════════════════ */

static gboolean tick_clock(gpointer) {
    if (S.clock_label && GTK_IS_LABEL(S.clock_label))
        gtk_label_set_text(GTK_LABEL(S.clock_label), get_clock_text().c_str());
    return G_SOURCE_CONTINUE;
}

/* ═══════════════════════════════════════════════════════
   Real-Time Hardware Telemetry
   ═══════════════════════════════════════════════════════ */

static unsigned long long prev_total = 0, prev_idle = 0;

static double get_cpu_usage() {
    std::ifstream stat_file("/proc/stat");
    std::string line;
    if (std::getline(stat_file, line)) {
        if (line.compare(0, 3, "cpu") == 0) {
            std::istringstream iss(line);
            std::string cpu;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
            if (iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice) {
                unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
                unsigned long long idle_all = idle + iowait;
                double usage = 0.0;
                if (prev_total != 0) {
                    unsigned long long total_d = total - prev_total;
                    unsigned long long idle_d = idle_all - prev_idle;
                    if (total_d > 0) usage = (1.0 - ((double)idle_d / total_d)) * 100.0;
                }
                prev_total = total;
                prev_idle = idle_all;
                return usage;
            }
        }
    }
    return 0.0;
}

static double get_ram_usage() {
    std::ifstream mem_file("/proc/meminfo");
    std::string line;
    double total = 0, avail = 0;
    while (std::getline(mem_file, line)) {
        if (line.compare(0, 8, "MemTotal") == 0) {
            sscanf(line.c_str(), "MemTotal: %lf kB", &total);
        } else if (line.compare(0, 12, "MemAvailable") == 0) {
            sscanf(line.c_str(), "MemAvailable: %lf kB", &avail);
        }
    }
    if (total > 0) return ((total - avail) / total) * 100.0;
    return 0.0;
}

static gboolean tick_telemetry(gpointer) {
    if (S.cpu_label && GTK_IS_LABEL(S.cpu_label)) {
        double cpu = get_cpu_usage();
        char buf[32]; snprintf(buf, sizeof(buf), "⚙ CPU: %.1f%%  ", cpu);
        gtk_label_set_text(GTK_LABEL(S.cpu_label), buf);
    }
    if (S.ram_label && GTK_IS_LABEL(S.ram_label)) {
        double ram = get_ram_usage();
        char buf[32]; snprintf(buf, sizeof(buf), "💾 RAM: %.1f%%  ", ram);
        gtk_label_set_text(GTK_LABEL(S.ram_label), buf);
    }
    return G_SOURCE_CONTINUE;
}

/* ═══════════════════════════════════════════════════════
   Search filter
   ═══════════════════════════════════════════════════════ */

static gboolean filter_func(GtkFlowBoxChild *child, gpointer) {
    if (!S.search_entry) return TRUE;
    const char *query = gtk_entry_get_text(GTK_ENTRY(S.search_entry));
    if (!query || strlen(query) == 0) return TRUE;
    GtkWidget *btn = gtk_bin_get_child(GTK_BIN(child));
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "task_idx"));
    if (idx < 0 || idx >= TASK_COUNT) return TRUE;
    std::string n = TASKS[idx].name;
    std::string q(query);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    return n.find(q) != std::string::npos ? TRUE : FALSE;
}

static void on_search_changed(GtkEntry *, gpointer) {
    if (S.flow_box) gtk_flow_box_invalidate_filter(GTK_FLOW_BOX(S.flow_box));
}

/* ═══════════════════════════════════════════════════════
   Task click — direct launch (no terminal)
   ═══════════════════════════════════════════════════════ */

static void on_app_click(GtkWidget *btn, gpointer) {
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "task_idx"));
    if (idx >= 0 && idx < TASK_COUNT)
        launch_gui_task(TASKS[idx].exec_path.c_str());
}

/* ═══════════════════════════════════════════════════════
   Refresh Desktop — reload registry and rebuild grid
   ═══════════════════════════════════════════════════════ */

static void refresh_desktop() {
    if (!S.flow_box) return;

    /* Reload tasks from file */
    load_tasks();

    /* Hot-reload CSS Theme */
    apply_css();

    /* Remove all children from the flow box */
    GList *children = gtk_container_get_children(GTK_CONTAINER(S.flow_box));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);

    /* Re-populate the grid */
    populate_grid();

    /* Re-apply the filter */
    gtk_flow_box_invalidate_filter(GTK_FLOW_BOX(S.flow_box));
}

static void on_refresh_clicked(GtkWidget *, gpointer) {
    refresh_desktop();
}

static gboolean on_desktop_focus(GtkWidget *, GdkEventFocus *, gpointer) {
    refresh_desktop();
    return FALSE;
}

/* ═══════════════════════════════════════════════════════
   Populate Icon Grid
   ═══════════════════════════════════════════════════════ */

static void populate_grid() {
    if (!S.flow_box) return;
    for (int i = 0; i < TASK_COUNT; i++) {
        GtkWidget *btn = gtk_button_new();
        add_class(btn, "app-btn");
        g_object_set_data(G_OBJECT(btn), "task_idx", GINT_TO_POINTER(i));

        GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_halign(inner, GTK_ALIGN_CENTER);

        GtkWidget *emoji_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        add_class(emoji_wrapper, "app-icon-wrapper");
        gtk_widget_set_halign(emoji_wrapper, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(emoji_wrapper, GTK_ALIGN_CENTER);

        const char *icon_name = get_icon_for_app(TASKS[i].name);
        if (TASKS[i].exec_path.find("xdg-open") != std::string::npos) {
            icon_name = "folder";
        }
        GtkWidget *emoji = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_DIALOG);
        add_class(emoji, "app-icon-emoji"); // Keep class for possible CSS
        gtk_box_pack_start(GTK_BOX(emoji_wrapper), emoji, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(inner), emoji_wrapper, FALSE, FALSE, 0);

        GtkWidget *name = gtk_label_new(TASKS[i].name.c_str());
        add_class(name, "app-icon-name");
        gtk_label_set_max_width_chars(GTK_LABEL(name), 14);
        gtk_label_set_ellipsize(GTK_LABEL(name), PANGO_ELLIPSIZE_END);
        gtk_box_pack_start(GTK_BOX(inner), name, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(btn), inner);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_app_click), NULL);
        gtk_container_add(GTK_CONTAINER(S.flow_box), btn);
    }
    gtk_widget_show_all(S.flow_box);
}

/* ═══════════════════════════════════════════════════════
   Shutdown Screen
   ═══════════════════════════════════════════════════════ */

static GtkWidget *shutdown_win = NULL;
static GtkWidget *shutdown_bar = NULL;

static gboolean shutdown_tick(gpointer) {
    S.shutdown_step++;
    if (S.shutdown_step >= 25) {
        g_application_quit(G_APPLICATION(S.app));
        return G_SOURCE_REMOVE;
    }
    if (shutdown_bar)
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(shutdown_bar), (double)S.shutdown_step / 25.0);
    return G_SOURCE_CONTINUE;
}

static void do_shutdown(GtkWidget *desktop) {
    gtk_widget_destroy(desktop);
    S.desktop_win = NULL;
    S.shutdown_step = 0;

    shutdown_win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(shutdown_win), "");
    gtk_window_set_default_size(GTK_WINDOW(shutdown_win), 500, 350);
    gtk_window_set_position(GTK_WINDOW(shutdown_win), GTK_WIN_POS_CENTER);
    gtk_window_set_decorated(GTK_WINDOW(shutdown_win), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(shutdown_win), FALSE);
    add_class(shutdown_win, "shutdown");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(shutdown_win), box);

    GtkWidget *icon = gtk_label_new("⏻");
    add_class(icon, "shutdown-icon");
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);

    GtkWidget *txt = gtk_label_new("Shutting down...");
    add_class(txt, "shutdown-text");
    gtk_box_pack_start(GTK_BOX(box), txt, FALSE, FALSE, 0);

    GtkWidget *spinner = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(spinner));
    gtk_box_pack_start(GTK_BOX(box), spinner, FALSE, FALSE, 12);

    shutdown_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(shutdown_bar, 220, -1);
    add_class(shutdown_bar, "shutdown-bar");
    gtk_box_pack_start(GTK_BOX(box), shutdown_bar, FALSE, FALSE, 0);

    gtk_widget_show_all(shutdown_win);
    g_timeout_add(100, shutdown_tick, NULL);
}

/* ═══════════════════════════════════════════════════════
   Power Menu (Shutdown / Log Out / Cancel)
   ═══════════════════════════════════════════════════════ */

static void on_power_clicked(GtkWidget *, gpointer desktop_win) {
    GtkWidget *dlg = gtk_dialog_new_with_buttons("", GTK_WINDOW(desktop_win),
        (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        NULL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dlg), 380, 200);
    gtk_window_set_decorated(GTK_WINDOW(dlg), FALSE);
    add_class(dlg, "mode-select");

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(content), vbox);

    GtkWidget *title = gtk_label_new("What would you like to do?");
    add_class(title, "mode-label");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 8);

    GtkWidget *cards = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(cards, GTK_ALIGN_CENTER);

    /* Shutdown card */
    GtkWidget *sd_btn = gtk_button_new();
    add_class(sd_btn, "shutdown-card");
    GtkWidget *sd_inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_halign(sd_inner, GTK_ALIGN_CENTER);
    GtkWidget *sd_e = gtk_label_new("⏻"); add_class(sd_e, "shutdown-card-emoji");
    GtkWidget *sd_l = gtk_label_new("Shut Down"); add_class(sd_l, "shutdown-card-label");
    gtk_box_pack_start(GTK_BOX(sd_inner), sd_e, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sd_inner), sd_l, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(sd_btn), sd_inner);

    /* Log Out card */
    GtkWidget *lo_btn = gtk_button_new();
    add_class(lo_btn, "shutdown-card");
    GtkWidget *lo_inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_halign(lo_inner, GTK_ALIGN_CENTER);
    GtkWidget *lo_e = gtk_label_new("🚪"); add_class(lo_e, "shutdown-card-emoji");
    GtkWidget *lo_l = gtk_label_new("Log Out"); add_class(lo_l, "shutdown-card-label");
    gtk_box_pack_start(GTK_BOX(lo_inner), lo_e, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(lo_inner), lo_l, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(lo_btn), lo_inner);

    /* Cancel card */
    GtkWidget *cc_btn = gtk_button_new();
    add_class(cc_btn, "shutdown-card");
    GtkWidget *cc_inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_halign(cc_inner, GTK_ALIGN_CENTER);
    GtkWidget *cc_e = gtk_label_new("✕"); add_class(cc_e, "shutdown-card-emoji");
    GtkWidget *cc_l = gtk_label_new("Cancel"); add_class(cc_l, "shutdown-card-label");
    gtk_box_pack_start(GTK_BOX(cc_inner), cc_e, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cc_inner), cc_l, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(cc_btn), cc_inner);

    gtk_box_pack_start(GTK_BOX(cards), sd_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cards), lo_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cards), cc_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), cards, FALSE, FALSE, 0);

    gtk_widget_show_all(dlg);

    g_signal_connect_swapped(cc_btn, "clicked", G_CALLBACK(gtk_widget_destroy), dlg);
    g_signal_connect(sd_btn, "clicked", G_CALLBACK(+[](GtkWidget *, gpointer data) {
        GtkWidget *dlg = GTK_WIDGET(data);
        GtkWidget *desktop = gtk_widget_get_toplevel(
            GTK_WIDGET(gtk_window_get_transient_for(GTK_WINDOW(dlg))));
        gtk_widget_destroy(dlg);
        do_shutdown(desktop);
    }), dlg);
    g_signal_connect(lo_btn, "clicked", G_CALLBACK(+[](GtkWidget *, gpointer data) {
        GtkWidget *dlg = GTK_WIDGET(data);
        GtkWidget *desktop = gtk_widget_get_toplevel(
            GTK_WIDGET(gtk_window_get_transient_for(GTK_WINDOW(dlg))));
        gtk_widget_destroy(dlg);
        gtk_widget_destroy(desktop);
        S.desktop_win = NULL;
        show_login();
    }), dlg);
}

/* ═══════════════════════════════════════════════════════
   Desktop Window
   ═══════════════════════════════════════════════════════ */

static void on_lock_clicked(GtkWidget *, gpointer win) {
    gtk_widget_hide(GTK_WIDGET(win));
    show_login();
}

static void on_wallpaper_change(GtkWidget*, gpointer) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choose Wallpaper",
        GTK_WINDOW(S.desktop_win), GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL, "Apply", GTK_RESPONSE_ACCEPT, NULL);
    
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        /* Rewrite CSS file to include wallpaper */
        std::ifstream in("data/theme.css");
        std::string css_str;
        if (in.is_open()) {
            css_str.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            in.close();
            
            size_t pos = css_str.find("window.desktop { background-image:");
            if (pos != std::string::npos) {
                size_t end = css_str.find("}", pos);
                if (end != std::string::npos) css_str.erase(pos, end - pos + 1);
            }
            
            std::string fpath(filename);
            for(char& c : fpath) { if(c == '\\') c = '/'; }
            
            css_str += "\nwindow.desktop { background-image: url('file:///" + fpath + "'); background-size: cover; background-position: center; }\n";
            
            std::ofstream out("data/theme.css");
            out << css_str;
            out.close();
            apply_css(); /* Hot reload */
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_terminal_open(GtkWidget*, gpointer) {
    if (fork() == 0) { setsid(); execlp("./build/gui_terminal", "./build/gui_terminal", (char*)NULL); _exit(1); }
}

static gboolean on_desktop_click(GtkWidget* /*widget*/, GdkEventButton *event, gpointer) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        GtkWidget *menu = gtk_menu_new();
        
        GtkWidget *item1 = gtk_menu_item_new_with_label("🖼️ Change Wallpaper...");
        g_signal_connect(item1, "activate", G_CALLBACK(on_wallpaper_change), NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item1);
        
        GtkWidget *item2 = gtk_menu_item_new_with_label("💻 Open Terminal");
        g_signal_connect(item2, "activate", G_CALLBACK(on_terminal_open), NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item2);
        
        GtkWidget *item3 = gtk_menu_item_new_with_label("🔄 Refresh Desktop");
        g_signal_connect(item3, "activate", G_CALLBACK(on_refresh_clicked), NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item3);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)event);
        return TRUE;
    }
    return FALSE;
}

static void show_desktop() {
    /* Reload tasks fresh */
    load_tasks();

    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(win), "AMS OS");
    gtk_window_set_default_size(GTK_WINDOW(win), 1280, 800);
    gtk_window_maximize(GTK_WINDOW(win));
    add_class(win, "desktop");
    S.desktop_win = win;

    gtk_widget_add_events(win, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(win, "button-press-event", G_CALLBACK(on_desktop_click), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* ── Top Bar ── */
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    add_class(bar, "top-bar");

    GtkWidget *logo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(logo_box, 16);
    
    GdkPixbuf *pixbuf_tb = gdk_pixbuf_new_from_file_at_scale("data/logo.png", 24, 24, TRUE, NULL);
    GtkWidget *logo_img = pixbuf_tb ? gtk_image_new_from_pixbuf(pixbuf_tb) : gtk_image_new_from_icon_name("start-here", GTK_ICON_SIZE_LARGE_TOOLBAR);
    if (pixbuf_tb) g_object_unref(pixbuf_tb);
    
    GtkWidget *logo_lbl = gtk_label_new("AMS OS");
    add_class(logo_lbl, "top-logo");
    gtk_box_pack_start(GTK_BOX(logo_box), logo_img, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(logo_box), logo_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bar), logo_box, FALSE, FALSE, 0);

    /* Power button */
    GtkWidget *pwr = gtk_button_new_with_label("⏻  ");
    add_class(pwr, "power-btn");
    g_signal_connect(pwr, "clicked", G_CALLBACK(on_power_clicked), win);
    gtk_box_pack_end(GTK_BOX(bar), pwr, FALSE, FALSE, 0);

    /* Lock button */
    GtkWidget *lck = gtk_button_new_with_label("🔒 ");
    add_class(lck, "refresh-btn"); /* Reuse refresh styling for the lock button */
    gtk_widget_set_tooltip_text(lck, "Lock OS");
    g_signal_connect(lck, "clicked", G_CALLBACK(on_lock_clicked), win);
    gtk_box_pack_end(GTK_BOX(bar), lck, FALSE, FALSE, 6);

    /* Refresh button */
    GtkWidget *ref = gtk_button_new_with_label("🔄 ");
    add_class(ref, "refresh-btn");
    gtk_widget_set_tooltip_text(ref, "Refresh desktop apps");
    g_signal_connect(ref, "clicked", G_CALLBACK(on_refresh_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(bar), ref, FALSE, FALSE, 0);

    /* Clock */
    S.clock_label = gtk_label_new(get_clock_text().c_str());
    add_class(S.clock_label, "top-clock");
    gtk_box_pack_end(GTK_BOX(bar), S.clock_label, FALSE, FALSE, 10);

    /* Telemetry */
    S.ram_label = gtk_label_new("💾 RAM: --.-%  ");
    add_class(S.ram_label, "top-clock");
    gtk_box_pack_end(GTK_BOX(bar), S.ram_label, FALSE, FALSE, 0);

    S.cpu_label = gtk_label_new("⚙ CPU: --.-%  ");
    add_class(S.cpu_label, "top-clock");
    gtk_box_pack_end(GTK_BOX(bar), S.cpu_label, FALSE, FALSE, 0);

    /* User label */
    GtkWidget *user = gtk_label_new("👤 admin  ");
    add_class(user, "top-user");
    gtk_box_pack_end(GTK_BOX(bar), user, FALSE, FALSE, 4);

    gtk_box_pack_start(GTK_BOX(vbox), bar, FALSE, FALSE, 0);

    /* ── Scrollable content ── */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(content, GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(scroll), content);
    S.desktop_content = content;

    /* ── Search ── */
    GtkWidget *search_wrap = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(search_wrap, GTK_ALIGN_CENTER);
    S.search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(S.search_entry), "🔍  Search applications...");
    add_class(S.search_entry, "search-box");
    gtk_box_pack_start(GTK_BOX(search_wrap), S.search_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), search_wrap, FALSE, FALSE, 0);

    /* ── Icon Grid ── */
    S.flow_box = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(S.flow_box), 6);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(S.flow_box), 4);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(S.flow_box), 10);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(S.flow_box), 10);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(S.flow_box), GTK_SELECTION_NONE);
    gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(S.flow_box), TRUE);
    gtk_widget_set_margin_start(S.flow_box, 40);
    gtk_widget_set_margin_end(S.flow_box, 40);

    gtk_flow_box_set_filter_func(GTK_FLOW_BOX(S.flow_box), filter_func, NULL, NULL);
    g_signal_connect(S.search_entry, "changed", G_CALLBACK(on_search_changed), NULL);

    populate_grid();
    gtk_box_pack_start(GTK_BOX(content), S.flow_box, FALSE, FALSE, 0);

    /* ── Bottom Dock ── */
    GtkWidget *dock_outer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(dock_outer, GTK_ALIGN_CENTER);
    add_class(dock_outer, "dock-outer");

    GtkWidget *dock = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    add_class(dock, "dock");

    int pinned[] = {17, 7, 6, 8, 16, 10, 18, 19, 11, 12, 9};
    for (int p : pinned) {
        for (int i = 0; i < TASK_COUNT; i++) {
            if (TASKS[i].id != p) continue;
            GtkWidget *dbtn = gtk_button_new();
            add_class(dbtn, "dock-btn");
            gtk_widget_set_tooltip_text(dbtn, TASKS[i].name.c_str());
            g_object_set_data(G_OBJECT(dbtn), "task_idx", GINT_TO_POINTER(i));

            GtkWidget *de_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            add_class(de_wrapper, "dock-icon-wrapper");
            gtk_widget_set_halign(de_wrapper, GTK_ALIGN_CENTER);
            gtk_widget_set_valign(de_wrapper, GTK_ALIGN_CENTER);

            const char *icon_name = get_icon_for_app(TASKS[i].name);
            GtkWidget *de = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_DND);
            add_class(de, "dock-emoji");
            gtk_box_pack_start(GTK_BOX(de_wrapper), de, TRUE, TRUE, 0);

            gtk_container_add(GTK_CONTAINER(dbtn), de_wrapper);
            g_signal_connect(dbtn, "clicked", G_CALLBACK(on_app_click), NULL);
            gtk_box_pack_start(GTK_BOX(dock), dbtn, FALSE, FALSE, 2);
            break;
        }
    }

    gtk_box_pack_start(GTK_BOX(dock_outer), dock, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), dock_outer, FALSE, FALSE, 0);

    /* Auto-refresh when window gains focus */
    g_signal_connect(win, "focus-in-event", G_CALLBACK(on_desktop_focus), NULL);

    g_timeout_add_seconds(1, tick_clock, NULL);
    g_timeout_add_seconds(1, tick_telemetry, NULL); /* Ultra-smooth 1s updates */
    gtk_widget_show_all(win);
}

/* ═══════════════════════════════════════════════════════
   Splash Screen
   ═══════════════════════════════════════════════════════ */

static gboolean splash_tick(gpointer) {
    S.splash_step++;
    if (S.splash_step >= 30) {
        if (S.splash_window) { gtk_widget_destroy(S.splash_window); S.splash_window = NULL; }
        show_desktop();
        return G_SOURCE_REMOVE;
    }
    if (S.progress_bar)
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(S.progress_bar), (double)S.splash_step / 30.0);
    return G_SOURCE_CONTINUE;
}

static void show_splash() {
    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_default_size(GTK_WINDOW(win), 520, 380);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    add_class(win, "splash");
    S.splash_window = win; S.splash_step = 0;

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(win), box);

    GdkPixbuf *pixbuf_sp = gdk_pixbuf_new_from_file_at_scale("data/logo.png", 128, 128, TRUE, NULL);
    GtkWidget *icon = pixbuf_sp ? gtk_image_new_from_pixbuf(pixbuf_sp) : gtk_label_new("⚛️");
    if (pixbuf_sp) g_object_unref(pixbuf_sp);
    
    add_class(icon, "splash-logo");
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    GtkWidget *name = gtk_label_new("AMS  OS"); add_class(name, "splash-name");
    gtk_box_pack_start(GTK_BOX(box), name, FALSE, FALSE, 0);
    GtkWidget *ver = gtk_label_new("Atomic Management System  ·  v3.0"); add_class(ver, "splash-ver");
    gtk_box_pack_start(GTK_BOX(box), ver, FALSE, FALSE, 0);

    S.progress_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(S.progress_bar, 240, -1);
    add_class(S.progress_bar, "splash-bar");
    gtk_widget_set_margin_top(S.progress_bar, 30);
    gtk_box_pack_start(GTK_BOX(box), S.progress_bar, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
    g_timeout_add(100, splash_tick, NULL);
}

/* ═══════════════════════════════════════════════════════
   Login Screen
   ═══════════════════════════════════════════════════════ */

static GtkWidget *login_user_entry = NULL, *login_pass_entry = NULL;

static void on_login_clicked(GtkWidget *, gpointer win) {
    const char *user = gtk_entry_get_text(GTK_ENTRY(login_user_entry));
    const char *pass = gtk_entry_get_text(GTK_ENTRY(login_pass_entry));

    if ((strcmp(user, "admin") == 0 && strcmp(pass, "admin") == 0) ||
        (strcmp(user, "saim") == 0 && strcmp(pass, "1234") == 0) ||
        (strcmp(user, "mohsin") == 0 && strcmp(pass, "1234") == 0) ||
        (strcmp(user, "ahmed") == 0 && strcmp(pass, "1234") == 0)) {
        gtk_widget_destroy(GTK_WIDGET(win));
        if (S.desktop_win) {
            gtk_widget_show_all(S.desktop_win);
        } else {
            show_splash();
        }
    } else {
        gtk_label_set_text(GTK_LABEL(S.login_error), "❌ Incorrect username or password");
    }
}

static gboolean on_login_key(GtkWidget *, GdkEventKey *ev, gpointer win) {
    if (ev->keyval == GDK_KEY_Return) { on_login_clicked(NULL, win); return TRUE; }
    return FALSE;
}

static void show_login() {
    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(win), "AMS OS — Login");
    gtk_window_set_default_size(GTK_WINDOW(win), 440, 460);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    add_class(win, "login");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(win), box);

    GtkWidget *avatar = gtk_label_new("👤"); add_class(avatar, "login-avatar");
    gtk_box_pack_start(GTK_BOX(box), avatar, FALSE, FALSE, 0);

    GtkWidget *title = gtk_label_new("AMS OS"); add_class(title, "login-title");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *sub = gtk_label_new("Sign in to continue"); add_class(sub, "mode-desc");
    gtk_box_pack_start(GTK_BOX(box), sub, FALSE, FALSE, 8);

    login_user_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(login_user_entry), "Username");
    gtk_entry_set_text(GTK_ENTRY(login_user_entry), "admin");
    add_class(login_user_entry, "login-entry");
    gtk_box_pack_start(GTK_BOX(box), login_user_entry, FALSE, FALSE, 4);

    login_pass_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(login_pass_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(login_pass_entry), FALSE);
    add_class(login_pass_entry, "login-entry");
    gtk_box_pack_start(GTK_BOX(box), login_pass_entry, FALSE, FALSE, 4);

    S.login_error = gtk_label_new(""); add_class(S.login_error, "login-error");
    gtk_box_pack_start(GTK_BOX(box), S.login_error, FALSE, FALSE, 0);

    GtkWidget *login_btn = gtk_button_new_with_label("Sign In");
    add_class(login_btn, "login-btn");
    g_signal_connect(login_btn, "clicked", G_CALLBACK(on_login_clicked), win);
    gtk_box_pack_start(GTK_BOX(box), login_btn, FALSE, FALSE, 8);

    GtkWidget *hint = gtk_label_new("Credentials: admin/admin  ·  saim/1234  ·  mohsin/1234  ·  ahmed/1234");
    add_class(hint, "login-hint");
    gtk_box_pack_start(GTK_BOX(box), hint, FALSE, FALSE, 0);

    g_signal_connect(win, "key-press-event", G_CALLBACK(on_login_key), win);
    gtk_widget_show_all(win);
}

/* ═══════════════════════════════════════════════════════
   Mode Selector
   ═══════════════════════════════════════════════════════ */

static void on_gui_clicked(GtkWidget *, gpointer w) {
    gtk_widget_destroy(GTK_WIDGET(w));
    show_login();
}

static void on_cmd_clicked(GtkWidget *, gpointer w) {
    std::string cmd = "./OS";
    for (int i = 1; i < S.argc; i++) { cmd += " "; cmd += S.argv[i]; }
    gtk_widget_destroy(GTK_WIDGET(w));
    pid_t pid = fork();
    if (pid == 0) {
        execlp("gnome-terminal", "gnome-terminal", "--title", "AMS OS — Terminal Mode",
               "--geometry", "120x40", "--", "bash", "-c", cmd.c_str(), NULL);
        _exit(1);
    }
    g_application_quit(G_APPLICATION(S.app));
}

static GtkWidget *make_card(const char *emoji, const char *label, const char *desc) {
    GtkWidget *btn = gtk_button_new(); add_class(btn, "mode-card");
    GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_halign(inner, GTK_ALIGN_CENTER);
    GtkWidget *e = gtk_label_new(emoji); add_class(e, "mode-emoji");
    GtkWidget *l = gtk_label_new(label); add_class(l, "mode-label");
    GtkWidget *d = gtk_label_new(desc);  add_class(d, "mode-desc");
    gtk_box_pack_start(GTK_BOX(inner), e, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(inner), l, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(inner), d, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(btn), inner);
    return btn;
}

static void show_mode_selector() {
    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(win), "AMS OS — Select Mode");
    gtk_window_set_default_size(GTK_WINDOW(win), 560, 380);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    add_class(win, "mode-select");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *title = gtk_label_new("AMS OS"); add_class(title, "mode-title");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    GtkWidget *sub = gtk_label_new("Choose your interface"); add_class(sub, "mode-subtitle");
    gtk_box_pack_start(GTK_BOX(vbox), sub, FALSE, FALSE, 0);

    GtkWidget *cards = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_halign(cards, GTK_ALIGN_CENTER);
    GtkWidget *gui_card = make_card("🖥️", "GUI Mode", "Visual desktop interface");
    GtkWidget *cmd_card = make_card("⌨️",  "CMD Mode", "Classic terminal interface");
    g_signal_connect(gui_card, "clicked", G_CALLBACK(on_gui_clicked), win);
    g_signal_connect(cmd_card, "clicked", G_CALLBACK(on_cmd_clicked), win);
    gtk_box_pack_start(GTK_BOX(cards), gui_card, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cards), cmd_card, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), cards, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
}

/* ═══════════════════════════════════════════════════════
   Application Entry
   ═══════════════════════════════════════════════════════ */

static void on_activate(GtkApplication *, gpointer) {
    load_tasks();
    apply_css();
    show_mode_selector();
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    signal(SIGUSR1, handle_sigusr1); /* Listen for Copilot app-creation triggers */
    g_timeout_add(500, check_refresh_flag, NULL);

    S.argc = argc; S.argv = argv;
    S.app = gtk_application_new("com.ams.os.desktop", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(S.app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(S.app), 0, NULL);
    g_object_unref(S.app);
    return status;
}
