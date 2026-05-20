/* AMS OS — GUI Task Manager (live process list from /proc) */
#include "../gui_theme.h"

enum { COL_PID, COL_NAME, COL_STATE, COL_MEM, NUM_COLS };
static GtkListStore *store = NULL;

static void refresh_list() {
    gtk_list_store_clear(store);
    DIR *dir = opendir("/proc");
    if (!dir) return;
    struct dirent *ent;
    while ((ent = readdir(dir))) {
        if (ent->d_type != DT_DIR) continue;
        char *end; long pid = strtol(ent->d_name, &end, 10);
        if (*end != '\0') continue;

        char path[256]; snprintf(path, sizeof(path), "/proc/%ld/status", pid);
        std::ifstream f(path);
        if (!f.is_open()) continue;

        std::string name = "?", state = "?", mem_str = "0 kB";
        std::string line;
        while (std::getline(f, line)) {
            if (line.find("Name:") == 0) { name = line.substr(6); name.erase(0, name.find_first_not_of(" \t")); }
            if (line.find("State:") == 0) { state = line.substr(7); state.erase(0, state.find_first_not_of(" \t")); }
            if (line.find("VmRSS:") == 0) { mem_str = line.substr(7); mem_str.erase(0, mem_str.find_first_not_of(" \t")); }
        }

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            COL_PID, (int)pid,
            COL_NAME, name.c_str(),
            COL_STATE, state.c_str(),
            COL_MEM, mem_str.c_str(), -1);
    }
    closedir(dir);
}

static gboolean auto_refresh(gpointer) { refresh_list(); return G_SOURCE_CONTINUE; }

static void on_refresh(GtkWidget *, gpointer) { refresh_list(); }

static void on_end_process(GtkWidget *, gpointer data) {
    GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(data));
    GtkTreeIter iter;
    GtkTreeModel *model;
    if (gtk_tree_selection_get_selected(sel, &model, &iter)) {
        int pid;
        gtk_tree_model_get(model, &iter, COL_PID, &pid, -1);
        kill(pid, SIGTERM);
        refresh_list();
    }
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "Task Manager", "utilities-system-monitor", 650, 500);

    /* Header buttons */
    GtkWidget *hbar = gtk_window_get_titlebar(GTK_WINDOW(win));
    GtkWidget *ref_btn = gtk_button_new_with_label("🔄 Refresh");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), ref_btn);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 12); gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 8);    gtk_widget_set_margin_bottom(vbox, 12);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* Tree view */
    store = gtk_list_store_new(NUM_COLS, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkCellRenderer *r;
    r = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree),
        gtk_tree_view_column_new_with_attributes("PID", r, "text", COL_PID, NULL));
    r = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree),
        gtk_tree_view_column_new_with_attributes("Name", r, "text", COL_NAME, NULL));
    r = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree),
        gtk_tree_view_column_new_with_attributes("State", r, "text", COL_STATE, NULL));
    r = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree),
        gtk_tree_view_column_new_with_attributes("Memory", r, "text", COL_MEM, NULL));

    /* Make columns resizable */
    GList *cols = gtk_tree_view_get_columns(GTK_TREE_VIEW(tree));
    for (GList *c = cols; c; c = c->next)
        gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(c->data), TRUE);
    g_list_free(cols);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), tree);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    /* Bottom bar */
    GtkWidget *bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *end_btn = gtk_button_new_with_label("⚡ End Process");
    ams_css(end_btn, "destructive-action");
    g_signal_connect(end_btn, "clicked", G_CALLBACK(on_end_process), tree);
    gtk_box_pack_end(GTK_BOX(bottom), end_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), bottom, FALSE, FALSE, 0);

    g_signal_connect(ref_btn, "clicked", G_CALLBACK(on_refresh), NULL);

    refresh_list();
    g_timeout_add_seconds(3, auto_refresh, NULL);
    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.taskmgr", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
