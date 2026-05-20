/* AMS OS — GUI Notepad (dark text editor with open/save) */
#include "../gui_theme.h"

struct NoteState {
    GtkWidget     *text_view;
    GtkWidget     *header;
    std::string    file_path;
    bool           modified;
};
static NoteState N = {};

static void update_title() {
    std::string title = "Notepad";
    if (!N.file_path.empty()) {
        size_t pos = N.file_path.rfind('/');
        title = (pos != std::string::npos) ? N.file_path.substr(pos + 1) : N.file_path;
    }
    if (N.modified) title = "● " + title;
    gtk_header_bar_set_title(GTK_HEADER_BAR(N.header), title.c_str());
}

static void on_text_changed(GtkTextBuffer *, gpointer) {
    N.modified = true;
    update_title();
}

static void on_new(GtkWidget *, gpointer) {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(N.text_view));
    gtk_text_buffer_set_text(buf, "", -1);
    N.file_path.clear();
    N.modified = false;
    update_title();
}

static void on_open(GtkWidget *, gpointer win) {
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Open File",
        GTK_WINDOW(win), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        std::ifstream f(path);
        if (f.is_open()) {
            std::string content((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
            GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(N.text_view));
            gtk_text_buffer_set_text(buf, content.c_str(), -1);
            N.file_path = path;
            N.modified = false;
            update_title();
        }
        g_free(path);
    }
    gtk_widget_destroy(dlg);
}

static void save_to_path(const std::string &path) {
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(N.text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buf, &start, &end);
    char *text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
    std::ofstream f(path);
    if (f.is_open()) { f << text; N.file_path = path; N.modified = false; update_title(); }
    g_free(text);
}

static void on_save(GtkWidget *, gpointer win) {
    if (!N.file_path.empty()) {
        save_to_path(N.file_path);
        return;
    }
    GtkWidget *dlg = gtk_file_chooser_dialog_new("Save File",
        GTK_WINDOW(win), GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dlg), TRUE);
    if (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT) {
        char *path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg));
        save_to_path(path);
        g_free(path);
    }
    gtk_widget_destroy(dlg);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Notepad");
    gtk_window_set_default_size(GTK_WINDOW(win), 700, 500);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(win), "accessories-text-editor");

    /* Header bar with buttons */
    N.header = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(N.header), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(N.header), "Notepad");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(N.header), "AMS OS");

    GtkWidget *btn_new = gtk_button_new_with_label("📄 New");
    g_signal_connect(btn_new, "clicked", G_CALLBACK(on_new), NULL);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(N.header), btn_new);

    GtkWidget *btn_open = gtk_button_new_with_label("📂 Open");
    g_signal_connect(btn_open, "clicked", G_CALLBACK(on_open), win);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(N.header), btn_open);

    GtkWidget *btn_save = gtk_button_new_with_label("💾 Save");
    ams_css(btn_save, "suggested-action");
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save), win);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(N.header), btn_save);

    gtk_window_set_titlebar(GTK_WINDOW(win), N.header);

    /* Text view */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    N.text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(N.text_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(N.text_view), 12);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(N.text_view), 12);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(N.text_view), 12);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(N.text_view), 12);
    gtk_container_add(GTK_CONTAINER(scroll), N.text_view);
    gtk_container_add(GTK_CONTAINER(win), scroll);

    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(N.text_view));
    g_signal_connect(buf, "changed", G_CALLBACK(on_text_changed), NULL);
    N.modified = false;

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.notepad", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
