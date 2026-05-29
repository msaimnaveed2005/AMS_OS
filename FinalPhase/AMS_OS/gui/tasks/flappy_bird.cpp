/* AMS OS — GUI Flappy Bird Game (cairo-based, Space to flap) */
#include "../gui_theme.h"

#define WIN_W   400
#define WIN_H   500
#define BIRD_R  12
#define PIPE_W  52
#define PIPE_GAP 130
#define GRAVITY 0.45
#define FLAP_VEL -7.0
#define PIPE_SPEED 2.5
#define TICK_MS 16

struct Pipe { double x; double gap_y; };

struct FlappyState {
    GtkWidget *drawing_area;
    GtkWidget *header;
    double bird_y;
    double bird_vy;
    int score;
    int best;
    bool alive;
    bool started;
    std::vector<Pipe> pipes;
    guint timer_id;
};
static FlappyState G = {};

static void reset_game() {
    G.bird_y = WIN_H / 2.0;
    G.bird_vy = 0;
    G.score = 0;
    G.alive = true;
    G.started = false;
    G.pipes.clear();
    srand(time(NULL));
}

static void spawn_pipe() {
    double gap_y = 80 + rand() % (WIN_H - PIPE_GAP - 120);
    G.pipes.push_back({(double)WIN_W + 20, gap_y});
}

static void update_subtitle() {
    char buf[64];
    snprintf(buf, sizeof(buf), "Score: %d  |  Best: %d", G.score, G.best);
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(G.header), buf);
}

/* ── Rounded rect helper ── */
static void cairo_rounded_rect(cairo_t *cr, double x, double y,
                                double w, double h, double r) {
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r,     r, -M_PI/2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0,        M_PI/2);
    cairo_arc(cr, x + r,     y + h - r, r, M_PI/2,   M_PI);
    cairo_arc(cr, x + r,     y + r,     r, M_PI,      3*M_PI/2);
    cairo_close_path(cr);
}

/* ── Draw callback ── */
static gboolean on_draw(GtkWidget *, cairo_t *cr, gpointer) {
    /* Sky gradient background */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, 0, WIN_H);
    cairo_pattern_add_color_stop_rgb(bg, 0.0, 0.02, 0.04, 0.12);
    cairo_pattern_add_color_stop_rgb(bg, 0.6, 0.04, 0.06, 0.18);
    cairo_pattern_add_color_stop_rgb(bg, 1.0, 0.06, 0.10, 0.25);
    cairo_set_source(cr, bg);
    cairo_paint(cr);
    cairo_pattern_destroy(bg);

    /* Subtle ground */
    cairo_set_source_rgba(cr, 0.15, 0.25, 0.12, 0.6);
    cairo_rectangle(cr, 0, WIN_H - 30, WIN_W, 30);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 0.25, 0.55, 0.20, 0.5);
    cairo_rectangle(cr, 0, WIN_H - 30, WIN_W, 3);
    cairo_fill(cr);

    /* Pipes */
    for (auto &p : G.pipes) {
        /* Top pipe */
        cairo_pattern_t *pg = cairo_pattern_create_linear(p.x, 0, p.x + PIPE_W, 0);
        cairo_pattern_add_color_stop_rgb(pg, 0.0, 0.18, 0.65, 0.35);
        cairo_pattern_add_color_stop_rgb(pg, 0.5, 0.22, 0.80, 0.40);
        cairo_pattern_add_color_stop_rgb(pg, 1.0, 0.15, 0.55, 0.30);
        cairo_set_source(cr, pg);
        cairo_rounded_rect(cr, p.x, -4, PIPE_W, p.gap_y + 4, 4);
        cairo_fill(cr);
        /* Top pipe lip */
        cairo_rounded_rect(cr, p.x - 3, p.gap_y - 20, PIPE_W + 6, 20, 4);
        cairo_fill(cr);

        /* Bottom pipe */
        double bot_y = p.gap_y + PIPE_GAP;
        cairo_set_source(cr, pg);
        cairo_rounded_rect(cr, p.x, bot_y, PIPE_W, WIN_H - bot_y + 4, 4);
        cairo_fill(cr);
        /* Bottom pipe lip */
        cairo_rounded_rect(cr, p.x - 3, bot_y, PIPE_W + 6, 20, 4);
        cairo_fill(cr);
        cairo_pattern_destroy(pg);

        /* Pipe highlight */
        cairo_set_source_rgba(cr, 1, 1, 1, 0.08);
        cairo_rectangle(cr, p.x + 4, 0, 6, p.gap_y);
        cairo_fill(cr);
        cairo_rectangle(cr, p.x + 4, bot_y, 6, WIN_H - bot_y);
        cairo_fill(cr);
    }

    /* Bird body — gradient circle */
    double bx = 80, by = G.bird_y;
    cairo_pattern_t *bird_grad = cairo_pattern_create_radial(
        bx - 3, by - 3, 2, bx, by, BIRD_R);
    cairo_pattern_add_color_stop_rgb(bird_grad, 0.0, 1.0, 0.85, 0.15);
    cairo_pattern_add_color_stop_rgb(bird_grad, 0.7, 0.95, 0.65, 0.10);
    cairo_pattern_add_color_stop_rgb(bird_grad, 1.0, 0.85, 0.45, 0.05);
    cairo_set_source(cr, bird_grad);
    cairo_arc(cr, bx, by, BIRD_R, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(bird_grad);

    /* Bird eye */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_arc(cr, bx + 5, by - 3, 4, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_arc(cr, bx + 6, by - 3, 2, 0, 2 * M_PI);
    cairo_fill(cr);

    /* Bird beak */
    cairo_set_source_rgb(cr, 0.95, 0.45, 0.15);
    cairo_move_to(cr, bx + BIRD_R - 2, by);
    cairo_line_to(cr, bx + BIRD_R + 8, by + 2);
    cairo_line_to(cr, bx + BIRD_R - 2, by + 5);
    cairo_close_path(cr);
    cairo_fill(cr);

    /* Bird wing */
    double wing_angle = G.bird_vy < 0 ? -0.3 : 0.2;
    cairo_set_source_rgba(cr, 0.9, 0.6, 0.1, 0.8);
    cairo_save(cr);
    cairo_translate(cr, bx - 4, by + 2);
    cairo_rotate(cr, wing_angle);
    cairo_scale(cr, 1.0, 0.6);
    cairo_arc(cr, 0, 0, 8, 0, M_PI);
    cairo_fill(cr);
    cairo_restore(cr);

    /* Bird glow */
    cairo_pattern_t *glow = cairo_pattern_create_radial(bx, by, BIRD_R, bx, by, BIRD_R * 2.5);
    cairo_pattern_add_color_stop_rgba(glow, 0.0, 1.0, 0.8, 0.2, 0.15);
    cairo_pattern_add_color_stop_rgba(glow, 1.0, 1.0, 0.8, 0.2, 0.0);
    cairo_set_source(cr, glow);
    cairo_arc(cr, bx, by, BIRD_R * 2.5, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(glow);

    /* Score display (in-game) */
    if (G.started && G.alive) {
        cairo_set_source_rgba(cr, 1, 1, 1, 0.85);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 48);
        char sc[16]; snprintf(sc, sizeof(sc), "%d", G.score);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, sc, &ext);
        double tx = (WIN_W - ext.width) / 2.0 - ext.x_bearing;
        cairo_move_to(cr, tx + 2, 62);
        cairo_set_source_rgba(cr, 0, 0, 0, 0.3);
        cairo_show_text(cr, sc);
        cairo_move_to(cr, tx, 60);
        cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
        cairo_show_text(cr, sc);
    }

    /* "Tap to start" message */
    if (!G.started && G.alive) {
        cairo_set_source_rgba(cr, 1, 1, 1, 0.7);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 22);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, "Press SPACE to Start", &ext);
        cairo_move_to(cr, (WIN_W - ext.width) / 2.0, WIN_H / 2.0 + 60);
        cairo_show_text(cr, "Press SPACE to Start");
    }

    /* Game over overlay */
    if (!G.alive) {
        cairo_set_source_rgba(cr, 0, 0, 0, 0.55);
        cairo_paint(cr);

        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 36);
        cairo_set_source_rgb(cr, 0.973, 0.443, 0.443);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, "GAME OVER", &ext);
        cairo_move_to(cr, (WIN_W - ext.width) / 2.0, WIN_H / 2.0 - 30);
        cairo_show_text(cr, "GAME OVER");

        cairo_set_source_rgba(cr, 1, 1, 1, 0.6);
        cairo_set_font_size(cr, 16);
        char msg[64]; snprintf(msg, sizeof(msg), "Score: %d  |  Best: %d", G.score, G.best);
        cairo_text_extents(cr, msg, &ext);
        cairo_move_to(cr, (WIN_W - ext.width) / 2.0, WIN_H / 2.0 + 10);
        cairo_show_text(cr, msg);

        cairo_set_source_rgba(cr, 1, 1, 1, 0.4);
        cairo_set_font_size(cr, 13);
        const char *hint = "Press R to restart";
        cairo_text_extents(cr, hint, &ext);
        cairo_move_to(cr, (WIN_W - ext.width) / 2.0, WIN_H / 2.0 + 40);
        cairo_show_text(cr, hint);
    }

    return FALSE;
}

/* ── Game tick ── */
static gboolean game_tick(gpointer) {
    if (!G.alive || !G.started) return G_SOURCE_CONTINUE;

    /* Bird physics */
    G.bird_vy += GRAVITY;
    G.bird_y  += G.bird_vy;

    /* Ground / ceiling collision */
    if (G.bird_y + BIRD_R > WIN_H - 30 || G.bird_y - BIRD_R < 0) {
        G.alive = false;
        if (G.score > G.best) G.best = G.score;
        update_subtitle();
        goto redraw;
    }

    /* Spawn pipes */
    if (G.pipes.empty() || G.pipes.back().x < WIN_W - 200) {
        spawn_pipe();
    }

    /* Move pipes & check collisions */
    for (auto it = G.pipes.begin(); it != G.pipes.end(); ) {
        it->x -= PIPE_SPEED;

        /* Score — bird just passed the pipe's right edge */
        double bird_x = 80;
        if (it->x + PIPE_W < bird_x && it->x + PIPE_W >= bird_x - PIPE_SPEED) {
            G.score++;
            update_subtitle();
        }

        /* Collision: bird center at x=80 */
        if (bird_x + BIRD_R > it->x && bird_x - BIRD_R < it->x + PIPE_W) {
            if (G.bird_y - BIRD_R < it->gap_y ||
                G.bird_y + BIRD_R > it->gap_y + PIPE_GAP) {
                G.alive = false;
                if (G.score > G.best) G.best = G.score;
                update_subtitle();
                goto redraw;
            }
        }

        if (it->x + PIPE_W < -10)
            it = G.pipes.erase(it);
        else
            ++it;
    }

redraw:
    if (G.drawing_area) gtk_widget_queue_draw(G.drawing_area);
    return G_SOURCE_CONTINUE;
}

/* ── Key handler ── */
static gboolean on_key(GtkWidget *, GdkEventKey *ev, gpointer) {
    if (!G.alive && (ev->keyval == GDK_KEY_r || ev->keyval == GDK_KEY_R)) {
        reset_game();
        update_subtitle();
        gtk_widget_queue_draw(G.drawing_area);
        return TRUE;
    }
    if (ev->keyval == GDK_KEY_space) {
        if (!G.started) G.started = true;
        if (G.alive) G.bird_vy = FLAP_VEL;
        return TRUE;
    }
    return FALSE;
}

/* ── Activate ── */
static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Flappy Bird");
    gtk_window_set_default_size(GTK_WINDOW(win), WIN_W + 20, WIN_H + 20);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_icon_name(GTK_WINDOW(win), "applications-games");

    G.header = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(G.header), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(G.header), "🐦 Flappy Bird");
    gtk_window_set_titlebar(GTK_WINDOW(win), G.header);

    reset_game();
    update_subtitle();

    GtkWidget *frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(frame, 10);
    gtk_widget_set_margin_end(frame, 10);
    gtk_widget_set_margin_top(frame, 10);
    gtk_widget_set_margin_bottom(frame, 10);
    gtk_container_add(GTK_CONTAINER(win), frame);

    G.drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(G.drawing_area, WIN_W, WIN_H);
    ams_css(G.drawing_area, "game-area");
    g_signal_connect(G.drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    gtk_box_pack_start(GTK_BOX(frame), G.drawing_area, TRUE, TRUE, 0);

    g_signal_connect(win, "key-press-event", G_CALLBACK(on_key), NULL);
    G.timer_id = g_timeout_add(TICK_MS, game_tick, NULL);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.flappy_bird",
                                               G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
