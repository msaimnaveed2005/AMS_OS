/*
AMS OS — Sudoku Game
Premium 9×9 Sudoku puzzle with multiple difficulties.
Click a cell to select it, press 1-9 to enter, 0/Delete to clear.
*/

#include "../gui_theme.h"
#include <signal.h>
#include <cstdlib>

/* ══════════════════════════════════════════════════════════════
   Puzzle Data — 3 puzzles with solutions
   ══════════════════════════════════════════════════════════════ */

struct Puzzle {
    const char *name;
    int grid[9][9];
    int solution[9][9];
};

static const Puzzle PUZZLES[] = {
    /* Easy */
    {"Easy", {
        {5,3,0, 0,7,0, 0,0,0},
        {6,0,0, 1,9,5, 0,0,0},
        {0,9,8, 0,0,0, 0,6,0},
        {8,0,0, 0,6,0, 0,0,3},
        {4,0,0, 8,0,3, 0,0,1},
        {7,0,0, 0,2,0, 0,0,6},
        {0,6,0, 0,0,0, 2,8,0},
        {0,0,0, 4,1,9, 0,0,5},
        {0,0,0, 0,8,0, 0,7,9}
    }, {
        {5,3,4, 6,7,8, 9,1,2},
        {6,7,2, 1,9,5, 3,4,8},
        {1,9,8, 3,4,2, 5,6,7},
        {8,5,9, 7,6,1, 4,2,3},
        {4,2,6, 8,5,3, 7,9,1},
        {7,1,3, 9,2,4, 8,5,6},
        {9,6,1, 5,3,7, 2,8,4},
        {2,8,7, 4,1,9, 6,3,5},
        {3,4,5, 2,8,6, 1,7,9}
    }},
    /* Medium */
    {"Medium", {
        {0,0,0, 2,6,0, 7,0,1},
        {6,8,0, 0,7,0, 0,9,0},
        {1,9,0, 0,0,4, 5,0,0},
        {8,2,0, 1,0,0, 0,4,0},
        {0,0,4, 6,0,2, 9,0,0},
        {0,5,0, 0,0,3, 0,2,8},
        {0,0,9, 3,0,0, 0,7,4},
        {0,4,0, 0,5,0, 0,3,6},
        {7,0,3, 0,1,8, 0,0,0}
    }, {
        {4,3,5, 2,6,9, 7,8,1},
        {6,8,2, 5,7,1, 4,9,3},
        {1,9,7, 8,3,4, 5,6,2},
        {8,2,6, 1,9,5, 3,4,7},
        {3,7,4, 6,8,2, 9,1,5},
        {9,5,1, 7,4,3, 6,2,8},
        {5,1,9, 3,2,6, 8,7,4},
        {2,4,8, 9,5,7, 1,3,6},
        {7,6,3, 4,1,8, 2,5,9}
    }},
    /* Hard */
    {"Hard", {
        {0,0,0, 6,0,0, 4,0,0},
        {7,0,0, 0,0,3, 6,0,0},
        {0,0,0, 0,9,1, 0,8,0},
        {0,0,0, 0,0,0, 0,0,0},
        {0,5,0, 1,8,0, 0,0,3},
        {0,0,0, 3,0,6, 0,4,5},
        {0,4,0, 2,0,0, 0,6,0},
        {9,0,3, 0,0,0, 0,0,0},
        {0,2,0, 0,0,0, 1,0,0}
    }, {
        {5,8,1, 6,7,2, 4,3,9},
        {7,9,2, 8,4,3, 6,5,1},
        {3,6,4, 5,9,1, 7,8,2},
        {4,3,8, 9,5,7, 2,1,6},
        {2,5,6, 1,8,4, 9,7,3},
        {1,7,9, 3,2,6, 8,4,5},
        {8,4,5, 2,1,9, 3,6,7},
        {9,1,3, 7,6,8, 5,2,4},
        {6,2,7, 4,3,5, 1,9,8}
    }}
};
static const int PUZZLE_COUNT = 3;

/* ══════════════════════════════════════════════════════════════
   Game State
   ══════════════════════════════════════════════════════════════ */

static int board[9][9];        /* current player board */
static bool fixed[9][9];       /* true if cell was pre-filled */
static int current_puzzle = 0;
static int sel_r = -1, sel_c = -1;  /* selected cell */
static int elapsed = 0;        /* seconds */
static guint timer_id = 0;
static bool game_won = false;

static GtkWidget *cells[9][9];
static GtkWidget *timer_label = NULL;
static GtkWidget *diff_label  = NULL;
static GtkWidget *win_ref     = NULL;

/* ══════════════════════════════════════════════════════════════
   Additional CSS
   ══════════════════════════════════════════════════════════════ */

static const char *SUDOKU_CSS = R"CSS(

.sudoku-cell {
    font-size: 18px;
    font-weight: 600;
    min-width: 44px;
    min-height: 44px;
    border-radius: 6px;
    padding: 0;
    border: 1px solid rgba(255,255,255,0.04);
    transition: all 100ms ease;
}

.sudoku-fixed {
    color: rgba(255,255,255,0.85);
    background-color: rgba(255,255,255,0.08);
}

.sudoku-input {
    color: #a78bfa;
    background-color: rgba(0,0,0,0.2);
}

.sudoku-selected {
    border-color: rgba(139,92,246,0.7) !important;
    background-color: rgba(139,92,246,0.2) !important;
    box-shadow: 0 0 12px rgba(139,92,246,0.3);
}

.sudoku-error {
    color: #f87171 !important;
    background-color: rgba(220,38,38,0.15) !important;
    border-color: rgba(220,38,38,0.4) !important;
}

.sudoku-correct {
    color: #34d399 !important;
}

.sudoku-box {
    background-color: rgba(255,255,255,0.03);
    border: 1px solid rgba(255,255,255,0.08);
    border-radius: 10px;
    padding: 3px;
}

.sudoku-timer {
    color: rgba(255,255,255,0.5);
    font-size: 13px;
    font-weight: 600;
}

.sudoku-win {
    font-size: 24px;
    font-weight: 800;
    color: #34d399;
}

)CSS";

/* ══════════════════════════════════════════════════════════════
   Validation
   ══════════════════════════════════════════════════════════════ */

static bool has_conflict(int r, int c) {
    int val = board[r][c];
    if (val == 0) return false;

    /* Check row */
    for (int j = 0; j < 9; j++)
        if (j != c && board[r][j] == val) return true;

    /* Check column */
    for (int i = 0; i < 9; i++)
        if (i != r && board[i][c] == val) return true;

    /* Check 3×3 box */
    int br = (r / 3) * 3, bc = (c / 3) * 3;
    for (int i = br; i < br + 3; i++)
        for (int j = bc; j < bc + 3; j++)
            if ((i != r || j != c) && board[i][j] == val) return true;

    return false;
}

static bool is_board_complete() {
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            if (board[r][c] == 0 || has_conflict(r, c)) return false;
    return true;
}

/* ══════════════════════════════════════════════════════════════
   UI Refresh
   ══════════════════════════════════════════════════════════════ */

static void refresh_board() {
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            GtkStyleContext *ctx = gtk_widget_get_style_context(cells[r][c]);

            /* Clear dynamic classes */
            gtk_style_context_remove_class(ctx, "sudoku-selected");
            gtk_style_context_remove_class(ctx, "sudoku-error");
            gtk_style_context_remove_class(ctx, "sudoku-correct");

            /* Set label */
            if (board[r][c] != 0) {
                char buf[4];
                snprintf(buf, sizeof(buf), "%d", board[r][c]);
                gtk_button_set_label(GTK_BUTTON(cells[r][c]), buf);
            } else {
                gtk_button_set_label(GTK_BUTTON(cells[r][c]), "");
            }

            /* Base style */
            if (fixed[r][c]) {
                gtk_style_context_add_class(ctx, "sudoku-fixed");
            } else {
                gtk_style_context_remove_class(ctx, "sudoku-fixed");
                gtk_style_context_add_class(ctx, "sudoku-input");

                /* Check for conflicts */
                if (board[r][c] != 0 && has_conflict(r, c))
                    gtk_style_context_add_class(ctx, "sudoku-error");
            }

            /* Selection highlight */
            if (r == sel_r && c == sel_c)
                gtk_style_context_add_class(ctx, "sudoku-selected");
        }
    }
}

/* ══════════════════════════════════════════════════════════════
   Timer
   ══════════════════════════════════════════════════════════════ */

static void update_timer_label() {
    char buf[32];
    snprintf(buf, sizeof(buf), "⏱ %02d:%02d", elapsed / 60, elapsed % 60);
    if (timer_label)
        gtk_label_set_text(GTK_LABEL(timer_label), buf);
}

static gboolean on_timer_tick(gpointer) {
    if (game_won) return G_SOURCE_CONTINUE;
    elapsed++;
    update_timer_label();
    return G_SOURCE_CONTINUE;
}

/* ══════════════════════════════════════════════════════════════
   New Game
   ══════════════════════════════════════════════════════════════ */

static void start_new_game(int puzzle_idx) {
    current_puzzle = puzzle_idx % PUZZLE_COUNT;
    const Puzzle &p = PUZZLES[current_puzzle];

    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            board[r][c] = p.grid[r][c];
            fixed[r][c] = (p.grid[r][c] != 0);
        }
    }

    sel_r = sel_c = -1;
    elapsed = 0;
    game_won = false;
    update_timer_label();

    if (diff_label)
        gtk_label_set_text(GTK_LABEL(diff_label), p.name);

    refresh_board();
}

/* ══════════════════════════════════════════════════════════════
   Event Handlers
   ══════════════════════════════════════════════════════════════ */

static void on_cell_clicked(GtkWidget *, gpointer data) {
    if (game_won) return;
    int pos = GPOINTER_TO_INT(data);
    int r = pos / 9, c = pos % 9;

    if (fixed[r][c]) {
        /* Can't select fixed cells for editing, but highlight them */
        sel_r = r; sel_c = c;
    } else {
        sel_r = r; sel_c = c;
    }
    refresh_board();
}

static gboolean on_key_press(GtkWidget *, GdkEventKey *ev, gpointer) {
    if (game_won) return FALSE;
    if (sel_r < 0 || sel_c < 0) return FALSE;
    if (fixed[sel_r][sel_c]) return FALSE;

    int num = -1;

    if (ev->keyval >= GDK_KEY_1 && ev->keyval <= GDK_KEY_9)
        num = ev->keyval - GDK_KEY_0;
    else if (ev->keyval >= GDK_KEY_KP_1 && ev->keyval <= GDK_KEY_KP_9)
        num = ev->keyval - GDK_KEY_KP_0;
    else if (ev->keyval == GDK_KEY_0 || ev->keyval == GDK_KEY_KP_0 ||
             ev->keyval == GDK_KEY_Delete || ev->keyval == GDK_KEY_BackSpace)
        num = 0;

    if (num < 0) return FALSE;

    board[sel_r][sel_c] = num;
    refresh_board();

    /* Check win */
    if (is_board_complete()) {
        game_won = true;
        GtkWidget *dlg = gtk_message_dialog_new(
            GTK_WINDOW(win_ref), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "🎉 Congratulations!\n\nYou completed the %s puzzle in %02d:%02d!",
            PUZZLES[current_puzzle].name, elapsed / 60, elapsed % 60);
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
    }

    return TRUE;
}

static void on_new_game(GtkWidget *, gpointer) {
    current_puzzle = (current_puzzle + 1) % PUZZLE_COUNT;
    start_new_game(current_puzzle);
}

static void on_check(GtkWidget *, gpointer) {
    const Puzzle &p = PUZZLES[current_puzzle];
    int errors = 0, empty = 0;

    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++) {
            if (board[r][c] == 0) empty++;
            else if (board[r][c] != p.solution[r][c]) errors++;
        }

    char msg[256];
    if (errors == 0 && empty == 0)
        snprintf(msg, sizeof(msg), "🎉 Perfect! All cells are correct!");
    else if (errors == 0)
        snprintf(msg, sizeof(msg), "✅ No errors so far! %d cells remaining.", empty);
    else
        snprintf(msg, sizeof(msg), "❌ Found %d incorrect cell%s. %d cells empty.",
                 errors, errors > 1 ? "s" : "", empty);

    GtkWidget *dlg = gtk_message_dialog_new(
        GTK_WINDOW(win_ref), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dlg));
    gtk_widget_destroy(dlg);
}

/* ══════════════════════════════════════════════════════════════
   Application UI
   ══════════════════════════════════════════════════════════════ */

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();

    /* Load Sudoku CSS */
    GtkCssProvider *cp = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cp, SUDOKU_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(cp),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
    g_object_unref(cp);

    /* ── Window ── */
    GtkWidget *win = ams_window(app, "Sudoku", "applications-games", 480, 580);
    win_ref = win;

    /* Header bar extras */
    GtkWidget *hbar = gtk_window_get_titlebar(GTK_WINDOW(win));

    /* Timer label in header */
    timer_label = gtk_label_new("⏱ 00:00");
    ams_css(timer_label, "sudoku-timer");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(hbar), timer_label);

    /* Difficulty label */
    diff_label = gtk_label_new("Easy");
    ams_css(diff_label, "dim");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(hbar), "");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(hbar), diff_label);

    /* New Game button */
    GtkWidget *new_btn = gtk_button_new_with_label("New");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), new_btn);
    g_signal_connect(new_btn, "clicked", G_CALLBACK(on_new_game), NULL);

    /* Check button */
    GtkWidget *check_btn = gtk_button_new_with_label("Check");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), check_btn);
    g_signal_connect(check_btn, "clicked", G_CALLBACK(on_check), NULL);

    /* ── Main layout ── */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(vbox, 16);
    gtk_widget_set_margin_bottom(vbox, 16);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* ── 3×3 outer grid of 3×3 inner grids ── */
    GtkWidget *outer_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(outer_grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(outer_grid), 6);
    gtk_widget_set_halign(outer_grid, GTK_ALIGN_CENTER);

    for (int br = 0; br < 3; br++) {
        for (int bc = 0; bc < 3; bc++) {
            GtkWidget *inner = gtk_grid_new();
            gtk_grid_set_row_spacing(GTK_GRID(inner), 2);
            gtk_grid_set_column_spacing(GTK_GRID(inner), 2);
            ams_css(inner, "sudoku-box");

            for (int ir = 0; ir < 3; ir++) {
                for (int ic = 0; ic < 3; ic++) {
                    int r = br * 3 + ir;
                    int c = bc * 3 + ic;

                    GtkWidget *btn = gtk_button_new_with_label("");
                    ams_css(btn, "sudoku-cell");
                    gtk_widget_set_size_request(btn, 44, 44);

                    int pos = r * 9 + c;
                    g_signal_connect(btn, "clicked", G_CALLBACK(on_cell_clicked),
                                     GINT_TO_POINTER(pos));

                    gtk_grid_attach(GTK_GRID(inner), btn, ic, ir, 1, 1);
                    cells[r][c] = btn;
                }
            }

            gtk_grid_attach(GTK_GRID(outer_grid), inner, bc, br, 1, 1);
        }
    }

    gtk_box_pack_start(GTK_BOX(vbox), outer_grid, FALSE, FALSE, 0);

    /* ── Hint label ── */
    GtkWidget *hint = gtk_label_new("Click a cell, then press 1-9 to fill  ·  0 or Delete to clear");
    ams_css(hint, "dim");
    gtk_widget_set_margin_top(hint, 12);
    gtk_box_pack_start(GTK_BOX(vbox), hint, FALSE, FALSE, 0);

    /* ── Key handler ── */
    g_signal_connect(win, "key-press-event", G_CALLBACK(on_key_press), NULL);

    /* ── Initialize game ── */
    srand((unsigned)time(NULL));
    start_new_game(rand() % PUZZLE_COUNT);

    /* ── Start timer ── */
    timer_id = g_timeout_add_seconds(1, on_timer_tick, NULL);

    gtk_widget_show_all(win);
}

/* ══════════════════════════════════════════════════════════════
   Entry Point
   ══════════════════════════════════════════════════════════════ */

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.sudoku", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
