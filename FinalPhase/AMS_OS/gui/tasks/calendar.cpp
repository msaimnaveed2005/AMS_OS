/* AMS OS — GUI Calendar */
#include "../gui_theme.h"

static GtkWidget *detail_lbl = NULL;

static void on_day_selected(GtkCalendar *cal, gpointer) {
    guint y, m, d;
    gtk_calendar_get_date(cal, &y, &m, &d);
    char buf[128];
    snprintf(buf, sizeof(buf), "Selected: %02d/%02d/%04d", d, m + 1, y);
    gtk_label_set_text(GTK_LABEL(detail_lbl), buf);
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "Calendar", "x-office-calendar", 380, 380);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(box, 16); gtk_widget_set_margin_end(box, 16);
    gtk_widget_set_margin_top(box, 12);   gtk_widget_set_margin_bottom(box, 16);
    gtk_container_add(GTK_CONTAINER(win), box);

    /* Today label */
    time_t now = time(NULL); struct tm *t = localtime(&now);
    char today[128]; strftime(today, sizeof(today), "📅  Today is %A, %B %d, %Y", t);
    GtkWidget *today_lbl = gtk_label_new(today);
    ams_css(today_lbl, "accent");
    gtk_box_pack_start(GTK_BOX(box), today_lbl, FALSE, FALSE, 0);

    GtkWidget *cal = gtk_calendar_new();
    gtk_calendar_set_display_options(GTK_CALENDAR(cal),
        (GtkCalendarDisplayOptions)(GTK_CALENDAR_SHOW_HEADING | GTK_CALENDAR_SHOW_DAY_NAMES));
    g_signal_connect(cal, "day-selected", G_CALLBACK(on_day_selected), NULL);
    gtk_box_pack_start(GTK_BOX(box), cal, TRUE, TRUE, 0);

    detail_lbl = gtk_label_new("Click a date to see details");
    ams_css(detail_lbl, "subtitle-lbl");
    gtk_box_pack_start(GTK_BOX(box), detail_lbl, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.calendar", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
