/* AMS OS — AMS Studio (Native IDE) */
#include "../gui_theme.h"
#include <gtk/gtk.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <string>

static GtkWidget *code_view;
static GtkWidget *console_view;
static GtkWidget *header_subtitle;
static GtkWidget *window;
static std::string current_file = "";
static bool is_compiling = false;

static void set_current_file(const std::string &path) {
    current_file = path;
    if (path.empty()) {
        gtk_label_set_text(GTK_LABEL(header_subtitle), "Unsaved File");
    } else {
        size_t last_slash = path.find_last_of("/\\");
        std::string name = (last_slash == std::string::npos) ? path : path.substr(last_slash + 1);
        gtk_label_set_text(GTK_LABEL(header_subtitle), name.c_str());
    }
}

static void append_console(const std::string &text) {
    std::string *s = new std::string(text);
    g_idle_add([](gpointer data) -> gboolean {
        std::string *str = (std::string*)data;
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(console_view));
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buf, &end);
        gtk_text_buffer_insert(buf, &end, str->c_str(), -1);
        
        GtkWidget *sw = gtk_widget_get_parent(console_view);
        GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sw));
        gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj));
        
        delete str;
        return G_SOURCE_REMOVE;
    }, s);
}

static void on_open_clicked(GtkWidget*, gpointer) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open C++ Source",
        GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::ifstream in(filename);
        if (in.is_open()) {
            std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(code_view));
            gtk_text_buffer_set_text(buf, content.c_str(), -1);
            set_current_file(filename);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_save_clicked(GtkWidget*, gpointer) {
    std::string path_to_save = current_file;
    if (path_to_save.empty()) {
        GtkWidget *dialog = gtk_file_chooser_dialog_new("Save C++ Source",
            GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_SAVE,
            "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "new_app.cpp");
        
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            path_to_save = filename;
            set_current_file(filename);
            g_free(filename);
        }
        gtk_widget_destroy(dialog);
    }
    
    if (!path_to_save.empty()) {
        GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(code_view));
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(buf, &start);
        gtk_text_buffer_get_end_iter(buf, &end);
        char *text = gtk_text_buffer_get_text(buf, &start, &end, FALSE);
        
        std::ofstream out(path_to_save);
        out << text;
        out.close();
        g_free(text);
        append_console("💾 Saved file: " + path_to_save + "\n");
    }
}

static void on_compile_clicked(GtkWidget*, gpointer) {
    if (is_compiling) return;
    is_compiling = true;
    
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(console_view));
    gtk_text_buffer_set_text(buf, "--- Starting AMS OS Build System ---\n", -1);
    
    std::thread([]() {
        /* Support native make or fallback to wsl make */
        FILE *fp = popen("make 2>&1", "r");
        if (!fp) fp = popen("wsl make 2>&1", "r"); 
        
        if (fp) {
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), fp)) {
                append_console(buffer);
            }
            int status = pclose(fp);
            if (status == 0) {
                append_console("\n✅ Build Completed Successfully!\n");
                /* Tell Desktop to refresh to show any new apps */
                system("pkill -x -USR1 ams_os 2>/dev/null");
            } else {
                append_console("\n❌ Build Failed with errors.\n");
            }
        } else {
            append_console("⚠️ Failed to execute compiler command.\n");
        }
        is_compiling = false;
    }).detach();
}

static void on_run_clicked(GtkWidget*, gpointer) {
    if (current_file.empty()) {
        append_console("⚠️ Please save and open an application first.\n");
        return;
    }
    
    /* Auto-detect executable name from cpp filename */
    size_t last_slash = current_file.find_last_of("/\\");
    std::string name = (last_slash == std::string::npos) ? current_file : current_file.substr(last_slash + 1);
    size_t dot = name.find_last_of('.');
    if (dot != std::string::npos) name = name.substr(0, dot);
    
    std::string exec_path = "./build/gui_" + name;
    append_console("🚀 Launching " + exec_path + "...\n");
    
    if (fork() == 0) {
        setsid();
        execlp(exec_path.c_str(), exec_path.c_str(), (char*)NULL);
        _exit(1);
    }
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    window = ams_window(app, "AMS Studio", "applications-development", 1000, 700);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    /* Header Toolbar */
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start(toolbar, 10); gtk_widget_set_margin_end(toolbar, 10);
    gtk_widget_set_margin_top(toolbar, 10); gtk_widget_set_margin_bottom(toolbar, 10);
    
    GtkWidget *open_btn = gtk_button_new_with_label("📂 Open");
    g_signal_connect(open_btn, "clicked", G_CALLBACK(on_open_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), open_btn, FALSE, FALSE, 0);
    
    GtkWidget *save_btn = gtk_button_new_with_label("💾 Save");
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), save_btn, FALSE, FALSE, 0);
    
    header_subtitle = gtk_label_new("Unsaved File");
    gtk_widget_set_margin_start(header_subtitle, 20);
    ams_css(header_subtitle, "mode-desc");
    gtk_box_pack_start(GTK_BOX(toolbar), header_subtitle, FALSE, FALSE, 0);
    
    GtkWidget *run_btn = gtk_button_new_with_label("▶ Run App");
    g_signal_connect(run_btn, "clicked", G_CALLBACK(on_run_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(toolbar), run_btn, FALSE, FALSE, 0);
    
    GtkWidget *compile_btn = gtk_button_new_with_label("⚙ Compile OS");
    ams_css(compile_btn, "suggested-action");
    g_signal_connect(compile_btn, "clicked", G_CALLBACK(on_compile_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(toolbar), compile_btn, FALSE, FALSE, 6);
    
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    
    /* Paned layout */
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);
    
    /* Code Editor */
    GtkWidget *code_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(code_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    code_view = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(code_view), TRUE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(code_view), 8);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(code_view), 8);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(code_view), 8);
    
    /* Code Editor Styling */
    GtkCssProvider *cp = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cp, "textview { font-size: 13px; color: #e5e7eb; background-color: #1f2937; }", -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(code_view), GTK_STYLE_PROVIDER(cp), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(cp);
    
    gtk_container_add(GTK_CONTAINER(code_scroll), code_view);
    gtk_paned_pack1(GTK_PANED(paned), code_scroll, TRUE, FALSE);
    
    /* Compiler Console */
    GtkWidget *console_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(console_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(console_scroll, -1, 200);
    
    console_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(console_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(console_view), TRUE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(console_view), 8);
    
    GtkCssProvider *cp2 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cp2, "textview { font-size: 12px; color: #34d399; background-color: #0a0a0a; }", -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(console_view), GTK_STYLE_PROVIDER(cp2), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(cp2);
    
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(console_view));
    gtk_text_buffer_set_text(buf, "AMS Studio IDE initialized.\nReady to compile.", -1);
    
    gtk_container_add(GTK_CONTAINER(console_scroll), console_view);
    gtk_paned_pack2(GTK_PANED(paned), console_scroll, FALSE, FALSE);
    
    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.studio", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
