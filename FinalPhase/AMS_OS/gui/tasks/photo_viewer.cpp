/* AMS OS — Photo Viewer */
#include "../gui_theme.h"
#include <gtk/gtk.h>
#include <string>

static GtkWidget *image_widget = NULL;
static GtkWidget *window = NULL;
static GtkWidget *header_subtitle = NULL;

static void load_image(const char *path) {
    if (!path || !image_widget) return;
    
    // Scale image while preserving aspect ratio
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    if (pixbuf) {
        int w = gdk_pixbuf_get_width(pixbuf);
        int h = gdk_pixbuf_get_height(pixbuf);
        
        // Target max 800x600 for viewing in this simple app
        double scale = 1.0;
        if (w > 800 || h > 600) {
            double scale_w = 800.0 / w;
            double scale_h = 600.0 / h;
            scale = (scale_w < scale_h) ? scale_w : scale_h;
        }
        
        GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, w * scale, h * scale, GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf(GTK_IMAGE(image_widget), scaled);
        g_object_unref(scaled);
        g_object_unref(pixbuf);
        
        std::string spath(path);
        size_t last_slash = spath.find_last_of("/\\");
        std::string name = (last_slash == std::string::npos) ? spath : spath.substr(last_slash + 1);
        gtk_label_set_text(GTK_LABEL(header_subtitle), name.c_str());
    } else {
        gtk_label_set_text(GTK_LABEL(header_subtitle), "Error loading image");
    }
}

static void on_open_clicked(GtkWidget*, gpointer) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Open Image",
        GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
    
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        load_image(filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    window = ams_window(app, "AMS Photo Viewer", "image-x-generic", 840, 680);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    /* Header Toolbar */
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_margin_start(toolbar, 10); gtk_widget_set_margin_end(toolbar, 10);
    gtk_widget_set_margin_top(toolbar, 10); gtk_widget_set_margin_bottom(toolbar, 10);
    
    GtkWidget *open_btn = gtk_button_new_with_label("📂 Open Image");
    g_signal_connect(open_btn, "clicked", G_CALLBACK(on_open_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), open_btn, FALSE, FALSE, 0);
    
    header_subtitle = gtk_label_new("No Image Loaded");
    gtk_widget_set_margin_start(header_subtitle, 20);
    ams_css(header_subtitle, "mode-desc");
    gtk_box_pack_start(GTK_BOX(toolbar), header_subtitle, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    
    /* Image Area */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
    
    image_widget = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(scroll), image_widget);
    
    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.photoviewer", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
