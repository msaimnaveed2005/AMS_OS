/*
AMS OS Desktop — Graphical Operating System Interface
A GTK3-based desktop environment for the AMS OS simulator.
Provides a macOS-inspired Launchpad interface with app icons,
taskbar, search filtering, and task launching into terminal windows.

Build: g++ -std=c++17 $(pkg-config --cflags gtk+-3.0) gui/ams_desktop.cpp $(pkg-config --libs gtk+-3.0) -o build/ams_desktop
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

/* ═══════════════════════════════════════════════════════
   Task Registry — maps to tasks/ executables
   ═══════════════════════════════════════════════════════ */

struct TaskEntry {
    int         id;
    const char *name;
    const char *emoji;
    const char *exec_path;
};

static const TaskEntry TASKS[] = {
    { 1, "Create File",      "📄", "./build/create_file"},
    { 2, "Delete File",      "🗑️",  "./build/delete_file"},
    { 3, "Copy File",        "📋", "./build/file_copy"},
    { 4, "Move File",        "📦", "./build/move_file"},
    { 5, "File Info",        "ℹ️",  "./build/file_info"},
    { 6, "Notepad",          "📝", "./build/notepad"},
    { 7, "Calculator",       "🧮", "./build/calculator"},
    { 8, "Digital Clock",    "🕐", "./build/clock"},
    { 9, "System Info",      "💻", "./build/system_info"},
    {10, "Snake Game",       "🐍", "./build/snake"},
    {11, "Minesweeper",      "💣", "./build/minesweeper"},
    {12, "Music Player",     "🎵", "./build/music_player"},
    {13, "Downloads",        "⬇️",  "./build/download_simulator"},
    {14, "Task Manager",     "📊", "./build/task_manager"},
    {15, "Process Killer",   "⚡", "./build/process_killer"},
    {16, "Calendar",         "📅", "./build/calendar"},
};
static const int TASK_COUNT = sizeof(TASKS) / sizeof(TASKS[0]);

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
    int             splash_step;
    int             argc;
    char          **argv;
};

static AppState S = {};

/* ═══════════════════════════════════════════════════════
   CSS Theme — Premium dark desktop aesthetic
   ═══════════════════════════════════════════════════════ */

static const char *APP_CSS = R"CSS(

/* ── Mode Selector ── */
window.mode-select {
    background-color: #08081a;
}
.mode-title {
    font-size: 48px;
    font-weight: 800;
    color: #a78bfa;
    margin-top: 36px;
}
.mode-subtitle {
    font-size: 13px;
    color: rgba(255,255,255,0.4);
    margin-bottom: 32px;
}
.mode-card {
    background-color: rgba(255,255,255,0.05);
    border: 1px solid rgba(255,255,255,0.08);
    border-radius: 22px;
    padding: 36px 52px;
    margin: 10px;
    min-width: 180px;
    min-height: 120px;
    transition: all 200ms ease-in-out;
}
.mode-card:hover {
    background-color: rgba(139,92,246,0.18);
    border-color: rgba(139,92,246,0.45);
    box-shadow: 0 8px 40px rgba(139,92,246,0.25);
}
.mode-emoji { font-size: 52px; }
.mode-label {
    font-size: 18px;
    font-weight: 700;
    color: white;
    margin-top: 10px;
}
.mode-desc {
    font-size: 11px;
    color: rgba(255,255,255,0.45);
    margin-top: 2px;
}

/* ── Splash Screen ── */
window.splash { background-color: #06060f; }
.splash-logo {
    font-size: 72px;
    margin-top: 50px;
}
.splash-name {
    font-size: 18px;
    font-weight: 700;
    color: rgba(255,255,255,0.75);
    letter-spacing: 8px;
    margin-top: 10px;
}
.splash-ver {
    font-size: 11px;
    color: rgba(255,255,255,0.25);
    margin-top: 20px;
}
progressbar.splash-bar trough {
    background-color: rgba(255,255,255,0.06);
    border-radius: 3px;
    min-height: 4px;
}
progressbar.splash-bar progress {
    background-image: linear-gradient(to right, #6366f1, #a855f7, #ec4899);
    border-radius: 3px;
    min-height: 4px;
}

/* ── Desktop Wallpaper ── */
window.desktop {
    background-image: linear-gradient(
        160deg,
        #0f0c29 0%,
        #1a1050 12%,
        #302b63 30%,
        #24243e 48%,
        #4a1942 62%,
        #b91c1c 78%,
        #ea580c 88%,
        #f59e0b 100%
    );
}

/* ── Top Bar ── */
.top-bar {
    background-color: rgba(0,0,0,0.6);
    padding: 3px 16px;
    min-height: 26px;
}
.top-logo {
    color: rgba(255,255,255,0.92);
    font-size: 13px;
    font-weight: 700;
}
.top-clock {
    color: rgba(255,255,255,0.88);
    font-size: 13px;
    font-weight: 500;
}

/* ── Search ── */
.search-box {
    background-color: rgba(255,255,255,0.12);
    border: 1px solid rgba(255,255,255,0.06);
    border-radius: 10px;
    color: white;
    padding: 8px 18px;
    font-size: 14px;
    min-width: 300px;
    margin-top: 24px;
    margin-bottom: 20px;
    caret-color: white;
}
.search-box:focus {
    background-color: rgba(255,255,255,0.18);
    border-color: rgba(139,92,246,0.5);
}

/* ── App Icons ── */
.app-btn {
    background-color: transparent;
    border: none;
    border-radius: 18px;
    padding: 12px;
    min-width: 100px;
    min-height: 100px;
    transition: all 150ms ease-in-out;
    box-shadow: none;
}
.app-btn:hover {
    background-color: rgba(255,255,255,0.12);
}
.app-btn:active {
    background-color: rgba(255,255,255,0.22);
}
.app-icon-emoji { font-size: 46px; }
.app-icon-name {
    color: white;
    font-size: 11px;
    font-weight: 500;
    margin-top: 5px;
    text-shadow: 0 1px 3px rgba(0,0,0,0.6);
}

/* ── Bottom Dock ── */
.dock-outer { margin: 0px 0px 12px 0px; }
.dock {
    background-color: rgba(255,255,255,0.10);
    border: 1px solid rgba(255,255,255,0.15);
    border-radius: 20px;
    padding: 5px 12px;
}
.dock-btn {
    background-color: transparent;
    border: none;
    border-radius: 14px;
    padding: 5px 10px;
    min-width: 50px;
    min-height: 50px;
    transition: all 150ms ease-in-out;
}
.dock-btn:hover {
    background-color: rgba(255,255,255,0.15);
}
.dock-emoji { font-size: 28px; }

/* ── Power button ── */
.power-btn {
    background-color: transparent;
    border: none;
    padding: 2px 8px;
    border-radius: 6px;
    color: rgba(255,255,255,0.7);
    font-size: 14px;
}
.power-btn:hover {
    background-color: rgba(255,60,60,0.3);
    color: white;
}

)CSS";

/* ═══════════════════════════════════════════════════════
   Utilities
   ═══════════════════════════════════════════════════════ */

static std::string get_clock_text() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%a %b %d   %I:%M %p", t);
    return buf;
}

static void launch_in_terminal(const char *path, const char *title) {
    pid_t pid = fork();
    if (pid == 0) {
        setsid();
        execlp("gnome-terminal", "gnome-terminal",
               "--title", title, "--", path, title, NULL);
        execlp("xfce4-terminal", "xfce4-terminal",
               "--disable-server", "--title", title,
               "--execute", path, title, NULL);
        execlp("x-terminal-emulator", "x-terminal-emulator",
               "-T", title, "-e", path, title, NULL);
        _exit(1);
    }
}

static void apply_css() {
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_data(p, APP_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}

static void add_class(GtkWidget *w, const char *cls) {
    gtk_style_context_add_class(gtk_widget_get_style_context(w), cls);
}

/* ═══════════════════════════════════════════════════════
   Clock tick (every second)
   ═══════════════════════════════════════════════════════ */

static gboolean tick_clock(gpointer) {
    if (S.clock_label && GTK_IS_LABEL(S.clock_label))
        gtk_label_set_text(GTK_LABEL(S.clock_label), get_clock_text().c_str());
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
    const TaskEntry *task = (const TaskEntry *)g_object_get_data(G_OBJECT(btn), "task");
    if (!task) return TRUE;

    std::string n(task->name), q(query);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    return n.find(q) != std::string::npos ? TRUE : FALSE;
}

static void on_search_changed(GtkEntry *, gpointer) {
    if (S.flow_box)
        gtk_flow_box_invalidate_filter(GTK_FLOW_BOX(S.flow_box));
}

/* ═══════════════════════════════════════════════════════
   Task click
   ═══════════════════════════════════════════════════════ */

static void on_app_click(GtkWidget *, gpointer data) {
    const TaskEntry *t = (const TaskEntry *)data;
    launch_in_terminal(t->exec_path, t->name);
}

/* ═══════════════════════════════════════════════════════
   Desktop Window
   ═══════════════════════════════════════════════════════ */

static void show_desktop() {
    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(win), "AMS OS");
    gtk_window_set_default_size(GTK_WINDOW(win), 1280, 800);
    gtk_window_maximize(GTK_WINDOW(win));
    add_class(win, "desktop");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* ── Top Bar ── */
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    add_class(bar, "top-bar");

    GtkWidget *logo = gtk_label_new("  ⚛  AMS OS");
    add_class(logo, "top-logo");
    gtk_box_pack_start(GTK_BOX(bar), logo, FALSE, FALSE, 0);

    /* Power button */
    GtkWidget *pwr = gtk_button_new_with_label("⏻  ");
    add_class(pwr, "power-btn");
    g_signal_connect_swapped(pwr, "clicked", G_CALLBACK(gtk_window_close), win);
    gtk_box_pack_end(GTK_BOX(bar), pwr, FALSE, FALSE, 0);

    /* Clock */
    S.clock_label = gtk_label_new(get_clock_text().c_str());
    add_class(S.clock_label, "top-clock");
    gtk_box_pack_end(GTK_BOX(bar), S.clock_label, FALSE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(vbox), bar, FALSE, FALSE, 0);

    /* ── Scrollable content ── */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(content, GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(scroll), content);

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

    for (int i = 0; i < TASK_COUNT; i++) {
        GtkWidget *btn = gtk_button_new();
        add_class(btn, "app-btn");
        g_object_set_data(G_OBJECT(btn), "task", (gpointer)&TASKS[i]);

        GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_halign(inner, GTK_ALIGN_CENTER);

        GtkWidget *emoji = gtk_label_new(TASKS[i].emoji);
        add_class(emoji, "app-icon-emoji");
        gtk_box_pack_start(GTK_BOX(inner), emoji, FALSE, FALSE, 0);

        GtkWidget *name = gtk_label_new(TASKS[i].name);
        add_class(name, "app-icon-name");
        gtk_label_set_max_width_chars(GTK_LABEL(name), 14);
        gtk_label_set_ellipsize(GTK_LABEL(name), PANGO_ELLIPSIZE_END);
        gtk_box_pack_start(GTK_BOX(inner), name, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(btn), inner);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_app_click), (gpointer)&TASKS[i]);
        gtk_container_add(GTK_CONTAINER(S.flow_box), btn);
    }

    gtk_box_pack_start(GTK_BOX(content), S.flow_box, FALSE, FALSE, 0);

    /* ── Bottom Dock ── */
    GtkWidget *dock_outer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(dock_outer, GTK_ALIGN_CENTER);
    add_class(dock_outer, "dock-outer");

    GtkWidget *dock = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    add_class(dock, "dock");

    int pinned[] = {7, 6, 8, 16, 10, 11, 12, 9};
    for (int p : pinned) {
        for (int i = 0; i < TASK_COUNT; i++) {
            if (TASKS[i].id != p) continue;
            GtkWidget *dbtn = gtk_button_new();
            add_class(dbtn, "dock-btn");
            gtk_widget_set_tooltip_text(dbtn, TASKS[i].name);

            GtkWidget *de = gtk_label_new(TASKS[i].emoji);
            add_class(de, "dock-emoji");
            gtk_container_add(GTK_CONTAINER(dbtn), de);

            g_signal_connect(dbtn, "clicked", G_CALLBACK(on_app_click), (gpointer)&TASKS[i]);
            gtk_box_pack_start(GTK_BOX(dock), dbtn, FALSE, FALSE, 2);
            break;
        }
    }

    gtk_box_pack_start(GTK_BOX(dock_outer), dock, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), dock_outer, FALSE, FALSE, 0);

    g_timeout_add_seconds(1, tick_clock, NULL);
    gtk_widget_show_all(win);
}

/* ═══════════════════════════════════════════════════════
   Splash Screen
   ═══════════════════════════════════════════════════════ */

static gboolean splash_tick(gpointer) {
    S.splash_step++;
    if (S.splash_step >= 30) {
        if (S.splash_window) {
            gtk_widget_destroy(S.splash_window);
            S.splash_window = NULL;
        }
        show_desktop();
        return G_SOURCE_REMOVE;
    }
    if (S.progress_bar)
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(S.progress_bar),
                                      (double)S.splash_step / 30.0);
    return G_SOURCE_CONTINUE;
}

static void show_splash() {
    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(win), "AMS OS");
    gtk_window_set_default_size(GTK_WINDOW(win), 520, 380);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    add_class(win, "splash");
    S.splash_window = win;
    S.splash_step = 0;

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(win), box);

    GtkWidget *icon = gtk_label_new("⚛️");
    add_class(icon, "splash-logo");
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);

    GtkWidget *name = gtk_label_new("AMS  OS");
    add_class(name, "splash-name");
    gtk_box_pack_start(GTK_BOX(box), name, FALSE, FALSE, 0);

    GtkWidget *ver = gtk_label_new("Atomic Management System  ·  v3.0");
    add_class(ver, "splash-ver");
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
   Mode Selector
   ═══════════════════════════════════════════════════════ */

static void on_gui_clicked(GtkWidget *, gpointer w) {
    gtk_widget_destroy(GTK_WIDGET(w));
    show_splash();
}

static void on_cmd_clicked(GtkWidget *, gpointer w) {
    std::string cmd = "./OS";
    for (int i = 1; i < S.argc; i++) { cmd += " "; cmd += S.argv[i]; }

    gtk_widget_destroy(GTK_WIDGET(w));

    pid_t pid = fork();
    if (pid == 0) {
        execlp("gnome-terminal", "gnome-terminal",
               "--title", "AMS OS — Terminal Mode",
               "--geometry", "120x40",
               "--", "bash", "-c", cmd.c_str(), NULL);
        _exit(1);
    }
    g_application_quit(G_APPLICATION(S.app));
}

static GtkWidget *make_card(const char *emoji, const char *label,
                            const char *desc) {
    GtkWidget *btn = gtk_button_new();
    add_class(btn, "mode-card");

    GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_halign(inner, GTK_ALIGN_CENTER);

    GtkWidget *e = gtk_label_new(emoji);
    add_class(e, "mode-emoji");
    gtk_box_pack_start(GTK_BOX(inner), e, FALSE, FALSE, 0);

    GtkWidget *l = gtk_label_new(label);
    add_class(l, "mode-label");
    gtk_box_pack_start(GTK_BOX(inner), l, FALSE, FALSE, 0);

    GtkWidget *d = gtk_label_new(desc);
    add_class(d, "mode-desc");
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

    GtkWidget *title = gtk_label_new("AMS OS");
    add_class(title, "mode-title");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    GtkWidget *sub = gtk_label_new("Choose your interface");
    add_class(sub, "mode-subtitle");
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
    apply_css();
    show_mode_selector();
}

int main(int argc, char *argv[]) {
    /* Auto-reap child processes (gnome-terminal forks internally) */
    signal(SIGCHLD, SIG_IGN);

    S.argc = argc;
    S.argv = argv;

    S.app = gtk_application_new("com.ams.os.desktop", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(S.app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(S.app), 0, NULL);
    g_object_unref(S.app);
    return status;
}
