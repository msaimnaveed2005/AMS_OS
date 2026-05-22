/* AMS OS — GUI Terminal Emulator */
#include "../gui_theme.h"
#include <gtk/gtk.h>
#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

static GtkWidget *output_view;
static GtkWidget *cmd_entry;
static GtkWidget *scroll;

/* Function to run a shell command and capture output */
static std::string exec_command(const char* cmd) {
    std::array<char, 256> buffer;
    std::string result;
    /* Redirect stderr to stdout to capture error messages too */
    std::string full_cmd = std::string(cmd) + " 2>&1";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(full_cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

static void append_to_output(const std::string& text, bool is_command) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    
    std::string formatted = text;
    if (is_command) {
        formatted = "\nadmin@ams-os:~$ " + text + "\n";
    }
    
    gtk_text_buffer_insert(buffer, &iter, formatted.c_str(), -1);
    
    /* Auto-scroll to bottom */
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroll));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
}

static void on_cmd_activate(GtkEntry *entry, gpointer) {
    const char *cmd = gtk_entry_get_text(entry);
    if (!cmd || strlen(cmd) == 0) return;
    
    std::string command(cmd);
    gtk_entry_set_text(entry, "");
    
    if (command == "clear") {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
        gtk_text_buffer_set_text(buffer, "", -1);
        append_to_output("AMS OS Terminal v3.0\nType 'help' for commands.\n", false);
        return;
    }
    
    append_to_output(command, true);
    
    std::string output = exec_command(cmd);
    if (output.empty()) {
        output = "[No Output]\n";
    }
    append_to_output(output, false);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "AMS Terminal", "utilities-terminal", 600, 450);
    ams_css(win, "terminal");
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    
    /* Output Area */
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
    
    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(output_view), TRUE);
    gtk_widget_set_margin_start(output_view, 8);
    gtk_widget_set_margin_end(output_view, 8);
    gtk_widget_set_margin_top(output_view, 8);
    gtk_widget_set_margin_bottom(output_view, 8);
    
    /* Change font style directly inline */
    PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 11");
    gtk_widget_override_font(output_view, font_desc);
    pango_font_description_free(font_desc);
    
    /* Apply a green-on-black retro theme */
    GdkRGBA bg_color; gdk_rgba_parse(&bg_color, "#000000");
    GdkRGBA fg_color; gdk_rgba_parse(&fg_color, "#34d399");
    gtk_widget_override_background_color(output_view, GTK_STATE_FLAG_NORMAL, &bg_color);
    gtk_widget_override_color(output_view, GTK_STATE_FLAG_NORMAL, &fg_color);
    
    gtk_container_add(GTK_CONTAINER(scroll), output_view);
    
    /* Input Area */
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(input_box, 8);
    gtk_widget_set_margin_end(input_box, 8);
    gtk_widget_set_margin_top(input_box, 8);
    gtk_widget_set_margin_bottom(input_box, 8);
    
    GtkWidget *prompt_label = gtk_label_new("admin@ams-os:~$ ");
    gtk_widget_override_font(prompt_label, pango_font_description_from_string("Monospace 11"));
    gtk_widget_override_color(prompt_label, GTK_STATE_FLAG_NORMAL, &fg_color);
    
    cmd_entry = gtk_entry_new();
    gtk_widget_override_font(cmd_entry, pango_font_description_from_string("Monospace 11"));
    gtk_widget_override_color(cmd_entry, GTK_STATE_FLAG_NORMAL, &fg_color);
    gtk_widget_override_background_color(cmd_entry, GTK_STATE_FLAG_NORMAL, &bg_color);
    g_signal_connect(cmd_entry, "activate", G_CALLBACK(on_cmd_activate), NULL);
    
    gtk_box_pack_start(GTK_BOX(input_box), prompt_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(input_box), cmd_entry, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), input_box, FALSE, FALSE, 0);
    
    append_to_output("AMS OS Terminal v3.0\nType 'help' for commands.\n", false);
    
    gtk_widget_show_all(win);
    gtk_widget_grab_focus(cmd_entry);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.terminal", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
