/* AMS OS — GUI Move File */
#include "../gui_theme.h"

static GtkWidget *src_entry, *dst_entry, *status_lbl;

static void browse(GtkWidget *, gpointer data) {
    GtkWidget **entry = (GtkWidget **)data;
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Select", NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Select", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *p = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        gtk_entry_set_text(GTK_ENTRY(*entry), p); g_free(p);
    }
    gtk_widget_destroy(dlg);
}

static void on_move(GtkWidget *, gpointer) {
    const char *src = gtk_entry_get_text(GTK_ENTRY(src_entry));
    const char *dst = gtk_entry_get_text(GTK_ENTRY(dst_entry));
    if (!strlen(src) || !strlen(dst)) { gtk_label_set_text(GTK_LABEL(status_lbl), "❌ Both paths required"); return; }

    if (std::rename(src, dst) == 0) {
        gtk_label_set_text(GTK_LABEL(status_lbl), "✅ File moved successfully");
        ams_css(status_lbl, "success");
    } else {
        /* Fallback: copy + delete */
        std::ifstream in(src, std::ios::binary);
        std::ofstream out(dst, std::ios::binary);
        if (in.is_open() && out.is_open()) {
            out << in.rdbuf();
            in.close(); out.close();
            std::remove(src);
            gtk_label_set_text(GTK_LABEL(status_lbl), "✅ File moved (copy+delete)");
            ams_css(status_lbl, "success");
        } else {
            gtk_label_set_text(GTK_LABEL(status_lbl), "❌ Move failed");
            ams_css(status_lbl, "error-text");
        }
    }
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "📦 Move File", "document-send", 480, 300);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 16); gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 12);   gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *sl = gtk_label_new("Source"); ams_css(sl, "accent"); gtk_widget_set_halign(sl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), sl, FALSE, FALSE, 0);
    GtkWidget *sr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    src_entry = gtk_entry_new(); gtk_box_pack_start(GTK_BOX(sr), src_entry, TRUE, TRUE, 0);
    GtkWidget *sb = gtk_button_new_with_label("📂"); g_signal_connect(sb, "clicked", G_CALLBACK(browse), &src_entry);
    gtk_box_pack_start(GTK_BOX(sr), sb, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), sr, FALSE, FALSE, 0);

    GtkWidget *dl = gtk_label_new("Destination"); ams_css(dl, "accent"); gtk_widget_set_halign(dl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), dl, FALSE, FALSE, 0);
    GtkWidget *dr = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    dst_entry = gtk_entry_new(); gtk_box_pack_start(GTK_BOX(dr), dst_entry, TRUE, TRUE, 0);
    GtkWidget *db = gtk_button_new_with_label("📂"); g_signal_connect(db, "clicked", G_CALLBACK(browse), &dst_entry);
    gtk_box_pack_start(GTK_BOX(dr), db, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), dr, FALSE, FALSE, 0);

    GtkWidget *btn = gtk_button_new_with_label("📦 Move"); ams_css(btn, "suggested-action");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_move), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 4);

    status_lbl = gtk_label_new(""); gtk_box_pack_start(GTK_BOX(vbox), status_lbl, FALSE, FALSE, 0);
    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.movefile", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv); g_object_unref(app); return s;
}
