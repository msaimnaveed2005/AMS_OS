/* AMS OS — GUI Music Player (simulated playlist) */
#include "../gui_theme.h"

struct Song { const char *title; const char *artist; int duration; };
static const Song PLAYLIST[] = {
    {"Midnight Drive",     "Neon Pulse",      234},
    {"Electric Dreams",    "Synthwave Radio",  198},
    {"Starlight Sonata",   "Luna Echo",       312},
    {"Digital Rain",       "Cyber Horizon",    267},
    {"Atomic Heartbeat",   "AMS Band",        189},
    {"Velvet Cascade",     "Dream Weaver",    256},
};
static const int SONG_COUNT = sizeof(PLAYLIST) / sizeof(PLAYLIST[0]);

struct PlayerState {
    GtkWidget *title_lbl;
    GtkWidget *artist_lbl;
    GtkWidget *scale;
    GtkWidget *time_lbl;
    GtkWidget *play_btn;
    int  current;
    bool playing;
    double progress;
};
static PlayerState P = {NULL, NULL, NULL, NULL, NULL, 0, false, 0};

static void load_song(int idx) {
    P.current = idx;
    P.progress = 0;
    gtk_label_set_text(GTK_LABEL(P.title_lbl), PLAYLIST[idx].title);
    gtk_label_set_text(GTK_LABEL(P.artist_lbl), PLAYLIST[idx].artist);
    gtk_range_set_value(GTK_RANGE(P.scale), 0);
    char buf[32]; snprintf(buf, sizeof(buf), "0:00 / %d:%02d",
        PLAYLIST[idx].duration / 60, PLAYLIST[idx].duration % 60);
    gtk_label_set_text(GTK_LABEL(P.time_lbl), buf);
}

static gboolean playback_tick(gpointer) {
    if (!P.playing) return G_SOURCE_CONTINUE;
    P.progress += 0.5;
    int dur = PLAYLIST[P.current].duration;
    double pct = (P.progress / dur) * 100.0;
    if (pct > 100) { pct = 100; P.progress = 0; P.current = (P.current + 1) % SONG_COUNT; load_song(P.current); }
    gtk_range_set_value(GTK_RANGE(P.scale), pct);
    int elapsed = (int)P.progress;
    char buf[32]; snprintf(buf, sizeof(buf), "%d:%02d / %d:%02d",
        elapsed/60, elapsed%60, dur/60, dur%60);
    gtk_label_set_text(GTK_LABEL(P.time_lbl), buf);
    return G_SOURCE_CONTINUE;
}

static void on_play(GtkWidget *, gpointer) {
    P.playing = !P.playing;
    gtk_button_set_label(GTK_BUTTON(P.play_btn), P.playing ? "⏸" : "▶");
}
static void on_prev(GtkWidget *, gpointer) {
    P.current = (P.current - 1 + SONG_COUNT) % SONG_COUNT;
    load_song(P.current);
}
static void on_next(GtkWidget *, gpointer) {
    P.current = (P.current + 1) % SONG_COUNT;
    load_song(P.current);
}
static void on_row_activated(GtkListBox *, GtkListBoxRow *row, gpointer) {
    int idx = gtk_list_box_row_get_index(row);
    if (idx >= 0 && idx < SONG_COUNT) { load_song(idx); P.playing = true;
        gtk_button_set_label(GTK_BUTTON(P.play_btn), "⏸"); }
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "🎵 Music Player", "applications-multimedia", 400, 550);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 16); gtk_widget_set_margin_end(vbox, 16);
    gtk_widget_set_margin_top(vbox, 12);   gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* Album art placeholder */
    GtkWidget *art = gtk_label_new("🎧");
    ams_css(art, "title-xl");
    gtk_box_pack_start(GTK_BOX(vbox), art, FALSE, FALSE, 8);

    /* Now playing */
    P.title_lbl = gtk_label_new(PLAYLIST[0].title);
    ams_css(P.title_lbl, "now-playing");
    gtk_box_pack_start(GTK_BOX(vbox), P.title_lbl, FALSE, FALSE, 0);

    P.artist_lbl = gtk_label_new(PLAYLIST[0].artist);
    ams_css(P.artist_lbl, "artist");
    gtk_box_pack_start(GTK_BOX(vbox), P.artist_lbl, FALSE, FALSE, 0);

    /* Seek bar */
    P.scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_scale_set_draw_value(GTK_SCALE(P.scale), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), P.scale, FALSE, FALSE, 4);

    P.time_lbl = gtk_label_new("0:00 / 3:54");
    ams_css(P.time_lbl, "dim");
    gtk_box_pack_start(GTK_BOX(vbox), P.time_lbl, FALSE, FALSE, 0);

    /* Transport controls */
    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(controls, GTK_ALIGN_CENTER);

    GtkWidget *prev = gtk_button_new_with_label("⏮");
    ams_css(prev, "player-btn"); g_signal_connect(prev, "clicked", G_CALLBACK(on_prev), NULL);
    gtk_box_pack_start(GTK_BOX(controls), prev, FALSE, FALSE, 0);

    P.play_btn = gtk_button_new_with_label("▶");
    ams_css(P.play_btn, "play-btn"); g_signal_connect(P.play_btn, "clicked", G_CALLBACK(on_play), NULL);
    gtk_box_pack_start(GTK_BOX(controls), P.play_btn, FALSE, FALSE, 0);

    GtkWidget *next = gtk_button_new_with_label("⏭");
    ams_css(next, "player-btn"); g_signal_connect(next, "clicked", G_CALLBACK(on_next), NULL);
    gtk_box_pack_start(GTK_BOX(controls), next, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), controls, FALSE, FALSE, 8);

    /* Playlist */
    GtkWidget *playlist_lbl = gtk_label_new("Playlist");
    ams_css(playlist_lbl, "accent");
    gtk_widget_set_halign(playlist_lbl, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), playlist_lbl, FALSE, FALSE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *listbox = gtk_list_box_new();
    g_signal_connect(listbox, "row-activated", G_CALLBACK(on_row_activated), NULL);
    for (int i = 0; i < SONG_COUNT; i++) {
        GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
        gtk_widget_set_margin_start(row_box, 8); gtk_widget_set_margin_end(row_box, 8);
        gtk_widget_set_margin_top(row_box, 4);   gtk_widget_set_margin_bottom(row_box, 4);

        char num[8]; snprintf(num, sizeof(num), "%d.", i+1);
        GtkWidget *n = gtk_label_new(num); ams_css(n, "dim");
        gtk_box_pack_start(GTK_BOX(row_box), n, FALSE, FALSE, 0);

        GtkWidget *t = gtk_label_new(PLAYLIST[i].title);
        gtk_box_pack_start(GTK_BOX(row_box), t, TRUE, TRUE, 0);
        gtk_widget_set_halign(t, GTK_ALIGN_START);

        char dur[16]; snprintf(dur, sizeof(dur), "%d:%02d", PLAYLIST[i].duration/60, PLAYLIST[i].duration%60);
        GtkWidget *d = gtk_label_new(dur); ams_css(d, "dim");
        gtk_box_pack_end(GTK_BOX(row_box), d, FALSE, FALSE, 0);

        gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);
    }
    gtk_container_add(GTK_CONTAINER(scroll), listbox);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    load_song(0);
    g_timeout_add(500, playback_tick, NULL);
    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.music", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app); return s;
}
