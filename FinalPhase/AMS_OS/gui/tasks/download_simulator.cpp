/* AMS OS — GUI Download Simulator */
#include "../gui_theme.h"

struct Download {
    GtkWidget *bar;
    GtkWidget *speed_lbl;
    GtkWidget *status_lbl;
    double     progress;
    double     speed;
    bool       done;
};
static std::vector<Download*> downloads;

static gboolean dl_tick(gpointer) {
    for (auto *d : downloads) {
        if (d->done) continue;
        d->speed = 1.5 + (rand() % 30) / 10.0;
        d->progress += d->speed;
        if (d->progress >= 100.0) {
            d->progress = 100.0; d->done = true;
            gtk_label_set_text(GTK_LABEL(d->status_lbl), "✅ Complete");
            ams_css(d->status_lbl, "success");
            gtk_label_set_text(GTK_LABEL(d->speed_lbl), "");
        } else {
            char buf[32]; snprintf(buf, sizeof(buf), "%.1f MB/s", d->speed);
            gtk_label_set_text(GTK_LABEL(d->speed_lbl), buf);
            snprintf(buf, sizeof(buf), "%.0f%%", d->progress);
            gtk_label_set_text(GTK_LABEL(d->status_lbl), buf);
        }
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(d->bar), d->progress / 100.0);
    }
    return G_SOURCE_CONTINUE;
}

static int dl_counter = 0;

static void on_download(GtkWidget *, gpointer data) {
    GtkWidget *list_box = (GtkWidget *)data;
    dl_counter++;

    Download *d = new Download{};
    d->progress = 0; d->speed = 0; d->done = false;

    GtkWidget *card = ams_card(NULL);
    char name[64]; snprintf(name, sizeof(name), "📦 package_%03d.tar.gz", dl_counter);
    GtkWidget *nlbl = gtk_label_new(name);
    gtk_widget_set_halign(nlbl, GTK_ALIGN_START);
    ams_css(nlbl, "accent");
    gtk_box_pack_start(GTK_BOX(card), nlbl, FALSE, FALSE, 0);

    d->bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(card), d->bar, FALSE, FALSE, 4);

    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    d->status_lbl = gtk_label_new("0%"); ams_css(d->status_lbl, "dim");
    gtk_box_pack_start(GTK_BOX(row), d->status_lbl, FALSE, FALSE, 0);
    d->speed_lbl = gtk_label_new(""); ams_css(d->speed_lbl, "dim");
    gtk_box_pack_end(GTK_BOX(row), d->speed_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), row, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(list_box), card, FALSE, FALSE, 4);
    gtk_widget_show_all(card);
    downloads.push_back(d);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    srand(time(NULL));
    GtkWidget *win = ams_window(app, "⬇️ Downloads", "emblem-downloads", 520, 420);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 16); gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 12);   gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* URL entry + button */
    GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "https://ams-os.dev/packages/latest.tar.gz");
    gtk_box_pack_start(GTK_BOX(top), entry, TRUE, TRUE, 0);

    GtkWidget *dl_btn = gtk_button_new_with_label("⬇ Download");
    ams_css(dl_btn, "suggested-action");
    gtk_box_pack_start(GTK_BOX(top), dl_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), top, FALSE, FALSE, 0);

    /* Download list */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    GtkWidget *list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(scroll), list);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    g_signal_connect(dl_btn, "clicked", G_CALLBACK(on_download), list);
    g_timeout_add(200, dl_tick, NULL);

    /* Start with one download */
    on_download(NULL, list);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.downloads", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
