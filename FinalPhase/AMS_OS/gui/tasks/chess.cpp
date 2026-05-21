/*
AMS OS — Chess Game
Two-player chess with move validation, check detection,
valid move highlighting, captured pieces display, castling,
and an integrated Vs Copilot AI mode.
*/

#include "../gui_theme.h"
#include <signal.h>
#include <cctype>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* ══════════════════════════════════════════════════════════════
   Constants
   ══════════════════════════════════════════════════════════════ */

static const char* piece_unicode(char p) {
    switch (tolower(p)) {
        case 'k': return "♚"; case 'q': return "♛"; case 'r': return "♜";
        case 'b': return "♝"; case 'n': return "♞"; case 'p': return "♟";
        default:  return "";
    }
}

/* ══════════════════════════════════════════════════════════════
   Game State
   ══════════════════════════════════════════════════════════════ */

static char board[8][8];
static bool white_turn  = true;
static bool selected    = false;
static int  sel_r = -1, sel_c = -1;
static bool game_over   = false;
static std::string game_result;

static std::vector<char> captured_white; /* white pieces captured by black */
static std::vector<char> captured_black; /* black pieces captured by white */

static GtkWidget *cell_btns[8][8];
static GtkWidget *turn_label     = NULL;
static GtkWidget *cap_white_lbl  = NULL;
static GtkWidget *cap_black_lbl  = NULL;
static GtkWidget *win_ref        = NULL;

/* Game Mode State */
enum GameMode { MODE_PVP, MODE_VS_COPILOT_WHITE, MODE_VS_COPILOT_BLACK };
static GameMode current_mode = MODE_PVP;
static bool copilot_thinking = false;

/* Castling Flags */
static bool white_king_moved = false;
static bool black_king_moved = false;
static bool white_rook_k_moved = false; /* Column 7 */
static bool white_rook_q_moved = false; /* Column 0 */
static bool black_rook_k_moved = false;
static bool black_rook_q_moved = false;

/* ══════════════════════════════════════════════════════════════
   Additional CSS
   ══════════════════════════════════════════════════════════════ */

static const char *CHESS_CSS = R"CSS(

.chess-light {
    background-color: rgba(255,255,255,0.12) !important;
    background-image: none !important;
    border: none;
    border-radius: 2px;
    min-width: 56px;
    min-height: 56px;
    font-size: 34px;
    padding: 0;
    transition: all 100ms ease;
}

.chess-dark {
    background-color: rgba(139,92,246,0.15) !important;
    background-image: none !important;
    border: none;
    border-radius: 2px;
    min-width: 56px;
    min-height: 56px;
    font-size: 34px;
    padding: 0;
    transition: all 100ms ease;
}

.chess-selected {
    background-color: rgba(99,102,241,0.45) !important;
    background-image: none !important;
    box-shadow: inset 0 0 12px rgba(99,102,241,0.5) !important;
}

.chess-valid {
    background-color: rgba(52,211,153,0.3) !important;
    background-image: none !important;
    box-shadow: inset 0 0 8px rgba(52,211,153,0.3) !important;
}

.chess-valid:hover {
    background-color: rgba(52,211,153,0.5) !important;
    background-image: none !important;
}

.chess-check {
    background-color: rgba(220,38,38,0.35) !important;
    background-image: none !important;
    box-shadow: inset 0 0 12px rgba(220,38,38,0.4) !important;
}

.chess-last-from {
    background-color: rgba(234,179,8,0.15) !important;
    background-image: none !important;
}

.chess-last-to {
    background-color: rgba(234,179,8,0.25) !important;
    background-image: none !important;
}

.chess-board {
    background-color: rgba(0,0,0,0.3);
    border: 2px solid rgba(255,255,255,0.12);
    border-radius: 8px;
    padding: 4px;
}

.chess-turn-white { color: white;   font-size: 14px; font-weight: 700; }
.chess-turn-black { color: #a78bfa; font-size: 14px; font-weight: 700; }

.chess-captured {
    color: rgba(255,255,255,0.5);
    font-size: 18px;
    min-height: 24px;
    padding: 2px 8px;
}

.chess-gameover {
    font-size: 18px;
    font-weight: 800;
    color: #34d399;
}

.chess-rank-label {
    color: rgba(255,255,255,0.3);
    font-size: 11px;
    font-weight: 600;
    min-width: 20px;
}

.chess-file-label {
    color: rgba(255,255,255,0.3);
    font-size: 11px;
    font-weight: 600;
    min-height: 16px;
}

.piece-white, .piece-white label {
    color: #f3f4f6 !important;
}

.piece-black, .piece-black label {
    color: #000000 !important;
    text-shadow: 0px 0px 4px rgba(255, 255, 255, 0.6);
}

)CSS";

/* ══════════════════════════════════════════════════════════════
   Board Helpers
   ══════════════════════════════════════════════════════════════ */

static bool is_white_piece(char p) { return p >= 'A' && p <= 'Z'; }
static bool is_black_piece(char p) { return p >= 'a' && p <= 'z'; }
static bool is_own(char p, bool wt) { return wt ? is_white_piece(p) : is_black_piece(p); }

static void init_board() {
    const char *init =
        "rnbqkbnr"
        "pppppppp"
        "........"
        "........"
        "........"
        "........"
        "PPPPPPPP"
        "RNBQKBNR";
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            board[r][c] = init[r * 8 + c];
}

/* ══════════════════════════════════════════════════════════════
   Path Clear (for rook/bishop/queen)
   ══════════════════════════════════════════════════════════════ */

static bool is_path_clear(int fr, int fc, int tr, int tc) {
    int dr = (tr > fr) ? 1 : (tr < fr) ? -1 : 0;
    int dc = (tc > fc) ? 1 : (tc < fc) ? -1 : 0;
    int r = fr + dr, c = fc + dc;
    while (r != tr || c != tc) {
        if (board[r][c] != '.') return false;
        r += dr;
        c += dc;
    }
    return true;
}

/* ══════════════════════════════════════════════════════════════
   can_reach — for check detection (no check-recursion)
   ══════════════════════════════════════════════════════════════ */

static bool can_reach(int fr, int fc, int tr, int tc) {
    char piece = board[fr][fc];
    if (piece == '.') return false;
    char lower = tolower(piece);
    int dr = abs(tr - fr), dc = abs(tc - fc);

    switch (lower) {
        case 'p': {
            int dir = is_white_piece(piece) ? -1 : 1;
            return ((tr - fr) == dir && abs(tc - fc) == 1);
        }
        case 'r':
            return (fr == tr || fc == tc) && is_path_clear(fr, fc, tr, tc);
        case 'n':
            return (dr == 2 && dc == 1) || (dr == 1 && dc == 2);
        case 'b':
            return (dr == dc && dr > 0) && is_path_clear(fr, fc, tr, tc);
        case 'q':
            return ((fr == tr || fc == tc) || (dr == dc && dr > 0)) &&
                   is_path_clear(fr, fc, tr, tc);
        case 'k':
            return dr <= 1 && dc <= 1 && (dr + dc > 0);
        default:
            return false;
    }
}

/* ══════════════════════════════════════════════════════════════
   Check Detection
   ══════════════════════════════════════════════════════════════ */

static bool is_in_check(bool white_king) {
    char king = white_king ? 'K' : 'k';
    int kr = -1, kc = -1;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (board[r][c] == king) { kr = r; kc = c; }
    if (kr < 0) return true;  /* king missing = effectively in check */

    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++) {
            char p = board[r][c];
            if (p == '.') continue;
            if (white_king && is_white_piece(p)) continue;
            if (!white_king && is_black_piece(p)) continue;
            if (can_reach(r, c, kr, kc)) return true;
        }
    return false;
}

/* ══════════════════════════════════════════════════════════════
   Castling Logic
   ══════════════════════════════════════════════════════════════ */

static bool can_castle(int fr, int fc, int tr, int tc) {
    if (game_over) return false;
    char king = board[fr][fc];
    if (tolower(king) != 'k') return false;
    bool white = is_white_piece(king);
    if (white ? white_king_moved : black_king_moved) return false;
    if (fr != (white ? 7 : 0) || fc != 4) return false;
    if (tr != fr) return false;

    /* Kingside castling */
    if (tc == 6) {
        if (white ? white_rook_k_moved : black_rook_k_moved) return false;
        if (board[fr][5] != '.' || board[fr][6] != '.') return false;
        if (is_in_check(white)) return false;
        /* Check if square passed through is under attack */
        board[fr][5] = king; board[fr][4] = '.';
        bool attacked = is_in_check(white);
        board[fr][4] = king; board[fr][5] = '.';
        if (attacked) return false;
        return true;
    }
    /* Queenside castling */
    if (tc == 2) {
        if (white ? white_rook_q_moved : black_rook_q_moved) return false;
        if (board[fr][1] != '.' || board[fr][2] != '.' || board[fr][3] != '.') return false;
        if (is_in_check(white)) return false;
        /* Check if square passed through is under attack */
        board[fr][3] = king; board[fr][4] = '.';
        bool attacked = is_in_check(white);
        board[fr][4] = king; board[fr][3] = '.';
        if (attacked) return false;
        return true;
    }
    return false;
}

/* ══════════════════════════════════════════════════════════════
   Move Validation (full, including check safety)
   ══════════════════════════════════════════════════════════════ */

static bool is_valid_move(int fr, int fc, int tr, int tc) {
    char piece = board[fr][fc];
    char target = board[tr][tc];
    if (piece == '.') return false;
    if (fr == tr && fc == tc) return false;
    /* Can't capture own piece */
    if (target != '.' && is_white_piece(piece) == is_white_piece(target)) return false;

    char lower = tolower(piece);
    int dr = tr - fr, dc = tc - fc;
    int adr = abs(dr), adc = abs(dc);
    bool valid = false;

    switch (lower) {
        case 'p': {
            int dir = is_white_piece(piece) ? -1 : 1;
            int start_row = is_white_piece(piece) ? 6 : 1;
            if (dc == 0 && dr == dir && target == '.') valid = true;
            else if (dc == 0 && dr == 2 * dir && fr == start_row &&
                     board[fr + dir][fc] == '.' && target == '.') valid = true;
            else if (adc == 1 && dr == dir && target != '.') valid = true;
            break;
        }
        case 'r':
            valid = (fr == tr || fc == tc) && is_path_clear(fr, fc, tr, tc);
            break;
        case 'n':
            valid = (adr == 2 && adc == 1) || (adr == 1 && adc == 2);
            break;
        case 'b':
            valid = (adr == adc && adr > 0) && is_path_clear(fr, fc, tr, tc);
            break;
        case 'q':
            valid = ((fr == tr || fc == tc) || (adr == adc && adr > 0)) &&
                    is_path_clear(fr, fc, tr, tc);
            break;
        case 'k':
            if (adr == 0 && adc == 2) {
                return can_castle(fr, fc, tr, tc);
            }
            valid = adr <= 1 && adc <= 1;
            break;
    }

    if (!valid) return false;

    /* Verify move doesn't leave own king in check */
    char saved_src = board[fr][fc];
    char saved_dst = board[tr][tc];
    board[tr][tc] = saved_src;
    board[fr][fc] = '.';
    bool leaves_check = is_in_check(is_white_piece(saved_src));
    board[fr][fc] = saved_src;
    board[tr][tc] = saved_dst;

    return !leaves_check;
}

/* ══════════════════════════════════════════════════════════════
   Make Move
   ══════════════════════════════════════════════════════════════ */

static int last_fr = -1, last_fc = -1, last_tr = -1, last_tc = -1;

static void make_move(int fr, int fc, int tr, int tc) {
    char piece = board[fr][fc];
    char target = board[tr][tc];

    /* Capture */
    if (target != '.') {
        if (is_white_piece(target))
            captured_white.push_back(target);
        else
            captured_black.push_back(target);
    }

    /* Auto-promote pawn */
    if (tolower(piece) == 'p') {
        if ((is_white_piece(piece) && tr == 0) || (is_black_piece(piece) && tr == 7))
            piece = is_white_piece(piece) ? 'Q' : 'q';
    }

    /* If castling, move the rook as well */
    if (tolower(piece) == 'k' && abs(tc - fc) == 2) {
        if (tc == 6) { /* Kingside */
            board[tr][5] = board[tr][7];
            board[tr][7] = '.';
        } else if (tc == 2) { /* Queenside */
            board[tr][3] = board[tr][0];
            board[tr][0] = '.';
        }
    }

    board[tr][tc] = piece;
    board[fr][fc] = '.';

    last_fr = fr; last_fc = fc;
    last_tr = tr; last_tc = tc;

    /* Track king/rook moves for castling */
    if (fr == 7 && fc == 4) white_king_moved = true;
    if (fr == 0 && fc == 4) black_king_moved = true;
    if (fr == 7 && fc == 7) white_rook_k_moved = true;
    if (fr == 7 && fc == 0) white_rook_q_moved = true;
    if (fr == 0 && fc == 7) black_rook_k_moved = true;
    if (fr == 0 && fc == 0) black_rook_q_moved = true;
}

/* ══════════════════════════════════════════════════════════════
   Checkmate / Stalemate Detection
   ══════════════════════════════════════════════════════════════ */

static bool has_any_legal_move(bool white) {
    for (int fr = 0; fr < 8; fr++)
        for (int fc = 0; fc < 8; fc++) {
            char p = board[fr][fc];
            if (p == '.' || is_white_piece(p) != white) continue;
            for (int tr = 0; tr < 8; tr++)
                for (int tc = 0; tc < 8; tc++)
                    if (is_valid_move(fr, fc, tr, tc)) return true;
        }
    return false;
}

/* ══════════════════════════════════════════════════════════════
   Chess AI — Minimax Depth-2 engine
   ══════════════════════════════════════════════════════════════ */

struct ChessMove {
    int fr, fc, tr, tc;
    int score;
};

static int get_piece_value(char p) {
    switch (p) {
        case 'P': return 100;   case 'p': return -100;
        case 'N': return 320;   case 'n': return -320;
        case 'B': return 330;   case 'b': return -330;
        case 'R': return 500;   case 'r': return -500;
        case 'Q': return 900;   case 'q': return -900;
        case 'K': return 20000;  case 'k': return -20000;
        default: return 0;
    }
}

static int evaluate_board() {
    int score = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            char p = board[r][c];
            if (p == '.') continue;
            score += get_piece_value(p);
            
            /* Positional bonus: control of center */
            if (r >= 3 && r <= 4 && c >= 3 && c <= 4) {
                score += (is_white_piece(p) ? 12 : -12);
            } else if (r >= 2 && r <= 5 && c >= 2 && c <= 5) {
                score += (is_white_piece(p) ? 5 : -5);
            }
        }
    }
    return score;
}

static std::vector<ChessMove> get_all_legal_moves(bool white) {
    std::vector<ChessMove> moves;
    for (int fr = 0; fr < 8; fr++) {
        for (int fc = 0; fc < 8; fc++) {
            char p = board[fr][fc];
            if (p == '.' || is_white_piece(p) != white) continue;
            for (int tr = 0; tr < 8; tr++) {
                for (int tc = 0; tc < 8; tc++) {
                    if (is_valid_move(fr, fc, tr, tc)) {
                        moves.push_back({fr, fc, tr, tc, 0});
                    }
                }
            }
        }
    }
    return moves;
}

static int minimax(int depth, bool maximizing, int alpha, int beta) {
    if (depth == 0) {
        return evaluate_board();
    }

    std::vector<ChessMove> moves = get_all_legal_moves(maximizing);
    if (moves.empty()) {
        if (is_in_check(maximizing)) {
            return maximizing ? -999999 + (2 - depth) : 999999 - (2 - depth);
        }
        return 0;
    }

    if (maximizing) {
        int max_eval = -9999999;
        for (auto &m : moves) {
            char saved_src = board[m.fr][m.fc];
            char saved_dst = board[m.tr][m.tc];
            board[m.tr][m.tc] = saved_src;
            board[m.fr][m.fc] = '.';
            
            int eval = minimax(depth - 1, false, alpha, beta);
            
            board[m.fr][m.fc] = saved_src;
            board[m.tr][m.tc] = saved_dst;

            max_eval = std::max(max_eval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        int min_eval = 9999999;
        for (auto &m : moves) {
            char saved_src = board[m.fr][m.fc];
            char saved_dst = board[m.tr][m.tc];
            board[m.tr][m.tc] = saved_src;
            board[m.fr][m.fc] = '.';

            int eval = minimax(depth - 1, true, alpha, beta);

            board[m.fr][m.fc] = saved_src;
            board[m.tr][m.tc] = saved_dst;

            min_eval = std::min(min_eval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

static ChessMove get_best_move(bool white) {
    std::vector<ChessMove> moves = get_all_legal_moves(white);
    if (moves.empty()) return {-1, -1, -1, -1, 0};

    /* Shuffle moves slightly */
    for (size_t i = 0; i < moves.size(); i++) {
        size_t j = i + rand() % (moves.size() - i);
        std::swap(moves[i], moves[j]);
    }

    ChessMove best_move = moves[0];
    int best_val = white ? -9999999 : 9999999;

    for (auto &m : moves) {
        char saved_src = board[m.fr][m.fc];
        char saved_dst = board[m.tr][m.tc];
        board[m.tr][m.tc] = saved_src;
        board[m.fr][m.fc] = '.';

        int eval = minimax(1, !white, -9999999, 9999999);

        board[m.fr][m.fc] = saved_src;
        board[m.tr][m.tc] = saved_dst;

        if (white) {
            if (eval > best_val) {
                best_val = eval;
                best_move = m;
            }
        } else {
            if (eval < best_val) {
                best_val = eval;
                best_move = m;
            }
        }
    }
    return best_move;
}

/* ══════════════════════════════════════════════════════════════
   UI Refresh
   ══════════════════════════════════════════════════════════════ */

static void refresh_ui() {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            GtkStyleContext *ctx = gtk_widget_get_style_context(cell_btns[r][c]);

            /* Remove dynamic classes */
            gtk_style_context_remove_class(ctx, "chess-selected");
            gtk_style_context_remove_class(ctx, "chess-valid");
            gtk_style_context_remove_class(ctx, "chess-check");
            gtk_style_context_remove_class(ctx, "chess-last-from");
            gtk_style_context_remove_class(ctx, "chess-last-to");
            gtk_style_context_remove_class(ctx, "piece-white");
            gtk_style_context_remove_class(ctx, "piece-black");

            /* Piece label */
            char p = board[r][c];
            gtk_button_set_label(GTK_BUTTON(cell_btns[r][c]),
                                 piece_unicode(p));

            if (is_white_piece(p)) {
                gtk_style_context_add_class(ctx, "piece-white");
            } else if (is_black_piece(p)) {
                gtk_style_context_add_class(ctx, "piece-black");
            }

            /* Last move highlight */
            if (r == last_fr && c == last_fc)
                gtk_style_context_add_class(ctx, "chess-last-from");
            if (r == last_tr && c == last_tc)
                gtk_style_context_add_class(ctx, "chess-last-to");

            /* Selection & valid moves */
            if (selected && !copilot_thinking) {
                if (r == sel_r && c == sel_c)
                    gtk_style_context_add_class(ctx, "chess-selected");
                else if (is_valid_move(sel_r, sel_c, r, c))
                    gtk_style_context_add_class(ctx, "chess-valid");
            }

            /* Check highlight on king */
            char king = white_turn ? 'K' : 'k';
            if (board[r][c] == king && is_in_check(white_turn))
                gtk_style_context_add_class(ctx, "chess-check");
        }
    }

    /* Turn label */
    if (turn_label) {
        if (game_over) {
            gtk_label_set_text(GTK_LABEL(turn_label), game_result.c_str());
            ams_css(turn_label, "chess-gameover");
        } else if (copilot_thinking) {
            gtk_label_set_text(GTK_LABEL(turn_label), "✦ Copilot is thinking...");
            GtkStyleContext *ctx = gtk_widget_get_style_context(turn_label);
            gtk_style_context_remove_class(ctx, "chess-turn-white");
            gtk_style_context_remove_class(ctx, "chess-turn-black");
            gtk_style_context_remove_class(ctx, "chess-gameover");
            ams_css(turn_label, "chess-turn-black");
        } else {
            bool cop_turn = (current_mode == MODE_VS_COPILOT_WHITE && !white_turn) ||
                            (current_mode == MODE_VS_COPILOT_BLACK && white_turn);
            
            std::string status_text;
            if (cop_turn) {
                status_text = "✦ Copilot's Turn";
            } else {
                status_text = white_turn ? "⬜ White's Turn" : "⬛ Black's Turn";
                if (current_mode != MODE_PVP) {
                    status_text += " (You)";
                }
            }
            gtk_label_set_text(GTK_LABEL(turn_label), status_text.c_str());

            GtkStyleContext *ctx = gtk_widget_get_style_context(turn_label);
            gtk_style_context_remove_class(ctx, "chess-turn-white");
            gtk_style_context_remove_class(ctx, "chess-turn-black");
            gtk_style_context_remove_class(ctx, "chess-gameover");
            ams_css(turn_label, white_turn ? "chess-turn-white" : "chess-turn-black");
        }
    }

    /* Captured pieces */
    if (cap_white_lbl) {
        std::string s;
        for (char c : captured_white) s += piece_unicode(c);
        if (s.empty()) s = "—";
        gtk_label_set_text(GTK_LABEL(cap_white_lbl), s.c_str());
    }
    if (cap_black_lbl) {
        std::string s;
        for (char c : captured_black) s += piece_unicode(c);
        if (s.empty()) s = "—";
        gtk_label_set_text(GTK_LABEL(cap_black_lbl), s.c_str());
    }
}

/* ══════════════════════════════════════════════════════════════
   AI Move Trigger & Execution
   ══════════════════════════════════════════════════════════════ */

static gboolean do_copilot_move(gpointer) {
    if (game_over) {
        copilot_thinking = false;
        return G_SOURCE_REMOVE;
    }

    ChessMove m = get_best_move(white_turn);
    if (m.fr != -1) {
        make_move(m.fr, m.fc, m.tr, m.tc);
        white_turn = !white_turn;
        
        /* Check for checkmate / stalemate */
        if (!has_any_legal_move(white_turn)) {
            game_over = true;
            if (is_in_check(white_turn))
                game_result = white_turn
                    ? "♚ Checkmate! Black wins! 🎉"
                    : "♔ Checkmate! White wins! 🎉";
            else
                game_result = "Stalemate — Draw! 🤝";
        }
    }

    copilot_thinking = false;
    refresh_ui();
    return G_SOURCE_REMOVE;
}

static void trigger_copilot_move() {
    copilot_thinking = true;
    if (turn_label) {
        gtk_label_set_text(GTK_LABEL(turn_label), "✦ Copilot is thinking...");
        GtkStyleContext *ctx = gtk_widget_get_style_context(turn_label);
        gtk_style_context_remove_class(ctx, "chess-turn-white");
        gtk_style_context_remove_class(ctx, "chess-turn-black");
        ams_css(turn_label, "chess-turn-black");
    }
    g_timeout_add(600, do_copilot_move, NULL);
}

/* ══════════════════════════════════════════════════════════════
   Cell Click Handler
   ══════════════════════════════════════════════════════════════ */

static void on_cell_click(GtkWidget *, gpointer data) {
    if (game_over || copilot_thinking) return;
    int pos = GPOINTER_TO_INT(data);
    int r = pos / 8, c = pos % 8;

    /* Prevent clicking if it is Copilot's turn */
    bool is_copilot_turn = (current_mode == MODE_VS_COPILOT_WHITE && !white_turn) ||
                           (current_mode == MODE_VS_COPILOT_BLACK && white_turn);
    if (is_copilot_turn) return;

    if (selected) {
        if (r == sel_r && c == sel_c) {
            /* Deselect */
            selected = false;
        } else if (is_valid_move(sel_r, sel_c, r, c)) {
            /* Execute move */
            make_move(sel_r, sel_c, r, c);
            white_turn = !white_turn;
            selected = false;

            /* Check for checkmate / stalemate */
            if (!has_any_legal_move(white_turn)) {
                game_over = true;
                if (is_in_check(white_turn))
                    game_result = white_turn
                        ? "♚ Checkmate! Black wins! 🎉"
                        : "♔ Checkmate! White wins! 🎉";
                else
                    game_result = "Stalemate — Draw! 🤝";
            } else {
                /* Check if Copilot plays next */
                bool next_is_copilot = (current_mode == MODE_VS_COPILOT_WHITE && !white_turn) ||
                                       (current_mode == MODE_VS_COPILOT_BLACK && white_turn);
                if (next_is_copilot) {
                    trigger_copilot_move();
                }
            }
        } else if (is_own(board[r][c], white_turn)) {
            /* Select different piece */
            sel_r = r; sel_c = c;
        } else {
            selected = false;
        }
    } else {
        if (is_own(board[r][c], white_turn)) {
            sel_r = r; sel_c = c;
            selected = true;
        }
    }

    refresh_ui();
}

/* ══════════════════════════════════════════════════════════════
   New Game
   ══════════════════════════════════════════════════════════════ */

static void on_new_game(GtkWidget *, gpointer) {
    init_board();
    white_turn = true;
    selected = false;
    game_over = false;
    copilot_thinking = false;
    game_result.clear();
    captured_white.clear();
    captured_black.clear();
    last_fr = last_fc = last_tr = last_tc = -1;

    white_king_moved = black_king_moved = false;
    white_rook_k_moved = white_rook_q_moved = false;
    black_rook_k_moved = black_rook_q_moved = false;

    refresh_ui();

    /* If Copilot plays White, trigger immediate first move! */
    if (current_mode == MODE_VS_COPILOT_BLACK) {
        trigger_copilot_move();
    }
}

static void on_mode_changed(GtkComboBox *combo, gpointer) {
    int active = gtk_combo_box_get_active(combo);
    if (active == 0) current_mode = MODE_PVP;
    else if (active == 1) current_mode = MODE_VS_COPILOT_WHITE;
    else if (active == 2) current_mode = MODE_VS_COPILOT_BLACK;
    
    on_new_game(NULL, NULL);
}

/* ══════════════════════════════════════════════════════════════
   Application UI
   ══════════════════════════════════════════════════════════════ */

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();

    /* Load Chess CSS */
    GtkCssProvider *cp = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cp, CHESS_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(cp),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
    g_object_unref(cp);

    /* ── Window ── */
    GtkWidget *win = ams_window(app, "Chess", "applications-games", 540, 640);
    win_ref = win;

    /* Header bar extras */
    GtkWidget *hbar = gtk_window_get_titlebar(GTK_WINDOW(win));

    GtkWidget *new_btn = gtk_button_new_with_label("New Game");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), new_btn);
    g_signal_connect(new_btn, "clicked", G_CALLBACK(on_new_game), NULL);

    /* Game Mode Combo Box */
    GtkWidget *mode_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "Mode: 2 Players");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "Vs Copilot (You: White)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "Vs Copilot (You: Black)");
    gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo), 0);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(hbar), mode_combo);
    g_signal_connect(mode_combo, "changed", G_CALLBACK(on_mode_changed), NULL);

    /* ── Main layout ── */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(vbox, 8);
    gtk_widget_set_margin_bottom(vbox, 8);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* ── Turn indicator ── */
    turn_label = gtk_label_new("⬜ White's Turn");
    ams_css(turn_label, "chess-turn-white");
    gtk_widget_set_margin_bottom(turn_label, 4);
    gtk_box_pack_start(GTK_BOX(vbox), turn_label, FALSE, FALSE, 0);

    /* ── Captured by White (Black pieces taken) ── */
    GtkWidget *cap_row_b = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign(cap_row_b, GTK_ALIGN_CENTER);
    GtkWidget *cap_b_lbl = gtk_label_new("⬜ Captured: ");
    ams_css(cap_b_lbl, "dim");
    cap_black_lbl = gtk_label_new("—");
    ams_css(cap_black_lbl, "chess-captured");
    gtk_box_pack_start(GTK_BOX(cap_row_b), cap_b_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cap_row_b), cap_black_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), cap_row_b, FALSE, FALSE, 0);

    /* ── Board with coordinates ── */
    GtkWidget *board_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    ams_css(board_frame, "chess-board");
    gtk_widget_set_halign(board_frame, GTK_ALIGN_CENTER);

    /* File labels (top) */
    GtkWidget *top_files = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_start(top_files, 20); /* offset for rank labels */
    for (int c = 0; c < 8; c++) {
        char fl[4]; snprintf(fl, sizeof(fl), "%c", 'a' + c);
        GtkWidget *lbl = gtk_label_new(fl);
        ams_css(lbl, "chess-file-label");
        gtk_widget_set_size_request(lbl, 56, -1);
        gtk_widget_set_halign(lbl, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(top_files), lbl, FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(board_frame), top_files, FALSE, FALSE, 0);

    /* Board grid with rank labels */
    for (int r = 0; r < 8; r++) {
        GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        /* Rank label */
        char rk[4]; snprintf(rk, sizeof(rk), "%d", 8 - r);
        GtkWidget *rank_lbl = gtk_label_new(rk);
        ams_css(rank_lbl, "chess-rank-label");
        gtk_widget_set_halign(rank_lbl, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(rank_lbl, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(row_box), rank_lbl, FALSE, FALSE, 0);

        for (int c = 0; c < 8; c++) {
            GtkWidget *btn = gtk_button_new_with_label("");
            bool light = (r + c) % 2 == 0;
            ams_css(btn, light ? "chess-light" : "chess-dark");
            gtk_widget_set_size_request(btn, 56, 56);

            int pos = r * 8 + c;
            g_signal_connect(btn, "clicked", G_CALLBACK(on_cell_click),
                             GINT_TO_POINTER(pos));

            gtk_box_pack_start(GTK_BOX(row_box), btn, FALSE, FALSE, 0);
            cell_btns[r][c] = btn;
        }

        gtk_box_pack_start(GTK_BOX(board_frame), row_box, FALSE, FALSE, 0);
    }

    gtk_box_pack_start(GTK_BOX(vbox), board_frame, FALSE, FALSE, 0);

    /* ── Captured by Black (White pieces taken) ── */
    GtkWidget *cap_row_w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign(cap_row_w, GTK_ALIGN_CENTER);
    GtkWidget *cap_w_lbl = gtk_label_new("⬛ Captured: ");
    ams_css(cap_w_lbl, "dim");
    cap_white_lbl = gtk_label_new("—");
    ams_css(cap_white_lbl, "chess-captured");
    gtk_box_pack_start(GTK_BOX(cap_row_w), cap_w_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cap_row_w), cap_white_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), cap_row_w, FALSE, FALSE, 0);

    /* ── Initialize ── */
    init_board();
    gtk_widget_show_all(win);
    refresh_ui();
}

/* ══════════════════════════════════════════════════════════════
   Entry Point
   ══════════════════════════════════════════════════════════════ */

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    srand(time(NULL));
    
    GtkApplication *app = gtk_application_new("com.ams.task.chess", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
