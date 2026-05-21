/* AMS OS — GUI Music Player
   Plays built-in demo playlist + real audio files from data/music/ folder.
   Supports .wav, .mp3, .ogg playback via aplay/ffplay. */
#include "../gui_theme.h"
#include <string>
#include <algorithm>
#include <cctype>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
<<<<<<< HEAD
=======
#include <dirent.h>
>>>>>>> 32ae8bb3ce880439b4d53d4fe1678862b3a64d16

struct Song {
    std::string title;
    std::string artist;
    int duration;
    std::string filepath;  /* empty = simulated, non-empty = real audio */
};

static std::vector<Song> PLAYLIST;
static int SONG_COUNT = 0;

/* ── Audio playback child process ── */
static pid_t audio_pid = -1;

static void stop_audio() {
    if (audio_pid > 0) {
        kill(audio_pid, SIGTERM);
        waitpid(audio_pid, NULL, WNOHANG);
        audio_pid = -1;
    }
}

static void play_audio(const std::string &filepath) {
    stop_audio();
    if (filepath.empty()) return;

    audio_pid = fork();
    if (audio_pid == 0) {
        setsid();
        /* Redirect stdout/stderr to /dev/null for clean output */
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        /* Determine player based on extension */
        std::string ext = "";
        size_t dot = filepath.rfind('.');
        if (dot != std::string::npos) {
            ext = filepath.substr(dot);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }

        if (ext == ".wav") {
            execlp("aplay", "aplay", "-q", filepath.c_str(), (char *)NULL);
            /* Fallback to ffplay if aplay not found */
            execlp("ffplay", "ffplay", "-nodisp", "-autoexit", "-loglevel", "quiet",
                   filepath.c_str(), (char *)NULL);
        } else {
            /* .mp3, .ogg, and others — try ffplay first, then mpg123 */
            execlp("ffplay", "ffplay", "-nodisp", "-autoexit", "-loglevel", "quiet",
                   filepath.c_str(), (char *)NULL);
            execlp("mpg123", "mpg123", "-q", filepath.c_str(), (char *)NULL);
        }
        _exit(1);
    }
}

<<<<<<< HEAD
=======
/* ── Get real duration using ffprobe or WAV header ── */
static int get_audio_duration(const std::string &filepath) {
    /* 1. Try ffprobe (if installed) */
    std::string cmd = "ffprobe -v quiet -show_entries format=duration -of csv=p=0 \"" + filepath + "\" 2>/dev/null";
    FILE *fp = popen(cmd.c_str(), "r");
    if (fp) {
        char buf[64];
        if (fgets(buf, sizeof(buf), fp)) {
            pclose(fp);
            double dur = atof(buf);
            if (dur > 0.5 && dur < 360000) return (int)(dur + 0.5);
        } else {
            pclose(fp);
        }
    }

    /* 2. Native C++ WAV Header Parser */
    if (filepath.length() >= 4 && filepath.substr(filepath.length()-4) == ".wav") {
        std::ifstream f(filepath, std::ios::binary);
        if (f.is_open()) {
            f.seekg(28);
            uint32_t byte_rate = 0;
            f.read((char*)&byte_rate, 4);
            f.seekg(40);
            uint32_t data_size = 0;
            f.read((char*)&data_size, 4);
            if (byte_rate > 0) {
                return data_size / byte_rate;
            }
        }
    }

    /* 3. Fallback to 10 hours so the user is never interrupted */
    return 36000;
}

>>>>>>> 32ae8bb3ce880439b4d53d4fe1678862b3a64d16
/* ── Scan data/music/ directory for audio files ── */
static void scan_music_dir() {
    DIR *dir = opendir("data/music");
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string fname = entry->d_name;
        if (fname == "." || fname == "..") continue;

        /* Check extension */
        size_t dot = fname.rfind('.');
        if (dot == std::string::npos) continue;
        std::string ext = fname.substr(dot);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext != ".wav" && ext != ".mp3" && ext != ".ogg") continue;

        /* Derive title from filename without extension */
        std::string title = fname.substr(0, dot);

        Song s;
        s.title = title;
        s.artist = "📂 Local Library";
        s.duration = 180; /* default 3 minutes */
        s.filepath = "data/music/" + fname;
        PLAYLIST.push_back(s);
    }
    closedir(dir);
}

/* ── Player State ── */
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
static int initial_song_index = 0;
static bool should_autoplay = false;

static void load_song(int idx) {
    stop_audio();
    P.current = idx;
    P.progress = 0;
    gtk_label_set_text(GTK_LABEL(P.title_lbl), PLAYLIST[idx].title.c_str());
    gtk_label_set_text(GTK_LABEL(P.artist_lbl), PLAYLIST[idx].artist.c_str());
    gtk_range_set_value(GTK_RANGE(P.scale), 0);
    char buf[32]; snprintf(buf, sizeof(buf), "0:00 / %d:%02d",
        PLAYLIST[idx].duration / 60, PLAYLIST[idx].duration % 60);
    gtk_label_set_text(GTK_LABEL(P.time_lbl), buf);
}

static void start_playback() {
    P.playing = true;
    gtk_button_set_label(GTK_BUTTON(P.play_btn), "⏸");
    /* If the current song has a real audio file, play it */
    if (!PLAYLIST[P.current].filepath.empty()) {
        play_audio(PLAYLIST[P.current].filepath);
    }
}

static void stop_playback() {
    P.playing = false;
    gtk_button_set_label(GTK_BUTTON(P.play_btn), "▶");
    stop_audio();
}

static gboolean playback_tick(gpointer) {
    if (!P.playing) return G_SOURCE_CONTINUE;
    P.progress += 0.5;
    int dur = PLAYLIST[P.current].duration;
    double pct = (P.progress / dur) * 100.0;
    if (pct > 100) {
        pct = 100;
        P.progress = 0;
        int next = (P.current + 1) % SONG_COUNT;
        load_song(next);
        start_playback();
    }
    gtk_range_set_value(GTK_RANGE(P.scale), pct);
    int elapsed = (int)P.progress;
    char buf[32]; snprintf(buf, sizeof(buf), "%d:%02d / %d:%02d",
        elapsed/60, elapsed%60, dur/60, dur%60);
    gtk_label_set_text(GTK_LABEL(P.time_lbl), buf);
    return G_SOURCE_CONTINUE;
}

static void on_play(GtkWidget *, gpointer) {
    if (P.playing) {
        stop_playback();
    } else {
        start_playback();
    }
}
static void on_prev(GtkWidget *, gpointer) {
    int prev = (P.current - 1 + SONG_COUNT) % SONG_COUNT;
    bool was_playing = P.playing;
    load_song(prev);
    if (was_playing) start_playback();
}
static void on_next(GtkWidget *, gpointer) {
    int next = (P.current + 1) % SONG_COUNT;
    bool was_playing = P.playing;
    load_song(next);
    if (was_playing) start_playback();
}
static void on_row_activated(GtkListBox *, GtkListBoxRow *row, gpointer) {
    int idx = gtk_list_box_row_get_index(row);
    if (idx >= 0 && idx < SONG_COUNT) {
        load_song(idx);
        start_playback();
    }
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
    P.title_lbl = gtk_label_new(PLAYLIST[0].title.c_str());
    ams_css(P.title_lbl, "now-playing");
    gtk_box_pack_start(GTK_BOX(vbox), P.title_lbl, FALSE, FALSE, 0);

    P.artist_lbl = gtk_label_new(PLAYLIST[0].artist.c_str());
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

        GtkWidget *t = gtk_label_new(PLAYLIST[i].title.c_str());
        gtk_box_pack_start(GTK_BOX(row_box), t, TRUE, TRUE, 0);
        gtk_widget_set_halign(t, GTK_ALIGN_START);

        /* Show 🔊 icon for real audio files */
        if (!PLAYLIST[i].filepath.empty()) {
            GtkWidget *audio_icon = gtk_label_new("🔊");
            ams_css(audio_icon, "dim");
            gtk_box_pack_end(GTK_BOX(row_box), audio_icon, FALSE, FALSE, 4);
        }

        char dur[16]; snprintf(dur, sizeof(dur), "%d:%02d", PLAYLIST[i].duration/60, PLAYLIST[i].duration%60);
        GtkWidget *d = gtk_label_new(dur); ams_css(d, "dim");
        gtk_box_pack_end(GTK_BOX(row_box), d, FALSE, FALSE, 0);

        gtk_list_box_insert(GTK_LIST_BOX(listbox), row_box, -1);
    }
    gtk_container_add(GTK_CONTAINER(scroll), listbox);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    load_song(initial_song_index);
    if (should_autoplay) {
        start_playback();
    }
    g_timeout_add(500, playback_tick, NULL);
    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);

    /* Scan for real audio files in data/music/ */
    scan_music_dir();
    
    if (PLAYLIST.empty()) {
        PLAYLIST.push_back({"No music found", "Add songs to data/music/", 0, ""});
    }
    
    SONG_COUNT = (int)PLAYLIST.size();

    /* Handle command-line argument for initial song selection */
    if (argc > 1) {
        std::string query = argv[1];
        std::string q_lower = query;
        std::transform(q_lower.begin(), q_lower.end(), q_lower.begin(), ::tolower);
        for (int i = 0; i < SONG_COUNT; i++) {
            std::string title = PLAYLIST[i].title;
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            if (title.find(q_lower) != std::string::npos) {
                initial_song_index = i;
                should_autoplay = true;
                break;
            }
        }
    }

    GtkApplication *app = gtk_application_new("com.ams.task.music", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);

    /* Clean up audio on exit */
    stop_audio();

    g_object_unref(app); return s;
}
