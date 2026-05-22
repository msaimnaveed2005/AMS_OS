/* AMS OS — GUI File Explorer */
#include "../gui_theme.h"
#include <gtk/gtk.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <algorithm>

static GtkWidget *path_entry;
static GtkWidget *file_list;
static GtkListStore *list_store;
static std::string current_path = ".";

enum { COL_ICON, COL_NAME, COL_SIZE, COL_TYPE, COL_PATH, NUM_COLS };

static void load_directory(const std::string& path) {
    gtk_list_store_clear(list_store);
    gtk_entry_set_text(GTK_ENTRY(path_entry), path.c_str());
    current_path = path;
    
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        GtkTreeIter iter;
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter, COL_ICON, "dialog-error", COL_NAME, "Permission Denied", COL_SIZE, "", COL_TYPE, "", COL_PATH, "", -1);
        return;
    }
    
    struct dirent *ent;
    std::vector<std::pair<std::string, bool>> items; // name, is_dir
    while ((ent = readdir(dir))) {
        std::string name = ent->d_name;
        if (name == ".") continue;
        
        std::string full_path = path + "/" + name;
        struct stat st;
        bool is_dir = (ent->d_type == DT_DIR);
        if (ent->d_type == DT_UNKNOWN && stat(full_path.c_str(), &st) == 0) {
            is_dir = S_ISDIR(st.st_mode);
        }
        items.push_back({name, is_dir});
    }
    closedir(dir);
    
    /* Sort: directories first, then alphabetical */
    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    
    for (const auto& item : items) {
        std::string full = path + "/" + item.first;
        struct stat st;
        std::string size_str = "";
        if (stat(full.c_str(), &st) == 0 && !item.second) {
            char buf[32];
            if (st.st_size < 1024) snprintf(buf, sizeof(buf), "%ld B", (long)st.st_size);
            else if (st.st_size < 1024*1024) snprintf(buf, sizeof(buf), "%.1f KB", st.st_size/1024.0);
            else snprintf(buf, sizeof(buf), "%.1f MB", st.st_size/(1024.0*1024.0));
            size_str = buf;
        }
        
        const char *icon = item.second ? "folder" : "text-x-generic";
        const char *type = item.second ? "Directory" : "File";
        
        if (!item.second && item.first.find(".cpp") != std::string::npos) icon = "text-x-script";
        if (!item.second && item.first.find("gui_") != std::string::npos) icon = "application-x-executable";
        
        GtkTreeIter iter;
        gtk_list_store_append(list_store, &iter);
        gtk_list_store_set(list_store, &iter,
            COL_ICON, icon,
            COL_NAME, item.first.c_str(),
            COL_SIZE, size_str.c_str(),
            COL_TYPE, type,
            COL_PATH, full.c_str(),
            -1);
    }
}

static void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn* /*column*/, gpointer /*user_data*/) {
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gchar *file_path, *type;
        gtk_tree_model_get(model, &iter, COL_PATH, &file_path, COL_TYPE, &type, -1);
        
        if (std::string(type) == "Directory") {
            load_directory(file_path);
        } else {
            /* Open file based on type */
            std::string pstr = file_path;
            if (pstr.find("gui_") != std::string::npos && pstr.find(".cpp") == std::string::npos) {
                /* It's likely an executable app, launch it */
                if (fork() == 0) {
                    setsid();
                    execlp(file_path, file_path, (char*)NULL);
                    _exit(1);
                }
            } else if (pstr.find(".png") != std::string::npos || pstr.find(".jpg") != std::string::npos || pstr.find(".jpeg") != std::string::npos) {
                /* Open in Photo Viewer */
                if (fork() == 0) {
                    setsid();
                    execlp("./build/gui_photo_viewer", "./build/gui_photo_viewer", file_path, (char*)NULL);
                    _exit(1);
                }
            } else {
                /* Open in notepad */
                if (fork() == 0) {
                    setsid();
                    execlp("./build/gui_notepad", "./build/gui_notepad", file_path, (char*)NULL);
                    _exit(1);
                }
            }
        }
        g_free(file_path);
        g_free(type);
    }
}

static void on_up_clicked(GtkWidget*, gpointer) {
    if (current_path == "/") return;
    size_t p = current_path.find_last_of('/');
    if (p == std::string::npos) load_directory(".");
    else if (p == 0) load_directory("/");
    else load_directory(current_path.substr(0, p));
}

static void on_path_activate(GtkEntry *entry, gpointer) {
    const char *p = gtk_entry_get_text(entry);
    if (p && strlen(p) > 0) load_directory(p);
}

static void on_shortcut_clicked(GtkButton* /*btn*/, gpointer path) {
    load_directory((const char*)path);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "File Explorer", "system-file-manager", 750, 500);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    
    /* Top Toolbar */
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start(toolbar, 8); gtk_widget_set_margin_end(toolbar, 8);
    gtk_widget_set_margin_top(toolbar, 8); gtk_widget_set_margin_bottom(toolbar, 8);
    
    GtkWidget *up_btn = gtk_button_new_from_icon_name("go-up", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(up_btn, "clicked", G_CALLBACK(on_up_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), up_btn, FALSE, FALSE, 0);
    
    path_entry = gtk_entry_new();
    gtk_widget_set_hexpand(path_entry, TRUE);
    g_signal_connect(path_entry, "activate", G_CALLBACK(on_path_activate), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), path_entry, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    
    /* Paned layout */
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);
    
    /* Shortcuts sidebar */
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(sidebar, 8); gtk_widget_set_margin_end(sidebar, 8);
    gtk_widget_set_margin_top(sidebar, 8);
    gtk_widget_set_size_request(sidebar, 150, -1);
    
    auto add_shortcut = [&](const char *label, const char *icon, const char *path) {
        GtkWidget *btn = gtk_button_new();
        GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_box_pack_start(GTK_BOX(hb), gtk_image_new_from_icon_name(icon, GTK_ICON_SIZE_BUTTON), FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hb), gtk_label_new(label), FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(btn), hb);
        gtk_button_set_relief(GTK_BUTTON(btn), GTK_RELIEF_NONE);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_shortcut_clicked), (gpointer)path);
        gtk_box_pack_start(GTK_BOX(sidebar), btn, FALSE, FALSE, 0);
    };
    add_shortcut("Root", "drive-harddisk", "/");
    add_shortcut("Home", "user-home", ".");
    add_shortcut("Data", "folder-documents", "data");
    add_shortcut("Desktop", "user-desktop", "data/desktop");
    add_shortcut("Build", "applications-development", "build");
    add_shortcut("System", "applications-system", "gui");
    
    gtk_paned_pack1(GTK_PANED(paned), sidebar, FALSE, FALSE);
    
    /* Main file list */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_paned_pack2(GTK_PANED(paned), scroll, TRUE, FALSE);
    
    list_store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    file_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    g_object_unref(list_store);
    
    GtkCellRenderer *icon_rend = gtk_cell_renderer_pixbuf_new();
    GtkCellRenderer *text_rend = gtk_cell_renderer_text_new();
    
    GtkTreeViewColumn *col_name = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col_name, "Name");
    gtk_tree_view_column_pack_start(col_name, icon_rend, FALSE);
    gtk_tree_view_column_add_attribute(col_name, icon_rend, "icon-name", COL_ICON);
    gtk_tree_view_column_pack_start(col_name, text_rend, TRUE);
    gtk_tree_view_column_add_attribute(col_name, text_rend, "text", COL_NAME);
    gtk_tree_view_column_set_expand(col_name, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(file_list), col_name);
    
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(file_list), -1, "Size", text_rend, "text", COL_SIZE, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(file_list), -1, "Type", text_rend, "text", COL_TYPE, NULL);
    
    g_signal_connect(file_list, "row-activated", G_CALLBACK(on_row_activated), NULL);
    
    gtk_container_add(GTK_CONTAINER(scroll), file_list);
    
    load_directory("."); // Start in current dir
    
    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.fileexplorer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
