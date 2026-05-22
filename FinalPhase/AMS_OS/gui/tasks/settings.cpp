/* AMS OS — GUI Settings & Control Panel */
#include "../gui_theme.h"
#include <gtk/gtk.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

static GtkWidget *css_view;

static void load_theme() {
    std::ifstream in("data/theme.css");
    std::string css_str;
    if (in.is_open()) {
        css_str = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    } else {
        css_str = "/* No custom theme found. */\n";
    }
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(css_view));
    gtk_text_buffer_set_text(buffer, css_str.c_str(), -1);
}

static void on_save_apply(GtkWidget*, gpointer) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(css_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    
    std::ofstream out("data/theme.css");
    out << text;
    out.close();
    g_free(text);
    
    /* Trigger OS Desktop to reload the theme instantly! */
    system("pkill -x -USR1 ams_desktop 2>/dev/null");
    
    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Theme applied successfully!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void on_preset_clicked(GtkWidget *btn, gpointer css_code) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(css_view));
    gtk_text_buffer_set_text(buffer, (const char*)css_code, -1);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "Settings & Control Panel", "preferences-system", 800, 600);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 12); gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 12); gtk_widget_set_margin_bottom(vbox, 12);
    gtk_container_add(GTK_CONTAINER(win), vbox);
    
    /* Title */
    GtkWidget *title = gtk_label_new("Theme Engine Control");
    ams_css(title, "title-xl");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    
    GtkWidget *desc = gtk_label_new("Edit the raw GTK CSS below to instantly morph the OS interface. Click 'Apply' to hot-reload the entire OS desktop!");
    gtk_box_pack_start(GTK_BOX(vbox), desc, FALSE, FALSE, 0);
    
    /* Presets */
    GtkWidget *preset_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_pack_start(GTK_BOX(vbox), preset_box, FALSE, FALSE, 0);
    
    auto add_preset = [&](const char* name, const char* code) {
        GtkWidget *btn = gtk_button_new_with_label(name);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_preset_clicked), (gpointer)code);
        gtk_box_pack_start(GTK_BOX(preset_box), btn, FALSE, FALSE, 0);
    };
    
    add_preset("Hacker Matrix", "window.desktop { background-color: #000000; background-image: none; }\n.top-bar { background-color: rgba(0,255,0,0.1); border-bottom: 1px solid #00ff00; }\n* { color: #00ff00; }");
    add_preset("Cyberpunk Purple", "window.desktop { background-image: linear-gradient(135deg, #2a0845 0%, #6441A5 100%); }\n.top-bar { background-color: rgba(0,0,0,0.4); }");
    add_preset("Deep Ocean", "window.desktop { background-image: linear-gradient(to bottom right, #0f2027, #203a43, #2c5364); }");
    
    /* Editor */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
    
    css_view = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(css_view), TRUE);
    gtk_widget_set_margin_start(css_view, 8); gtk_widget_set_margin_end(css_view, 8);
    gtk_widget_set_margin_top(css_view, 8); gtk_widget_set_margin_bottom(css_view, 8);
    
    PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 10");
    gtk_widget_override_font(css_view, font_desc);
    pango_font_description_free(font_desc);
    
    gtk_container_add(GTK_CONTAINER(scroll), css_view);
    
    /* Bottom Actions */
    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), action_box, FALSE, FALSE, 0);
    
    GtkWidget *reload_btn = gtk_button_new_with_label("Reload from File");
    g_signal_connect(reload_btn, "clicked", G_CALLBACK(+[](GtkWidget*, gpointer){ load_theme(); }), NULL);
    gtk_box_pack_start(GTK_BOX(action_box), reload_btn, FALSE, FALSE, 0);
    
    GtkWidget *apply_btn = gtk_button_new_with_label("Save & Apply Theme");
    ams_css(apply_btn, "suggested-action");
    g_signal_connect(apply_btn, "clicked", G_CALLBACK(on_save_apply), NULL);
    gtk_box_pack_end(GTK_BOX(action_box), apply_btn, FALSE, FALSE, 0);
    
    load_theme();
    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.settings", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
