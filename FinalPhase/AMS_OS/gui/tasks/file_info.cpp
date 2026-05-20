/* AMS OS — GUI File Info */
#include "../gui_theme.h"

static GtkWidget *path_entry, *info_box;

static void clear_info() {
    GList *children = gtk_container_get_children(GTK_CONTAINER(info_box));
    for (GList *c = children; c; c = c->next) gtk_widget_destroy(GTK_WIDGET(c->data));
    g_list_free(children);
}

static void on_browse(GtkWidget *, gpointer win) {
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Select File", GTK_WINDOW(win),
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Select", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *p = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        gtk_entry_set_text(GTK_ENTRY(path_entry), p); g_free(p);
    }
    gtk_widget_destroy(dlg);
}

static void on_get_info(GtkWidget *, gpointer) {
    const char *path = gtk_entry_get_text(GTK_ENTRY(path_entry));
    if (strlen(path) == 0) return;
    clear_info();

    struct stat st;
    if (stat(path, &st) != 0) {
        GtkWidget *err = gtk_label_new("❌ File not found"); ams_css(err, "error-text");
        gtk_box_pack_start(GTK_BOX(info_box), err, FALSE, FALSE, 0);
        gtk_widget_show_all(info_box); return;
    }

    /* Extract filename */
    std::string fpath(path);
    size_t pos = fpath.rfind('/');
    std::string fname = (pos != std::string::npos) ? fpath.substr(pos+1) : fpath;

    /* Type */
    const char *type = S_ISDIR(st.st_mode) ? "Directory" : S_ISLNK(st.st_mode) ? "Symlink" : "Regular File";

    /* Size */
    char size[64];
    if (st.st_size > 1048576) snprintf(size, sizeof(size), "%.2f MB", st.st_size / 1048576.0);
    else if (st.st_size > 1024) snprintf(size, sizeof(size), "%.2f KB", st.st_size / 1024.0);
    else snprintf(size, sizeof(size), "%ld bytes", st.st_size);

    /* Permissions */
    char perms[16];
    snprintf(perms, sizeof(perms), "%c%c%c%c%c%c%c%c%c",
        st.st_mode & S_IRUSR ? 'r' : '-', st.st_mode & S_IWUSR ? 'w' : '-', st.st_mode & S_IXUSR ? 'x' : '-',
        st.st_mode & S_IRGRP ? 'r' : '-', st.st_mode & S_IWGRP ? 'w' : '-', st.st_mode & S_IXGRP ? 'x' : '-',
        st.st_mode & S_IROTH ? 'r' : '-', st.st_mode & S_IWOTH ? 'w' : '-', st.st_mode & S_IXOTH ? 'x' : '-');

    char mod_time[64]; strftime(mod_time, sizeof(mod_time), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));

    GtkWidget *card = ams_card("📋 File Information");
    gtk_box_pack_start(GTK_BOX(card), ams_info_row("Name", fname.c_str()), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), ams_info_row("Path", path), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), ams_info_row("Type", type), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), ams_info_row("Size", size), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), ams_info_row("Permissions", perms), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), ams_info_row("Modified", mod_time), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(info_box), card, FALSE, FALSE, 0);
    gtk_widget_show_all(info_box);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "ℹ️ File Info", "dialog-information", 480, 420);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 16); gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 12);   gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *lbl = gtk_label_new("File Path"); ams_css(lbl, "accent"); gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    path_entry = gtk_entry_new(); gtk_box_pack_start(GTK_BOX(row), path_entry, TRUE, TRUE, 0);
    GtkWidget *br = gtk_button_new_with_label("📂 Browse");
    g_signal_connect(br, "clicked", G_CALLBACK(on_browse), win);
    gtk_box_pack_start(GTK_BOX(row), br, FALSE, FALSE, 0);
    GtkWidget *gi = gtk_button_new_with_label("ℹ️ Get Info"); ams_css(gi, "suggested-action");
    g_signal_connect(gi, "clicked", G_CALLBACK(on_get_info), NULL);
    gtk_box_pack_start(GTK_BOX(row), gi, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), row, FALSE, FALSE, 0);

    info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_box_pack_start(GTK_BOX(vbox), info_box, TRUE, TRUE, 0);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.fileinfo", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv); g_object_unref(app); return s;
}
