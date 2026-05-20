/* AMS OS — GUI Minesweeper (9x9, 10 mines) */
#include "../gui_theme.h"

#define SZ 9
#define MINES 10

struct Cell { int value; bool revealed; bool flagged; bool mine; };

struct MineState {
    GtkWidget *buttons[SZ][SZ];
    GtkWidget *mine_lbl;
    GtkWidget *time_lbl;
    Cell       grid[SZ][SZ];
    int        flags;
    int        revealed_count;
    int        seconds;
    bool       game_over;
    bool       first_click;
    guint      timer_id;
};
static MineState M = {};

static const char *NUM_COLORS[] = {
    NULL, "#60a5fa", "#34d399", "#f87171", "#a78bfa",
    "#a16207", "#2dd4bf", "#1e1e1e", "#94a3b8"
};

static void place_mines(int safe_r, int safe_c) {
    srand(time(NULL));
    int placed = 0;
    while (placed < MINES) {
        int r = rand() % SZ, c = rand() % SZ;
        if (M.grid[r][c].mine) continue;
        if (abs(r - safe_r) <= 1 && abs(c - safe_c) <= 1) continue;
        M.grid[r][c].mine = true;
        placed++;
    }
    for (int r = 0; r < SZ; r++)
        for (int c = 0; c < SZ; c++) {
            if (M.grid[r][c].mine) continue;
            int cnt = 0;
            for (int dr = -1; dr <= 1; dr++)
                for (int dc = -1; dc <= 1; dc++) {
                    int nr = r+dr, nc = c+dc;
                    if (nr >= 0 && nr < SZ && nc >= 0 && nc < SZ && M.grid[nr][nc].mine) cnt++;
                }
            M.grid[r][c].value = cnt;
        }
}

static void update_mine_label() {
    char buf[16]; snprintf(buf, sizeof(buf), "💣 %d", MINES - M.flags);
    gtk_label_set_text(GTK_LABEL(M.mine_lbl), buf);
}

static gboolean tick_time(gpointer) {
    if (M.game_over) return G_SOURCE_REMOVE;
    M.seconds++;
    char buf[16]; snprintf(buf, sizeof(buf), "⏱ %d", M.seconds);
    gtk_label_set_text(GTK_LABEL(M.time_lbl), buf);
    return G_SOURCE_CONTINUE;
}

static void reveal_cell(int r, int c);

static void game_end(bool won) {
    M.game_over = true;
    for (int r = 0; r < SZ; r++)
        for (int c = 0; c < SZ; c++) {
            if (M.grid[r][c].mine) {
                gtk_button_set_label(GTK_BUTTON(M.buttons[r][c]), won ? "🚩" : "💥");
                ams_css(M.buttons[r][c], "mine-revealed");
            }
        }
}

static void reveal_cell(int r, int c) {
    if (r < 0 || r >= SZ || c < 0 || c >= SZ) return;
    Cell &cell = M.grid[r][c];
    if (cell.revealed || cell.flagged) return;

    cell.revealed = true;
    M.revealed_count++;
    ams_css(M.buttons[r][c], "mine-revealed");

    if (cell.mine) {
        gtk_button_set_label(GTK_BUTTON(M.buttons[r][c]), "💥");
        game_end(false);
        return;
    }

    if (cell.value > 0) {
        char buf[4]; snprintf(buf, sizeof(buf), "%d", cell.value);
        gtk_button_set_label(GTK_BUTTON(M.buttons[r][c]), buf);

        /* Color the number using inline CSS */
        if (cell.value >= 1 && cell.value <= 8) {
            char css_str[128];
            snprintf(css_str, sizeof(css_str), "* { color: %s; font-weight: 700; }", NUM_COLORS[cell.value]);
            GtkCssProvider *p = gtk_css_provider_new();
            gtk_css_provider_load_from_data(p, css_str, -1, NULL);
            gtk_style_context_add_provider(gtk_widget_get_style_context(M.buttons[r][c]),
                GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
            g_object_unref(p);
        }
    } else {
        gtk_button_set_label(GTK_BUTTON(M.buttons[r][c]), "");
        for (int dr = -1; dr <= 1; dr++)
            for (int dc = -1; dc <= 1; dc++)
                if (dr || dc) reveal_cell(r + dr, c + dc);
    }

    if (M.revealed_count == SZ * SZ - MINES) game_end(true);
}

static void on_click(GtkWidget *, gpointer data) {
    int idx = GPOINTER_TO_INT(data);
    int r = idx / SZ, c = idx % SZ;
    if (M.game_over || M.grid[r][c].flagged) return;
    if (M.first_click) {
        M.first_click = false;
        place_mines(r, c);
        M.timer_id = g_timeout_add_seconds(1, tick_time, NULL);
    }
    reveal_cell(r, c);
}

static gboolean on_right_click(GtkWidget *, GdkEventButton *ev, gpointer data) {
    if (ev->button != 3) return FALSE;
    int idx = GPOINTER_TO_INT(data);
    int r = idx / SZ, c = idx % SZ;
    if (M.game_over || M.grid[r][c].revealed) return TRUE;

    M.grid[r][c].flagged = !M.grid[r][c].flagged;
    M.flags += M.grid[r][c].flagged ? 1 : -1;
    gtk_button_set_label(GTK_BUTTON(M.buttons[r][c]), M.grid[r][c].flagged ? "🚩" : "");
    if (M.grid[r][c].flagged) ams_css(M.buttons[r][c], "mine-flag");
    update_mine_label();
    return TRUE;
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "💣 Minesweeper", "applications-games", 380, 460);

    /* Reset state */
    memset(M.grid, 0, sizeof(M.grid));
    M.flags = 0; M.revealed_count = 0; M.seconds = 0;
    M.game_over = false; M.first_click = true;

    /* Header extras */
    GtkWidget *hbar = gtk_window_get_titlebar(GTK_WINDOW(win));
    M.mine_lbl = gtk_label_new("💣 10");
    ams_css(M.mine_lbl, "game-score");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), M.mine_lbl);
    M.time_lbl = gtk_label_new("⏱ 0");
    ams_css(M.time_lbl, "dim");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(hbar), M.time_lbl);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(vbox, 12);
    gtk_widget_set_margin_end(vbox, 12);
    gtk_widget_set_margin_top(vbox, 8);
    gtk_widget_set_margin_bottom(vbox, 12);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 2);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);

    for (int r = 0; r < SZ; r++)
        for (int c = 0; c < SZ; c++) {
            GtkWidget *btn = gtk_button_new_with_label("");
            ams_css(btn, "mine-btn");
            gpointer idx = GINT_TO_POINTER(r * SZ + c);
            g_signal_connect(btn, "clicked", G_CALLBACK(on_click), idx);
            g_signal_connect(btn, "button-press-event", G_CALLBACK(on_right_click), idx);
            gtk_grid_attach(GTK_GRID(grid), btn, c, r, 1, 1);
            M.buttons[r][c] = btn;
        }

    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);
    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.minesweeper", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
