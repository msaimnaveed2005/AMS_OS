/* AMS OS — GUI Create File */
#include "../gui_theme.h"

static GtkWidget *path_entry, *text_view, *status_lbl;

static void on_create(GtkWidget *, gpointer) {
    const char *path = gtk_entry_get_text(GTK_ENTRY(path_entry));
    if (strlen(path) == 0) { gtk_label_set_text(GTK_LABEL(status_lbl), "❌ Please enter a file path"); return; }

    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter s, e; gtk_text_buffer_get_bounds(buf, &s, &e);
    char *content = gtk_text_buffer_get_text(buf, &s, &e, FALSE);

    std::ofstream f(path);
    if (f.is_open()) {
        f << content;
        gtk_label_set_text(GTK_LABEL(status_lbl), "✅ File created successfully");
        ams_css(status_lbl, "success");
    } else {
        gtk_label_set_text(GTK_LABEL(status_lbl), "❌ Failed to create file");
        ams_css(status_lbl, "error-text");
    }
    g_free(content);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "📄 Create File", "document-new", 450, 380);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 16); gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 12);   gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *lbl = gtk_label_new("File Path"); ams_css(lbl, "accent");
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    path_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(path_entry), "/home/user/newfile.txt");
    gtk_box_pack_start(GTK_BOX(vbox), path_entry, FALSE, FALSE, 0);

    GtkWidget *clbl = gtk_label_new("Content (optional)"); ams_css(clbl, "accent");
    gtk_widget_set_halign(clbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), clbl, FALSE, FALSE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *btn = gtk_button_new_with_label("📄 Create File");
    ams_css(btn, "suggested-action");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_create), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 0);

    status_lbl = gtk_label_new(""); ams_css(status_lbl, "dim");
    gtk_box_pack_start(GTK_BOX(vbox), status_lbl, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.createfile", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv); g_object_unref(app); return s;
}
