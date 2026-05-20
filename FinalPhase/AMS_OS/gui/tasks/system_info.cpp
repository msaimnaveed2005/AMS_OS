/* AMS OS — GUI System Info */
#include "../gui_theme.h"

static std::string read_file_line(const char *path, const char *prefix = NULL) {
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
        if (!prefix) return line;
        if (line.find(prefix) == 0) {
            size_t p = line.find(':');
            if (p != std::string::npos) {
                std::string val = line.substr(p + 1);
                val.erase(0, val.find_first_not_of(" \t"));
                return val;
            }
        }
    }
    return "N/A";
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "System Info", "utilities-system-monitor", 480, 450);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(win), scroll);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(vbox, 20); gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 16);   gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(scroll), vbox);

    /* System card */
    GtkWidget *sys_card = ams_card("🖥  System");
    char hostname[256] = "N/A"; gethostname(hostname, sizeof(hostname));
    struct utsname un; uname(&un);
    gtk_box_pack_start(GTK_BOX(sys_card), ams_info_row("Hostname", hostname), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sys_card), ams_info_row("OS", un.sysname), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sys_card), ams_info_row("Kernel", un.release), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sys_card), ams_info_row("Architecture", un.machine), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), sys_card, FALSE, FALSE, 0);

    /* Hardware card */
    GtkWidget *hw_card = ams_card("⚙  Hardware");
    std::string cpu = read_file_line("/proc/cpuinfo", "model name");
    std::string cores = read_file_line("/proc/cpuinfo", "cpu cores");
    gtk_box_pack_start(GTK_BOX(hw_card), ams_info_row("CPU", cpu.c_str()), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hw_card), ams_info_row("CPU Cores", cores.c_str()), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hw_card, FALSE, FALSE, 0);

    /* Memory card */
    GtkWidget *mem_card = ams_card("💾  Memory");
    std::string mem_total = read_file_line("/proc/meminfo", "MemTotal");
    std::string mem_avail = read_file_line("/proc/meminfo", "MemAvailable");
    std::string swap = read_file_line("/proc/meminfo", "SwapTotal");
    gtk_box_pack_start(GTK_BOX(mem_card), ams_info_row("Total RAM", mem_total.c_str()), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mem_card), ams_info_row("Available", mem_avail.c_str()), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mem_card), ams_info_row("Swap", swap.c_str()), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), mem_card, FALSE, FALSE, 0);

    /* Uptime card */
    GtkWidget *up_card = ams_card("⏱  Uptime");
    std::string uptime_str = read_file_line("/proc/uptime");
    double up_secs = 0;
    if (uptime_str != "N/A") up_secs = std::stod(uptime_str);
    int hours = (int)(up_secs / 3600), mins = (int)((up_secs - hours*3600) / 60);
    char upbuf[64]; snprintf(upbuf, sizeof(upbuf), "%d hours, %d minutes", hours, mins);
    gtk_box_pack_start(GTK_BOX(up_card), ams_info_row("System Uptime", upbuf), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), up_card, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.sysinfo", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
