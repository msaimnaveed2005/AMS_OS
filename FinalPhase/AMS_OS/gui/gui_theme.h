/*
AMS OS — Shared GUI Task Theme
Comprehensive dark theme CSS and helper functions shared by all GUI tasks.
Include this header in any GUI task .cpp file for consistent styling.
*/
#pragma once

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>

/* ══════════════════════════════════════════════════════════════
   CSS Theme — Premium Dark Mode for all AMS OS GUI Tasks
   ══════════════════════════════════════════════════════════════ */

static const char *AMS_TASK_CSS = R"CSS(

/* ── Base Window ── */
window { background-color: #111827; }

/* ── Header Bar ── */
headerbar {
    background-image: linear-gradient(to right, #1e1b4b, #172554);
    border-bottom: 1px solid rgba(255,255,255,0.08);
    min-height: 38px;
    padding: 0 8px;
}
headerbar .title   { color: rgba(255,255,255,0.95); font-weight: 700; font-size: 14px; }
headerbar .subtitle{ color: rgba(255,255,255,0.4);  font-size: 11px; }
headerbar button   { background: transparent; border: none; color: rgba(255,255,255,0.7); border-radius: 6px; padding: 4px 8px; min-height: 24px; }
headerbar button:hover { background-color: rgba(255,255,255,0.1); color: white; }

/* ── Buttons ── */
button {
    background-color: rgba(255,255,255,0.07);
    color: rgba(255,255,255,0.9);
    border: 1px solid rgba(255,255,255,0.08);
    border-radius: 10px;
    padding: 8px 16px;
    font-size: 13px;
    transition: all 150ms ease-in-out;
    min-height: 20px;
}
button:hover  { background-color: rgba(139,92,246,0.2); border-color: rgba(139,92,246,0.35); }
button:active { background-color: rgba(139,92,246,0.35); }
button.suggested-action       { background-color: rgba(99,102,241,0.4);  border-color: rgba(99,102,241,0.5);  color: white; }
button.suggested-action:hover { background-color: rgba(99,102,241,0.55); }
button.destructive-action       { background-color: rgba(220,38,38,0.3);  border-color: rgba(220,38,38,0.4);  color: #fca5a5; }
button.destructive-action:hover { background-color: rgba(220,38,38,0.45); }

/* ── Entries ── */
entry {
    background-color: rgba(255,255,255,0.07);
    color: white;
    border: 1px solid rgba(255,255,255,0.08);
    border-radius: 8px;
    padding: 8px 12px;
    caret-color: white;
    font-size: 13px;
}
entry:focus { border-color: rgba(139,92,246,0.5); background-color: rgba(255,255,255,0.1); }

/* ── Labels ── */
label         { color: rgba(255,255,255,0.88); }
.dim          { color: rgba(255,255,255,0.4); }
.accent       { color: #a78bfa; }
.success      { color: #34d399; }
.error-text   { color: #f87171; }
.title-lg     { font-size: 28px; font-weight: 800; color: white; }
.title-xl     { font-size: 64px; font-weight: 200; color: white; }
.subtitle-lbl { font-size: 13px; color: rgba(255,255,255,0.5); }

/* ── Text View ── */
textview      { background-color: rgba(0,0,0,0.25); color: rgba(255,255,255,0.88); font-size: 13px; padding: 12px; }
textview text { background-color: transparent; color: rgba(255,255,255,0.88); }

/* ── Scrollbar ── */
scrollbar slider       { background-color: rgba(255,255,255,0.12); border-radius: 100px; min-width: 6px; min-height: 6px; }
scrollbar slider:hover { background-color: rgba(255,255,255,0.22); }

/* ── Progress Bar ── */
progressbar trough   { background-color: rgba(255,255,255,0.06); border-radius: 100px; min-height: 8px; }
progressbar progress { background-image: linear-gradient(to right, #6366f1, #a855f7); border-radius: 100px; min-height: 8px; }

/* ── Calendar Widget ── */
calendar               { background-color: rgba(255,255,255,0.04); color: white; border-radius: 12px; padding: 12px; font-size: 13px; }
calendar:selected      { background-color: #6366f1; color: white; border-radius: 6px; }
calendar:indeterminate { color: rgba(255,255,255,0.2); }

/* ── TreeView ── */
treeview          { background-color: rgba(0,0,0,0.2); color: rgba(255,255,255,0.85); font-size: 12px; }
treeview:selected { background-color: rgba(139,92,246,0.25); }
treeview header button { background-color: rgba(255,255,255,0.04); color: rgba(255,255,255,0.5); border: none; border-bottom: 1px solid rgba(255,255,255,0.08); border-radius: 0; font-weight: 600; font-size: 11px; }

/* ── Separator ── */
separator { background-color: rgba(255,255,255,0.06); min-height: 1px; }

/* ── Scale (slider) ── */
scale trough    { background-color: rgba(255,255,255,0.08); border-radius: 100px; min-height: 4px; }
scale highlight { background-image: linear-gradient(to right, #6366f1, #a855f7); border-radius: 100px; min-height: 4px; }
scale slider    { background-color: white; border-radius: 100px; min-width: 16px; min-height: 16px; }

/* ── Spinner ── */
spinner { color: #a78bfa; }

/* ── Card Panel ── */
.card { background-color: rgba(255,255,255,0.04); border: 1px solid rgba(255,255,255,0.06); border-radius: 16px; padding: 16px; }

/* ═══ Calculator ═══ */
.calc-display { background-color: rgba(0,0,0,0.4); border-radius: 12px; padding: 16px 20px; margin: 8px; }
.calc-result  { font-size: 42px; font-weight: 300; color: white; }
.calc-expr    { font-size: 14px; color: rgba(255,255,255,0.4); }
.calc-num     { background-color: rgba(255,255,255,0.1);  color: white; font-size: 20px; font-weight: 500; border-radius: 50px; min-width: 60px; min-height: 60px; border: none; }
.calc-num:hover { background-color: rgba(255,255,255,0.18); }
.calc-op      { background-color: rgba(245,158,11,0.7);  color: white; font-size: 20px; font-weight: 600; border-radius: 50px; min-width: 60px; min-height: 60px; border: none; }
.calc-op:hover  { background-color: rgba(245,158,11,0.9); }
.calc-func    { background-color: rgba(255,255,255,0.05); color: rgba(255,255,255,0.7); font-size: 18px; border-radius: 50px; min-width: 60px; min-height: 60px; border: none; }
.calc-func:hover { background-color: rgba(255,255,255,0.12); }

/* ═══ Snake / Minesweeper ═══ */
.game-area       { background-color: #0a0a0a; border-radius: 8px; }
.game-score      { color: #34d399; font-size: 16px; font-weight: 700; }
.game-over-label { font-size: 32px; font-weight: 800; color: #f87171; }
.mine-btn          { background-color: rgba(255,255,255,0.08); border: 1px solid rgba(255,255,255,0.05); border-radius: 4px; min-width: 36px; min-height: 36px; font-size: 16px; padding: 0; }
.mine-btn:hover    { background-color: rgba(255,255,255,0.15); }
.mine-revealed     { background-color: rgba(0,0,0,0.3); border-color: rgba(255,255,255,0.03); }
.mine-flag         { background-color: rgba(234,179,8,0.2); }

/* ═══ Music Player ═══ */
.now-playing { font-size: 18px; font-weight: 700; color: white; }
.artist      { font-size: 13px; color: rgba(255,255,255,0.5); }
.player-btn  { background: transparent; border: none; color: white; font-size: 24px; border-radius: 100px; min-width: 48px; min-height: 48px; }
.player-btn:hover { background-color: rgba(255,255,255,0.1); }
.play-btn    { background-color: rgba(139,92,246,0.4); border: none; color: white; font-size: 28px; border-radius: 100px; min-width: 56px; min-height: 56px; }
.play-btn:hover { background-color: rgba(139,92,246,0.6); }

/* ═══ File/Info Cards ═══ */
.info-row   { padding: 6px 0; }
.info-key   { color: rgba(255,255,255,0.5); font-size: 12px; font-weight: 600; }
.info-value { color: white; font-size: 14px; }

)CSS";

/* ══════════════════════════════════════════════════════════════
   Helper Functions — used by every GUI task
   ══════════════════════════════════════════════════════════════ */

static inline void ams_apply_theme() {
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_data(p, AMS_TASK_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}

static inline void ams_css(GtkWidget *w, const char *cls) {
    gtk_style_context_add_class(gtk_widget_get_style_context(w), cls);
}

/* Create a standard AMS OS task window with a custom header bar */
static inline GtkWidget* ams_window(GtkApplication *app,
                                     const char *title,
                                     const char *icon_name,
                                     int w, int h) {
    GtkWidget *win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), title);
    gtk_window_set_default_size(GTK_WINDOW(win), w, h);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    if (icon_name)
        gtk_window_set_icon_name(GTK_WINDOW(win), icon_name);

    GtkWidget *hbar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(hbar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(hbar), title);
    gtk_window_set_titlebar(GTK_WINDOW(win), hbar);
    return win;
}

/* Convenience: create a labeled section card */
static inline GtkWidget* ams_card(const char *heading) {
    GtkWidget *frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    ams_css(frame, "card");
    if (heading) {
        GtkWidget *lbl = gtk_label_new(heading);
        ams_css(lbl, "accent");
        gtk_widget_set_halign(lbl, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(frame), lbl, FALSE, FALSE, 0);
    }
    return frame;
}

/* Convenience: key-value info row */
static inline GtkWidget* ams_info_row(const char *key, const char *value) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    ams_css(row, "info-row");

    GtkWidget *k = gtk_label_new(key);
    ams_css(k, "info-key");
    gtk_widget_set_size_request(k, 140, -1);
    gtk_widget_set_halign(k, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(row), k, FALSE, FALSE, 0);

    GtkWidget *v = gtk_label_new(value);
    ams_css(v, "info-value");
    gtk_widget_set_halign(v, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(row), v, TRUE, TRUE, 0);

    return row;
}
