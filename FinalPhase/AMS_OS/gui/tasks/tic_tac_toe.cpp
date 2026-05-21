/*
AMS OS — Tic Tac Toe
Interactive game with premium dark styling, 2-Player mode,
and a highly intelligent minimax-based Vs Copilot AI.
*/

#include "../gui_theme.h"
#include <signal.h>
#include <string>
#include <vector>
#include <algorithm>

/* ══════════════════════════════════════════════════════════════
   Game State
   ══════════════════════════════════════════════════════════════ */

static char grid[3][3];
static char current_player = 'X'; /* 'X' or 'O' */
static GtkWidget *buttons[3][3];
static GtkWidget *status_lbl = NULL;
static bool game_ended = false;
static bool vs_copilot = true; /* default vs copilot */

/* ══════════════════════════════════════════════════════════════
   CSS Styling
   ══════════════════════════════════════════════════════════════ */

static const char *TTT_CSS = R"CSS(
.ttt-btn {
    font-size: 48px;
    font-weight: 800;
    min-width: 90px;
    min-height: 90px;
    border-radius: 12px;
    border: 1px solid rgba(255,255,255,0.08) !important;
    background-color: rgba(255,255,255,0.03) !important;
    background-image: none !important;
    transition: all 150ms ease;
    padding: 0;
}
.ttt-btn:hover {
    background-color: rgba(255,255,255,0.08) !important;
}
.x-mark, .x-mark label {
    color: #60a5fa !important; /* light blue */
    text-shadow: 0 0 10px rgba(96,165,250,0.4);
}
.o-mark, .o-mark label {
    color: #c084fc !important; /* purple */
    text-shadow: 0 0 10px rgba(192,132,252,0.4);
}
.win-highlight {
    background-color: rgba(52,211,153,0.2) !important;
    border-color: #34d399 !important;
    box-shadow: 0 0 15px rgba(52,211,153,0.3);
}
.win-highlight:hover {
    background-color: rgba(52,211,153,0.25) !important;
}
.status-card {
    background-color: rgba(255,255,255,0.04);
    border: 1px solid rgba(255,255,255,0.06);
    border-radius: 10px;
    padding: 10px;
    font-size: 16px;
    font-weight: 700;
}
)CSS";

/* ══════════════════════════════════════════════════════════════
   Game Logic
   ══════════════════════════════════════════════════════════════ */

static bool check_win(char p, int &wr1, int &wc1, int &wr2, int &wc2, int &wr3, int &wc3) {
    // Rows
    for (int r = 0; r < 3; r++) {
        if (grid[r][0] == p && grid[r][1] == p && grid[r][2] == p) {
            wr1 = r; wc1 = 0; wr2 = r; wc2 = 1; wr3 = r; wc3 = 2;
            return true;
        }
    }
    // Cols
    for (int c = 0; c < 3; c++) {
        if (grid[0][c] == p && grid[1][c] == p && grid[2][c] == p) {
            wr1 = 0; wc1 = c; wr2 = 1; wc2 = c; wr3 = 2; wc3 = c;
            return true;
        }
    }
    // Diagonals
    if (grid[0][0] == p && grid[1][1] == p && grid[2][2] == p) {
        wr1 = 0; wc1 = 0; wr2 = 1; wc2 = 1; wr3 = 2; wc3 = 2;
        return true;
    }
    if (grid[0][2] == p && grid[1][1] == p && grid[2][0] == p) {
        wr1 = 0; wc1 = 2; wr2 = 1; wc2 = 1; wr3 = 2; wc3 = 0;
        return true;
    }
    return false;
}

static bool check_draw() {
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            if (grid[r][c] == ' ') return false;
    return true;
}

static void reset_game() {
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            grid[r][c] = ' ';
            gtk_button_set_label(GTK_BUTTON(buttons[r][c]), "");
            GtkStyleContext *ctx = gtk_widget_get_style_context(buttons[r][c]);
            gtk_style_context_remove_class(ctx, "x-mark");
            gtk_style_context_remove_class(ctx, "o-mark");
            gtk_style_context_remove_class(ctx, "win-highlight");
        }
    }
    current_player = 'X';
    game_ended = false;
    gtk_label_set_text(GTK_LABEL(status_lbl), "Your Turn (X)");
}

/* ══════════════════════════════════════════════════════════════
   AI Engine (Minimax)
   ══════════════════════════════════════════════════════════════ */

static int evaluate() {
    int w1, w2, w3, w4, w5, w6;
    if (check_win('O', w1, w2, w3, w4, w5, w6)) return +10;
    if (check_win('X', w1, w2, w3, w4, w5, w6)) return -10;
    return 0;
}

static int minimax(int depth, bool is_max) {
    int score = evaluate();
    if (score == 10) return score - depth;
    if (score == -10) return score + depth;
    if (check_draw()) return 0;

    if (is_max) {
        int best = -1000;
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (grid[r][c] == ' ') {
                    grid[r][c] = 'O';
                    best = std::max(best, minimax(depth + 1, false));
                    grid[r][c] = ' ';
                }
            }
        }
        return best;
    } else {
        int best = 1000;
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (grid[r][c] == ' ') {
                    grid[r][c] = 'X';
                    best = std::min(best, minimax(depth + 1, true));
                    grid[r][c] = ' ';
                }
            }
        }
        return best;
    }
}

static void copilot_move() {
    int best_val = -1000;
    int best_row = -1, best_col = -1;

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (grid[r][c] == ' ') {
                grid[r][c] = 'O';
                int move_val = minimax(0, false);
                grid[r][c] = ' ';
                if (move_val > best_val) {
                    best_row = r;
                    best_col = c;
                    best_val = move_val;
                }
            }
        }
    }

    if (best_row != -1 && best_col != -1) {
        grid[best_row][best_col] = 'O';
        gtk_button_set_label(GTK_BUTTON(buttons[best_row][best_col]), "O");
        GtkStyleContext *ctx = gtk_widget_get_style_context(buttons[best_row][best_col]);
        gtk_style_context_add_class(ctx, "o-mark");
    }
}

/* ══════════════════════════════════════════════════════════════
   Action Handlers
   ══════════════════════════════════════════════════════════════ */

static void on_button_clicked(GtkWidget *widget, gpointer data) {
    if (game_ended) return;
    int pos = GPOINTER_TO_INT(data);
    int r = pos / 3, c = pos % 3;

    if (grid[r][c] != ' ') return;

    /* Player's move */
    grid[r][c] = current_player;
    gtk_button_set_label(GTK_BUTTON(widget), current_player == 'X' ? "X" : "O");
    GtkStyleContext *ctx = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(ctx, current_player == 'X' ? "x-mark" : "o-mark");

    /* Check win */
    int wr1, wc1, wr2, wc2, wr3, wc3;
    if (check_win(current_player, wr1, wc1, wr2, wc2, wr3, wc3)) {
        game_ended = true;
        gtk_style_context_add_class(gtk_widget_get_style_context(buttons[wr1][wc1]), "win-highlight");
        gtk_style_context_add_class(gtk_widget_get_style_context(buttons[wr2][wc2]), "win-highlight");
        gtk_style_context_add_class(gtk_widget_get_style_context(buttons[wr3][wc3]), "win-highlight");
        if (vs_copilot) {
            gtk_label_set_text(GTK_LABEL(status_lbl), "🎉 You Win!");
        } else {
            std::string win_str = "🎉 Player ";
            win_str += current_player;
            win_str += " Wins!";
            gtk_label_set_text(GTK_LABEL(status_lbl), win_str.c_str());
        }
        return;
    }

    /* Check draw */
    if (check_draw()) {
        game_ended = true;
        gtk_label_set_text(GTK_LABEL(status_lbl), "🤝 Draw Game!");
        return;
    }

    if (vs_copilot) {
        /* Copilot AI's turn */
        gtk_label_set_text(GTK_LABEL(status_lbl), "🤖 Copilot is thinking...");
        copilot_move();

        if (check_win('O', wr1, wc1, wr2, wc2, wr3, wc3)) {
            game_ended = true;
            gtk_style_context_add_class(gtk_widget_get_style_context(buttons[wr1][wc1]), "win-highlight");
            gtk_style_context_add_class(gtk_widget_get_style_context(buttons[wr2][wc2]), "win-highlight");
            gtk_style_context_add_class(gtk_widget_get_style_context(buttons[wr3][wc3]), "win-highlight");
            gtk_label_set_text(GTK_LABEL(status_lbl), "🤖 Copilot Wins!");
            return;
        }

        if (check_draw()) {
            game_ended = true;
            gtk_label_set_text(GTK_LABEL(status_lbl), "🤝 Draw Game!");
            return;
        }

        gtk_label_set_text(GTK_LABEL(status_lbl), "Your Turn (X)");
    } else {
        /* Local 2-Player turn toggle */
        current_player = (current_player == 'X') ? 'O' : 'X';
        std::string next_turn = "Player ";
        next_turn += current_player;
        next_turn += "'s Turn";
        gtk_label_set_text(GTK_LABEL(status_lbl), next_turn.c_str());
    }
}

static void on_new_game_clicked(GtkWidget *, gpointer) {
    reset_game();
}

static void on_mode_changed(GtkComboBox *combo, gpointer) {
    int active = gtk_combo_box_get_active(combo);
    vs_copilot = (active == 0);
    reset_game();
}

/* ══════════════════════════════════════════════════════════════
   Application Setup
   ══════════════════════════════════════════════════════════════ */

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();

    /* Load Tic-Tac-Toe Specific CSS */
    GtkCssProvider *cp = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cp, TTT_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(cp),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
    g_object_unref(cp);

    /* ── Main Window ── */
    GtkWidget *win = ams_window(app, "Tic Tac Toe", "applications-games", 350, 480);

    /* Header Bar Buttons */
    GtkWidget *hbar = gtk_window_get_titlebar(GTK_WINDOW(win));

    GtkWidget *new_btn = gtk_button_new_with_label("New Game");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), new_btn);
    g_signal_connect(new_btn, "clicked", G_CALLBACK(on_new_game_clicked), NULL);

    GtkWidget *mode_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "Vs Copilot");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "2 Players");
    gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo), 0);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), mode_combo);
    g_signal_connect(mode_combo, "changed", G_CALLBACK(on_mode_changed), NULL);

    /* ── Main Layout ── */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* Status Label (Glassmorphic Card) */
    GtkWidget *status_card = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    ams_css(status_card, "status-card");
    gtk_widget_set_halign(status_card, GTK_ALIGN_CENTER);

    status_lbl = gtk_label_new("Your Turn (X)");
    gtk_box_pack_start(GTK_BOX(status_card), status_lbl, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), status_card, FALSE, FALSE, 0);

    /* Game Board Grid */
    GtkWidget *grid_box = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid_box), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid_box), 10);
    gtk_widget_set_halign(grid_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid_box, GTK_ALIGN_CENTER);

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            grid[r][c] = ' ';
            buttons[r][c] = gtk_button_new_with_label("");
            ams_css(buttons[r][c], "ttt-btn");
            gtk_widget_set_size_request(buttons[r][c], 90, 90);

            int pos = r * 3 + c;
            g_signal_connect(buttons[r][c], "clicked", G_CALLBACK(on_button_clicked), GINT_TO_POINTER(pos));
            gtk_grid_attach(GTK_GRID(grid_box), buttons[r][c], c, r, 1, 1);
        }
    }

    gtk_box_pack_start(GTK_BOX(vbox), grid_box, TRUE, TRUE, 0);

    gtk_widget_show_all(win);
}

/* ══════════════════════════════════════════════════════════════
   Entry Point
   ══════════════════════════════════════════════════════════════ */

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.ttt", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
