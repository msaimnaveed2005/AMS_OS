/* AMS OS — GUI Digital Clock */
#include "../gui_theme.h"

static GtkWidget *time_lbl = NULL, *date_lbl = NULL;

static gboolean tick(gpointer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char tbuf[32], dbuf[64];
    strftime(tbuf, sizeof(tbuf), "%I:%M:%S %p", t);
    strftime(dbuf, sizeof(dbuf), "%A, %B %d, %Y", t);
    if (time_lbl) gtk_label_set_text(GTK_LABEL(time_lbl), tbuf);
    if (date_lbl) gtk_label_set_text(GTK_LABEL(date_lbl), dbuf);
    return G_SOURCE_CONTINUE;
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "Clock", "preferences-system-time", 380, 220);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(win), box);

    time_lbl = gtk_label_new(""); ams_css(time_lbl, "title-xl");
    gtk_box_pack_start(GTK_BOX(box), time_lbl, FALSE, FALSE, 0);

    date_lbl = gtk_label_new(""); ams_css(date_lbl, "subtitle-lbl");
    gtk_box_pack_start(GTK_BOX(box), date_lbl, FALSE, FALSE, 0);

    tick(NULL);
    g_timeout_add_seconds(1, tick, NULL);
    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.clock", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
