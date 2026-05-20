/* AMS OS — GUI Process Killer */
#include "../gui_theme.h"

static GtkWidget *pid_entry, *status_lbl;
static int selected_signal = SIGTERM;

static void on_signal_changed(GtkComboBoxText *combo, gpointer) {
    const char *text = gtk_combo_box_text_get_active_text(combo);
    if (!text) return;
    if (strstr(text, "SIGTERM")) selected_signal = SIGTERM;
    else if (strstr(text, "SIGKILL")) selected_signal = SIGKILL;
    else if (strstr(text, "SIGSTOP")) selected_signal = SIGSTOP;
    else if (strstr(text, "SIGCONT")) selected_signal = SIGCONT;
}

static void on_send(GtkWidget *, gpointer) {
    const char *pid_str = gtk_entry_get_text(GTK_ENTRY(pid_entry));
    if (strlen(pid_str) == 0) { gtk_label_set_text(GTK_LABEL(status_lbl), "❌ Enter a PID"); return; }
    int pid = atoi(pid_str);
    if (pid <= 0) { gtk_label_set_text(GTK_LABEL(status_lbl), "❌ Invalid PID"); return; }

    if (kill(pid, selected_signal) == 0) {
        char buf[128]; snprintf(buf, sizeof(buf), "✅ Signal %d sent to PID %d", selected_signal, pid);
        gtk_label_set_text(GTK_LABEL(status_lbl), buf);
        ams_css(status_lbl, "success");
    } else {
        char buf[128]; snprintf(buf, sizeof(buf), "❌ Failed: %s", strerror(errno));
        gtk_label_set_text(GTK_LABEL(status_lbl), buf);
        ams_css(status_lbl, "error-text");
    }
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "⚡ Process Killer", "process-stop", 380, 320);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(vbox, 20); gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 16);   gtk_widget_set_margin_bottom(vbox, 20);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *lbl1 = gtk_label_new("Process ID (PID)"); ams_css(lbl1, "accent");
    gtk_widget_set_halign(lbl1, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), lbl1, FALSE, FALSE, 0);

    pid_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(pid_entry), "Enter PID...");
    gtk_box_pack_start(GTK_BOX(vbox), pid_entry, FALSE, FALSE, 0);

    GtkWidget *lbl2 = gtk_label_new("Signal"); ams_css(lbl2, "accent");
    gtk_widget_set_halign(lbl2, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), lbl2, FALSE, FALSE, 0);

    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "SIGTERM (15) — Graceful");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "SIGKILL (9) — Force");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "SIGSTOP (19) — Pause");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "SIGCONT (18) — Resume");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    g_signal_connect(combo, "changed", G_CALLBACK(on_signal_changed), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 0);

    GtkWidget *btn = gtk_button_new_with_label("⚡ Send Signal");
    ams_css(btn, "destructive-action");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_send), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), btn, FALSE, FALSE, 4);

    status_lbl = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), status_lbl, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.processkiller", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv); g_object_unref(app); return s;
}
