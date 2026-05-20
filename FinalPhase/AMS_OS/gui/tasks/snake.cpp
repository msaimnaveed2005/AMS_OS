/* AMS OS — GUI Snake Game (cairo-based with arrow key controls) */
#include "../gui_theme.h"

#define COLS 20
#define ROWS 20
#define CELL 25

struct Pos { int x, y; };

struct SnakeState {
    GtkWidget *drawing_area;
    GtkWidget *header;
    std::vector<Pos> body;
    Pos food;
    int dx, dy;
    int score;
    bool alive;
    bool paused;
    guint timer_id;
};
static SnakeState G = {};

static void place_food() {
    bool ok;
    do {
        G.food = {rand() % COLS, rand() % ROWS};
        ok = true;
        for (auto &s : G.body) if (s.x == G.food.x && s.y == G.food.y) { ok = false; break; }
    } while (!ok);
}

static void reset_game() {
    G.body.clear();
    G.body.push_back({COLS/2, ROWS/2});
    G.body.push_back({COLS/2 - 1, ROWS/2});
    G.body.push_back({COLS/2 - 2, ROWS/2});
    G.dx = 1; G.dy = 0;
    G.score = 0; G.alive = true; G.paused = false;
    srand(time(NULL));
    place_food();
}

static void update_score() {
    char buf[32];
    snprintf(buf, sizeof(buf), "Score: %d", G.score);
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(G.header), buf);
}

static gboolean on_draw(GtkWidget *, cairo_t *cr, gpointer) {
    /* Background */
    cairo_set_source_rgb(cr, 0.04, 0.04, 0.04);
    cairo_paint(cr);

    /* Grid lines */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.03);
    cairo_set_line_width(cr, 0.5);
    for (int i = 0; i <= COLS; i++) { cairo_move_to(cr, i*CELL, 0); cairo_line_to(cr, i*CELL, ROWS*CELL); }
    for (int i = 0; i <= ROWS; i++) { cairo_move_to(cr, 0, i*CELL); cairo_line_to(cr, COLS*CELL, i*CELL); }
    cairo_stroke(cr);

    /* Food */
    cairo_set_source_rgb(cr, 0.973, 0.443, 0.443); /* #f87171 */
    cairo_arc(cr, G.food.x * CELL + CELL/2.0, G.food.y * CELL + CELL/2.0, CELL/2.5, 0, 2*M_PI);
    cairo_fill(cr);

    /* Snake */
    for (size_t i = 0; i < G.body.size(); i++) {
        double g = i == 0 ? 0.9 : 0.75 - (double)i / G.body.size() * 0.3;
        cairo_set_source_rgb(cr, 0.2, g, 0.5);
        double x = G.body[i].x * CELL + 1;
        double y = G.body[i].y * CELL + 1;
        double r = 5.0;
        double w = CELL - 2, h = CELL - 2;
        cairo_new_sub_path(cr);
        cairo_arc(cr, x+w-r, y+r, r, -M_PI/2, 0);
        cairo_arc(cr, x+w-r, y+h-r, r, 0, M_PI/2);
        cairo_arc(cr, x+r, y+h-r, r, M_PI/2, M_PI);
        cairo_arc(cr, x+r, y+r, r, M_PI, 3*M_PI/2);
        cairo_close_path(cr);
        cairo_fill(cr);
    }

    /* Game over overlay */
    if (!G.alive) {
        cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
        cairo_paint(cr);
        cairo_set_source_rgb(cr, 0.973, 0.443, 0.443);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 32);
        cairo_move_to(cr, COLS*CELL/2 - 90, ROWS*CELL/2 - 10);
        cairo_show_text(cr, "GAME OVER");
        cairo_set_source_rgba(cr, 1, 1, 1, 0.5);
        cairo_set_font_size(cr, 14);
        char msg[64]; snprintf(msg, sizeof(msg), "Score: %d  —  Press R to restart", G.score);
        cairo_move_to(cr, COLS*CELL/2 - 110, ROWS*CELL/2 + 20);
        cairo_show_text(cr, msg);
    }
    return FALSE;
}

static gboolean game_tick(gpointer) {
    if (!G.alive || G.paused) return G_SOURCE_CONTINUE;

    Pos head = {G.body[0].x + G.dx, G.body[0].y + G.dy};

    /* Wall collision */
    if (head.x < 0 || head.x >= COLS || head.y < 0 || head.y >= ROWS) { G.alive = false; goto redraw; }
    /* Self collision */
    for (auto &s : G.body) if (s.x == head.x && s.y == head.y) { G.alive = false; goto redraw; }

    G.body.insert(G.body.begin(), head);

    if (head.x == G.food.x && head.y == G.food.y) {
        G.score += 10;
        update_score();
        place_food();
    } else {
        G.body.pop_back();
    }

redraw:
    if (G.drawing_area) gtk_widget_queue_draw(G.drawing_area);
    return G_SOURCE_CONTINUE;
}

static gboolean on_key(GtkWidget *, GdkEventKey *ev, gpointer) {
    if (!G.alive && (ev->keyval == GDK_KEY_r || ev->keyval == GDK_KEY_R)) {
        reset_game(); update_score();
        gtk_widget_queue_draw(G.drawing_area);
        return TRUE;
    }
    switch (ev->keyval) {
        case GDK_KEY_Up:    if (G.dy != 1)  { G.dx = 0; G.dy = -1; } break;
        case GDK_KEY_Down:  if (G.dy != -1) { G.dx = 0; G.dy = 1;  } break;
        case GDK_KEY_Left:  if (G.dx != 1)  { G.dx = -1; G.dy = 0; } break;
        case GDK_KEY_Right: if (G.dx != -1) { G.dx = 1; G.dy = 0;  } break;
        case GDK_KEY_space: G.paused = !G.paused; break;
        default: break;
    }
    return TRUE;
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "Snake");
    gtk_window_set_default_size(GTK_WINDOW(win), COLS*CELL + 20, ROWS*CELL + 20);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_icon_name(GTK_WINDOW(win), "applications-games");

    G.header = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(G.header), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(G.header), "🐍 Snake");
    gtk_window_set_titlebar(GTK_WINDOW(win), G.header);

    reset_game();
    update_score();

    GtkWidget *frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(frame, 10);
    gtk_widget_set_margin_end(frame, 10);
    gtk_widget_set_margin_top(frame, 10);
    gtk_widget_set_margin_bottom(frame, 10);
    gtk_container_add(GTK_CONTAINER(win), frame);

    G.drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(G.drawing_area, COLS*CELL, ROWS*CELL);
    ams_css(G.drawing_area, "game-area");
    g_signal_connect(G.drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    gtk_box_pack_start(GTK_BOX(frame), G.drawing_area, TRUE, TRUE, 0);

    g_signal_connect(win, "key-press-event", G_CALLBACK(on_key), NULL);
    G.timer_id = g_timeout_add(150, game_tick, NULL);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.snake", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
