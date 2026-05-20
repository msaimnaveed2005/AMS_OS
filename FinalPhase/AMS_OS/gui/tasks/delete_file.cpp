/* AMS OS — GUI Delete File */
#include "../gui_theme.h"

static GtkWidget *path_entry, *status_lbl, *info_lbl;

static void on_browse(GtkWidget *, gpointer win) {
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Select File", GTK_WINDOW(win),
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Select", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *p = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        gtk_entry_set_text(GTK_ENTRY(path_entry), p);
        struct stat st;
        if (stat(p, &st) == 0) {
            char buf[128]; snprintf(buf, sizeof(buf), "Size: %ld bytes | Last modified: %s", st.st_size, ctime(&st.st_mtime));
            gtk_label_set_text(GTK_LABEL(info_lbl), buf);
        }
        g_free(p);
    }
    gtk_widget_destroy(dlg);
}

static void on_delete(GtkWidget *, gpointer win) {
    const char *path = gtk_entry_get_text(GTK_ENTRY(path_entry));
    if (strlen(path) == 0) { gtk_label_set_text(GTK_LABEL(status_lbl), "❌ No file selected"); return; }

    GtkWidget *confirm = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL,
        GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "Delete \"%s\"?\nThis cannot be undone.", path);
    int resp = gtk_dialog_run(GTK_DIALOG(confirm));
    gtk_widget_destroy(confirm);
    if (resp != GTK_RESPONSE_YES) return;

    if (std::remove(path) == 0) {
        gtk_label_set_text(GTK_LABEL(status_lbl), "✅ File deleted");
        ams_css(status_lbl, "success");
    } else {
        gtk_label_set_text(GTK_LABEL(status_lbl), "❌ Failed to delete");
        ams_css(status_lbl, "error-text");
    }
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "🗑 Delete File", "edit-delete", 440, 300);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 16); gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 12);   gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *lbl = gtk_label_new("File Path"); ams_css(lbl, "accent");
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    path_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(row), path_entry, TRUE, TRUE, 0);
    GtkWidget *browse = gtk_button_new_with_label("📂 Browse");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse), win);
    gtk_box_pack_start(GTK_BOX(row), browse, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), row, FALSE, FALSE, 0);

    info_lbl = gtk_label_new(""); ams_css(info_lbl, "dim");
    gtk_widget_set_halign(info_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), info_lbl, FALSE, FALSE, 0);

    GtkWidget *btn = gtk_button_new_with_label("🗑 Delete File");
    ams_css(btn, "destructive-action");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_delete), win);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);

    status_lbl = gtk_label_new(""); gtk_box_pack_start(GTK_BOX(vbox), status_lbl, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.deletefile", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv); g_object_unref(app); return s;
}
