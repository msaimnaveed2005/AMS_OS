/*
AMS OS Desktop — Graphical Operating System Interface
GTK3 desktop with login screen, boot splash, app launcher, taskbar, and shutdown animation.
All tasks launch as native GTK3 GUI apps (not in terminals).
Dynamic app registry loaded from data/desktop_apps.txt.
*/

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <fstream>
#include <sstream>
#include <dirent.h>

/* ═══════════════════════════════════════════════════════
   Custom Cairo Icon Renderer — Professional desktop icons
   ═══════════════════════════════════════════════════════ */

#include <cairo.h>
#include <math.h>

/* Draw a rounded rectangle path */
static void cairo_rounded_rect(cairo_t *cr, double x, double y, double w, double h, double r) {
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r,     r, -M_PI/2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0,        M_PI/2);
    cairo_arc(cr, x + r,     y + h - r, r, M_PI/2,   M_PI);
    cairo_arc(cr, x + r,     y + r,     r, M_PI,      3*M_PI/2);
    cairo_close_path(cr);
}

/* Create a GdkPixbuf from a cairo surface */
static GdkPixbuf* pixbuf_from_surface(cairo_surface_t *surface) {
    return gdk_pixbuf_get_from_surface(surface, 0, 0,
        cairo_image_surface_get_width(surface),
        cairo_image_surface_get_height(surface));
}

/* ── Individual icon draw functions ── */

static GdkPixbuf* draw_icon_calculator() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Blue gradient background */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.24, 0.36, 0.96);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.38, 0.52, 1.0);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Display */
    cairo_set_source_rgba(cr, 0, 0, 0, 0.3);
    cairo_rounded_rect(cr, 8, 8, sz-16, 12, 3);
    cairo_fill(cr);
    /* "=" symbol */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 2.5);
    cairo_move_to(cr, 15, 28); cairo_line_to(cr, 33, 28); cairo_stroke(cr);
    cairo_move_to(cr, 15, 34); cairo_line_to(cr, 33, 34); cairo_stroke(cr);
    /* Small dots for buttons */
    for (int r = 0; r < 2; r++) for (int c = 0; c < 3; c++) {
        cairo_arc(cr, 14 + c*9, 24 + r*10, 1.5, 0, 2*M_PI);
        cairo_fill(cr);
    }
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_notepad() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Yellow page */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.98, 0.88, 0.4);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.95, 0.78, 0.28);
    cairo_rounded_rect(cr, 6, 4, sz-12, sz-8, 4);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Folded corner */
    cairo_set_source_rgba(cr, 0.8, 0.68, 0.2, 0.6);
    cairo_move_to(cr, sz-10, 4); cairo_line_to(cr, sz-6, 4);
    cairo_line_to(cr, sz-6, 12); cairo_close_path(cr); cairo_fill(cr);
    /* Lines */
    cairo_set_source_rgba(cr, 0.6, 0.5, 0.15, 0.5);
    cairo_set_line_width(cr, 1.2);
    for (int y = 16; y < sz-8; y += 6) {
        cairo_move_to(cr, 12, y); cairo_line_to(cr, sz-12, y); cairo_stroke(cr);
    }
    /* Pencil accent */
    cairo_set_source_rgba(cr, 0.4, 0.3, 0.1, 0.7);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, 14, 15); cairo_line_to(cr, 28, 15); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_snake() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Dark green background */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.1, 0.55, 0.2);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.05, 0.4, 0.12);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Snake body (S-curve) */
    cairo_set_source_rgb(cr, 0.3, 0.9, 0.4);
    cairo_set_line_width(cr, 5);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, 12, 14);
    cairo_curve_to(cr, 36, 14, 12, 34, 36, 34);
    cairo_stroke(cr);
    /* Eye */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_arc(cr, 14, 12, 3, 0, 2*M_PI); cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_arc(cr, 14.5, 11.5, 1.5, 0, 2*M_PI); cairo_fill(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_chess() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Checkered board background */
    int cell = 6;
    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) {
        if ((r+c)%2==0) cairo_set_source_rgb(cr, 0.94, 0.85, 0.71);
        else            cairo_set_source_rgb(cr, 0.71, 0.53, 0.39);
        cairo_rectangle(cr, r*cell, c*cell, cell, cell);
        cairo_fill(cr);
    }
    /* Dark overlay for vignette */
    cairo_rounded_rect(cr, 0, 0, sz, sz, 8);
    cairo_clip(cr);
    /* Knight symbol - white */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_font_size(cr, 28);
    cairo_move_to(cr, 12, 34);
    cairo_show_text(cr, "♞");
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_music() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Purple gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.55, 0.22, 0.92);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.72, 0.36, 1.0);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Music note */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 2.5);
    /* Note stem */
    cairo_move_to(cr, 30, 10); cairo_line_to(cr, 30, 32); cairo_stroke(cr);
    /* Note head */
    cairo_arc(cr, 26, 33, 5, 0, 2*M_PI); cairo_fill(cr);
    /* Flag */
    cairo_move_to(cr, 30, 10);
    cairo_curve_to(cr, 36, 14, 36, 20, 30, 22);
    cairo_set_line_width(cr, 2);
    cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_calendar() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* White card */
    cairo_set_source_rgb(cr, 0.95, 0.95, 0.97);
    cairo_rounded_rect(cr, 4, 6, sz-8, sz-10, 6);
    cairo_fill(cr);
    /* Red header */
    cairo_set_source_rgb(cr, 0.9, 0.2, 0.2);
    cairo_rounded_rect(cr, 4, 6, sz-8, 14, 6);
    cairo_fill(cr);
    cairo_rectangle(cr, 4, 14, sz-8, 6); cairo_fill(cr);
    /* Date number */
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_set_font_size(cr, 18);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char day[4]; snprintf(day, sizeof(day), "%d", t->tm_mday);
    cairo_move_to(cr, t->tm_mday >= 10 ? 13 : 18, 40);
    cairo_show_text(cr, day);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_copilot() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Purple-cyan gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.45, 0.15, 0.90);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.1, 0.8, 0.85);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Star/sparkle */
    cairo_set_source_rgb(cr, 1, 1, 1);
    double cx = sz/2.0, cy = sz/2.0;
    /* 4-point star */
    cairo_move_to(cr, cx, cy-12);
    cairo_line_to(cr, cx+3, cy-3);
    cairo_line_to(cr, cx+12, cy);
    cairo_line_to(cr, cx+3, cy+3);
    cairo_line_to(cr, cx, cy+12);
    cairo_line_to(cr, cx-3, cy+3);
    cairo_line_to(cr, cx-12, cy);
    cairo_line_to(cr, cx-3, cy-3);
    cairo_close_path(cr);
    cairo_fill(cr);
    /* Small sparkle dots */
    cairo_arc(cr, cx+10, cy-10, 2, 0, 2*M_PI); cairo_fill(cr);
    cairo_arc(cr, cx-8, cy+10, 1.5, 0, 2*M_PI); cairo_fill(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_terminal() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Dark background */
    cairo_set_source_rgb(cr, 0.12, 0.12, 0.15);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 8);
    cairo_fill(cr);
    /* Border */
    cairo_set_source_rgba(cr, 0.4, 0.4, 0.45, 0.5);
    cairo_set_line_width(cr, 1.5);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 8);
    cairo_stroke(cr);
    /* Green prompt > */
    cairo_set_source_rgb(cr, 0.2, 0.9, 0.3);
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, ">_");
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_folder() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Folder tab */
    cairo_set_source_rgb(cr, 0.95, 0.75, 0.2);
    cairo_move_to(cr, 6, 14);
    cairo_line_to(cr, 6, 10);
    cairo_curve_to(cr, 6, 8, 8, 8, 10, 8);
    cairo_line_to(cr, 20, 8);
    cairo_line_to(cr, 23, 14);
    cairo_close_path(cr);
    cairo_fill(cr);
    /* Folder body */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 14, 0, sz-6);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.95, 0.78, 0.25);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.85, 0.65, 0.15);
    cairo_rounded_rect(cr, 4, 14, sz-8, sz-20, 4);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_settings() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Gray gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.45, 0.45, 0.5);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.3, 0.3, 0.35);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Gear: outer ring with teeth */
    double cx = sz/2.0, cy = sz/2.0;
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.92);
    int teeth = 8;
    double outer_r = 14;
    for (int i = 0; i < teeth; i++) {
        double a = i * 2 * M_PI / teeth;
        cairo_move_to(cr, cx + outer_r * cos(a-0.15), cy + outer_r * sin(a-0.15));
        cairo_line_to(cr, cx + (outer_r+3) * cos(a), cy + (outer_r+3) * sin(a));
        cairo_line_to(cr, cx + outer_r * cos(a+0.15), cy + outer_r * sin(a+0.15));
    }
    cairo_arc(cr, cx, cy, outer_r, 0, 2*M_PI);
    cairo_fill(cr);
    /* Inner hole */
    cairo_set_source_rgb(cr, 0.35, 0.35, 0.4);
    cairo_arc(cr, cx, cy, 6, 0, 2*M_PI);
    cairo_fill(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_minesweeper() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Red gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.85, 0.2, 0.2);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.65, 0.1, 0.1);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Bomb circle */
    double cx = sz/2.0, cy = sz/2.0;
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_arc(cr, cx, cy, 10, 0, 2*M_PI); cairo_fill(cr);
    /* Spikes */
    cairo_set_line_width(cr, 2.5);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    for (int i = 0; i < 4; i++) {
        double a = i * M_PI / 4;
        cairo_move_to(cr, cx + 10 * cos(a), cy + 10 * sin(a));
        cairo_line_to(cr, cx + 15 * cos(a), cy + 15 * sin(a));
        cairo_stroke(cr);
    }
    /* Highlight */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.6);
    cairo_arc(cr, cx-3, cy-3, 3, 0, 2*M_PI); cairo_fill(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_sudoku() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Blue gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.2, 0.4, 0.9);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.15, 0.3, 0.75);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Grid */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.3);
    cairo_set_line_width(cr, 1);
    for (int i = 1; i < 3; i++) {
        cairo_move_to(cr, 8 + i*11, 8);  cairo_line_to(cr, 8 + i*11, sz-8); cairo_stroke(cr);
        cairo_move_to(cr, 8, 8 + i*11);  cairo_line_to(cr, sz-8, 8 + i*11); cairo_stroke(cr);
    }
    /* Numbers */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, 12, 22); cairo_show_text(cr, "9");
    cairo_move_to(cr, 23, 33); cairo_show_text(cr, "5");
    cairo_move_to(cr, 34, 22); cairo_show_text(cr, "1");
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_downloads() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Teal gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.1, 0.6, 0.75);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.08, 0.45, 0.6);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Down arrow */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 3);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, sz/2, 10); cairo_line_to(cr, sz/2, 30); cairo_stroke(cr);
    cairo_move_to(cr, sz/2-8, 24); cairo_line_to(cr, sz/2, 32); cairo_line_to(cr, sz/2+8, 24); cairo_stroke(cr);
    /* Tray */
    cairo_move_to(cr, 10, 36); cairo_line_to(cr, sz-10, 36); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_task_manager() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Dark indigo gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.22, 0.18, 0.55);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.15, 0.12, 0.4);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Bar chart */
    double bars[] = {20, 28, 16, 32, 22};
    double colors[][3] = {{0.4,0.8,0.95},{0.5,0.9,0.5},{0.95,0.7,0.3},{0.9,0.4,0.5},{0.7,0.5,0.9}};
    for (int i = 0; i < 5; i++) {
        cairo_set_source_rgb(cr, colors[i][0], colors[i][1], colors[i][2]);
        cairo_rounded_rect(cr, 8 + i*7, sz-6-bars[i], 5, bars[i], 2);
        cairo_fill(cr);
    }
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_system_info() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Cool gray gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.28, 0.32, 0.45);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.18, 0.2, 0.32);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* CPU chip */
    cairo_set_source_rgb(cr, 0.7, 0.75, 0.85);
    cairo_rounded_rect(cr, 12, 12, 24, 24, 4);
    cairo_fill(cr);
    /* Pins */
    cairo_set_line_width(cr, 2);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    for (int i = 0; i < 3; i++) {
        double p = 18 + i*6;
        cairo_move_to(cr, p, 12); cairo_line_to(cr, p, 6);  cairo_stroke(cr);
        cairo_move_to(cr, p, 36); cairo_line_to(cr, p, 42); cairo_stroke(cr);
        cairo_move_to(cr, 12, p); cairo_line_to(cr, 6, p);  cairo_stroke(cr);
        cairo_move_to(cr, 36, p); cairo_line_to(cr, 42, p); cairo_stroke(cr);
    }
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_clock() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Orange gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.95, 0.55, 0.15);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.85, 0.4, 0.1);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Clock face */
    double cx = sz/2.0, cy = sz/2.0;
    cairo_set_source_rgba(cr, 1, 1, 1, 0.2);
    cairo_arc(cr, cx, cy, 14, 0, 2*M_PI); cairo_fill(cr);
    /* Hands */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 2.5);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, cx, cy); cairo_line_to(cr, cx, cy-10); cairo_stroke(cr);
    cairo_move_to(cr, cx, cy); cairo_line_to(cr, cx+7, cy+3); cairo_stroke(cr);
    /* Center dot */
    cairo_arc(cr, cx, cy, 2, 0, 2*M_PI); cairo_fill(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_process_killer() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Red-orange gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.9, 0.25, 0.15);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.75, 0.15, 0.1);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Lightning bolt */
    cairo_set_source_rgb(cr, 1, 0.95, 0.3);
    cairo_move_to(cr, 26, 6);
    cairo_line_to(cr, 16, 24);
    cairo_line_to(cr, 24, 24);
    cairo_line_to(cr, 20, 42);
    cairo_line_to(cr, 34, 20);
    cairo_line_to(cr, 26, 20);
    cairo_close_path(cr);
    cairo_fill(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_flappy_bird() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Sky gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, 0, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.4, 0.75, 0.95);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.3, 0.6, 0.85);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Green pipe */
    cairo_set_source_rgb(cr, 0.3, 0.75, 0.25);
    cairo_rectangle(cr, 32, 0, 10, 18); cairo_fill(cr);
    cairo_rectangle(cr, 30, 16, 14, 6); cairo_fill(cr);
    cairo_rectangle(cr, 32, 34, 10, 14); cairo_fill(cr);
    cairo_rectangle(cr, 30, 32, 14, 6); cairo_fill(cr);
    /* Bird body */
    cairo_set_source_rgb(cr, 1, 0.85, 0.1);
    cairo_arc(cr, 18, 24, 8, 0, 2*M_PI); cairo_fill(cr);
    /* Wing */
    cairo_set_source_rgb(cr, 0.95, 0.7, 0.05);
    cairo_arc(cr, 14, 26, 5, 0, M_PI); cairo_fill(cr);
    /* Eye */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_arc(cr, 22, 22, 3, 0, 2*M_PI); cairo_fill(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_arc(cr, 23, 21.5, 1.5, 0, 2*M_PI); cairo_fill(cr);
    /* Beak */
    cairo_set_source_rgb(cr, 1, 0.4, 0.15);
    cairo_move_to(cr, 25, 24);
    cairo_line_to(cr, 30, 25);
    cairo_line_to(cr, 25, 27);
    cairo_close_path(cr); cairo_fill(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_photo_viewer() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Teal gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.1, 0.65, 0.65);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.08, 0.5, 0.52);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Photo frame */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
    cairo_rounded_rect(cr, 8, 10, 32, 26, 3);
    cairo_set_line_width(cr, 2); cairo_stroke(cr);
    /* Mountain landscape */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.4);
    cairo_move_to(cr, 10, 34);
    cairo_line_to(cr, 20, 20);
    cairo_line_to(cr, 26, 26);
    cairo_line_to(cr, 32, 16);
    cairo_line_to(cr, 38, 34);
    cairo_close_path(cr); cairo_fill(cr);
    /* Sun */
    cairo_arc(cr, 14, 16, 3, 0, 2*M_PI); cairo_fill(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_ams_studio() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Blue-indigo gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.2, 0.35, 0.9);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.35, 0.2, 0.8);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Code brackets < /> */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_font_size(cr, 18);
    cairo_move_to(cr, 7, 32);
    cairo_show_text(cr, "</>");
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_create_file() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Green gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.15, 0.7, 0.45);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.1, 0.55, 0.35);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* File page */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
    cairo_rounded_rect(cr, 13, 8, 22, 28, 3);
    cairo_set_line_width(cr, 2); cairo_stroke(cr);
    /* Plus sign */
    cairo_set_line_width(cr, 3);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, 24, 16); cairo_line_to(cr, 24, 30); cairo_stroke(cr);
    cairo_move_to(cr, 17, 23); cairo_line_to(cr, 31, 23); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_delete_file() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Red gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.85, 0.2, 0.22);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.7, 0.12, 0.15);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Trash can */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 2);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    /* Lid */
    cairo_move_to(cr, 14, 16); cairo_line_to(cr, 34, 16); cairo_stroke(cr);
    cairo_move_to(cr, 20, 16); cairo_line_to(cr, 20, 12); cairo_line_to(cr, 28, 12); cairo_line_to(cr, 28, 16); cairo_stroke(cr);
    /* Body */
    cairo_move_to(cr, 15, 16); cairo_line_to(cr, 17, 38); cairo_line_to(cr, 31, 38); cairo_line_to(cr, 33, 16); cairo_stroke(cr);
    /* Lines */
    cairo_move_to(cr, 21, 20); cairo_line_to(cr, 21, 34); cairo_stroke(cr);
    cairo_move_to(cr, 27, 20); cairo_line_to(cr, 27, 34); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_copy_file() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Blue gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.25, 0.5, 0.9);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.18, 0.38, 0.75);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Two stacked pages */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.5);
    cairo_rounded_rect(cr, 16, 6, 20, 26, 3);
    cairo_set_line_width(cr, 1.5); cairo_stroke(cr);
    cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
    cairo_rounded_rect(cr, 10, 12, 20, 26, 3);
    cairo_set_line_width(cr, 2); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_move_file() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Orange gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.95, 0.6, 0.15);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.85, 0.45, 0.08);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Page */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.8);
    cairo_rounded_rect(cr, 10, 10, 16, 22, 3);
    cairo_set_line_width(cr, 2); cairo_stroke(cr);
    /* Arrow */
    cairo_set_line_width(cr, 2.5);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, 26, 24); cairo_line_to(cr, 38, 24); cairo_stroke(cr);
    cairo_move_to(cr, 34, 19); cairo_line_to(cr, 39, 24); cairo_line_to(cr, 34, 29); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_file_info() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Cyan gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.15, 0.65, 0.85);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.1, 0.5, 0.7);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Info "i" */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_arc(cr, sz/2, 14, 3, 0, 2*M_PI); cairo_fill(cr);
    cairo_set_line_width(cr, 3.5);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, sz/2, 22); cairo_line_to(cr, sz/2, 38); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_browser() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Blue gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.2, 0.45, 0.95);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.15, 0.35, 0.8);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Globe */
    double cx = sz/2.0, cy = sz/2.0;
    cairo_set_source_rgba(cr, 1, 1, 1, 0.8);
    cairo_set_line_width(cr, 1.5);
    cairo_arc(cr, cx, cy, 13, 0, 2*M_PI); cairo_stroke(cr);
    /* Meridians */
    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_scale(cr, 0.5, 1);
    cairo_arc(cr, 0, 0, 13, 0, 2*M_PI); cairo_stroke(cr);
    cairo_restore(cr);
    /* Equator */
    cairo_move_to(cr, cx-13, cy); cairo_line_to(cr, cx+13, cy); cairo_stroke(cr);
    /* Vertical */
    cairo_move_to(cr, cx, cy-13); cairo_line_to(cr, cx, cy+13); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_tictactoe() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Pink gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.85, 0.3, 0.55);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.7, 0.2, 0.45);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Hash grid */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.7);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, 18, 8); cairo_line_to(cr, 18, 40); cairo_stroke(cr);
    cairo_move_to(cr, 30, 8); cairo_line_to(cr, 30, 40); cairo_stroke(cr);
    cairo_move_to(cr, 8, 18); cairo_line_to(cr, 40, 18); cairo_stroke(cr);
    cairo_move_to(cr, 8, 30); cairo_line_to(cr, 40, 30); cairo_stroke(cr);
    /* X */
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_set_line_width(cr, 2.5);
    cairo_move_to(cr, 10, 21); cairo_line_to(cr, 16, 27); cairo_stroke(cr);
    cairo_move_to(cr, 16, 21); cairo_line_to(cr, 10, 27); cairo_stroke(cr);
    /* O */
    cairo_arc(cr, 36, 13, 4, 0, 2*M_PI); cairo_stroke(cr);
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_generic() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Gray gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.35, 0.35, 0.42);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.25, 0.25, 0.32);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* Gear-like symbol */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.6);
    cairo_set_font_size(cr, 24);
    cairo_move_to(cr, 14, 34);
    cairo_show_text(cr, "⚡");
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

static GdkPixbuf* draw_icon_file_generic() {
    int sz = 48;
    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, sz, sz);
    cairo_t *cr = cairo_create(s);
    /* Light blue gradient */
    cairo_pattern_t *bg = cairo_pattern_create_linear(0, 0, sz, sz);
    cairo_pattern_add_color_stop_rgb(bg, 0, 0.4, 0.55, 0.8);
    cairo_pattern_add_color_stop_rgb(bg, 1, 0.3, 0.42, 0.65);
    cairo_rounded_rect(cr, 2, 2, sz-4, sz-4, 10);
    cairo_set_source(cr, bg); cairo_fill(cr);
    cairo_pattern_destroy(bg);
    /* File page with folded corner */
    cairo_set_source_rgba(cr, 1, 1, 1, 0.9);
    cairo_move_to(cr, 14, 8);
    cairo_line_to(cr, 30, 8);
    cairo_line_to(cr, 36, 14);
    cairo_line_to(cr, 36, 40);
    cairo_line_to(cr, 14, 40);
    cairo_close_path(cr);
    cairo_set_line_width(cr, 1.5); cairo_stroke(cr);
    /* Fold */
    cairo_move_to(cr, 30, 8); cairo_line_to(cr, 30, 14); cairo_line_to(cr, 36, 14);
    cairo_stroke(cr);
    /* Lines */
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgba(cr, 1, 1, 1, 0.5);
    for (int y = 20; y < 36; y += 5) {
        cairo_move_to(cr, 18, y); cairo_line_to(cr, 32, y); cairo_stroke(cr);
    }
    GdkPixbuf *pb = pixbuf_from_surface(s);
    cairo_destroy(cr); cairo_surface_destroy(s);
    return pb;
}

/* ── Master icon dispatcher ── */
static GdkPixbuf* get_custom_icon(const std::string& name) {
    if (name == "Calculator")     return draw_icon_calculator();
    if (name == "Notepad")        return draw_icon_notepad();
    if (name == "Snake Game")     return draw_icon_snake();
    if (name == "Chess")          return draw_icon_chess();
    if (name == "Music Player")   return draw_icon_music();
    if (name == "Calendar")       return draw_icon_calendar();
    if (name == "AI Copilot")     return draw_icon_copilot();
    if (name == "Terminal")       return draw_icon_terminal();
    if (name == "File Explorer")  return draw_icon_folder();
    if (name == "Settings")       return draw_icon_settings();
    if (name == "Minesweeper")    return draw_icon_minesweeper();
    if (name == "Sudoku")         return draw_icon_sudoku();
    if (name == "Downloads")      return draw_icon_downloads();
    if (name == "Task Manager")   return draw_icon_task_manager();
    if (name == "System Info")    return draw_icon_system_info();
    if (name == "Digital Clock")  return draw_icon_clock();
    if (name == "Process Killer") return draw_icon_process_killer();
    if (name == "Flappy Bird")    return draw_icon_flappy_bird();
    if (name == "Photo Viewer")   return draw_icon_photo_viewer();
    if (name == "AMS Studio")     return draw_icon_ams_studio();
    if (name == "Create File")    return draw_icon_create_file();
    if (name == "Delete File")    return draw_icon_delete_file();
    if (name == "Copy File")      return draw_icon_copy_file();
    if (name == "Move File")      return draw_icon_move_file();
    if (name == "File Info")      return draw_icon_file_info();
    if (name == "Web Browser")    return draw_icon_browser();
    if (name == "Tic Tac Toe")    return draw_icon_tictactoe();

    /* Check for file-like names (has extension) */
    if (name.find('.') != std::string::npos) return draw_icon_file_generic();

    return draw_icon_generic();
}

/* ═══════════════════════════════════════════════════════
   Task Registry — Dynamic, loaded from data/desktop_apps.txt
   ═══════════════════════════════════════════════════════ */

struct TaskEntry {
    int         id;
    std::string name;
    std::string emoji;
    std::string exec_path;
};

static std::vector<TaskEntry> TASKS;
static int TASK_COUNT = 0;

/* ── Default app definitions (written if registry file missing) ── */
static void write_default_tasks() {
    std::ofstream f("data/desktop_apps.txt");
    if (!f.is_open()) return;
    f << "1|Create File|📄|./build/gui_create_file\n";
    f << "2|Delete File|🗑️|./build/gui_delete_file\n";
    f << "3|Copy File|📋|./build/gui_file_copy\n";
    f << "4|Move File|📦|./build/gui_move_file\n";
    f << "5|File Info|ℹ️|./build/gui_file_info\n";
    f << "6|Notepad|📝|./build/gui_notepad\n";
    f << "7|Calculator|🧮|./build/gui_calculator\n";
    f << "8|Digital Clock|🕐|./build/gui_clock\n";
    f << "9|System Info|💻|./build/gui_system_info\n";
    f << "10|Snake Game|🐍|./build/gui_snake\n";
    f << "11|Minesweeper|💣|./build/gui_minesweeper\n";
    f << "12|Music Player|🎵|./build/gui_music_player\n";
    f << "13|Downloads|⬇️|./build/gui_download_simulator\n";
    f << "14|Task Manager|📊|./build/gui_task_manager\n";
    f << "15|Process Killer|⚡|./build/gui_process_killer\n";
    f << "16|Calendar|📅|./build/gui_calendar\n";
    f << "17|AI Copilot|✦|./build/gui_ai_copilot\n";
    f << "18|Sudoku|🔢|./build/gui_sudoku\n";
    f << "19|Chess|♟️|./build/gui_chess\n";
    f << "20|Terminal|💻|./build/gui_terminal\n";
    f << "21|File Explorer|📁|./build/gui_file_explorer\n";
    f << "22|Settings|⚙️|./build/gui_settings\n";
    f << "23|AMS Studio|💻|./build/gui_ams_studio\n";
    f << "24|Photo Viewer|🖼️|./build/gui_photo_viewer\n";
    f << "25|Flappy Bird|🐦|./build/gui_flappy_bird\n";
    f << "26|Tic Tac Toe|🎮|./build/gui_tic_tac_toe\n";
}

static void load_tasks() {
    TASKS.clear();

    /* Ensure data directory exists */
    int r1 = system("mkdir -p data 2>/dev/null");
    (void)r1;

    std::ifstream f("data/desktop_apps.txt");
    if (!f.is_open()) {
        write_default_tasks();
        f.open("data/desktop_apps.txt");
        if (!f.is_open()) return;
    }

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        /* Remove trailing \r if present (Windows line endings) */
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        /* Format: id|name|emoji|exec_path */
        size_t p1 = line.find('|');
        if (p1 == std::string::npos) continue;
        size_t p2 = line.find('|', p1 + 1);
        if (p2 == std::string::npos) continue;
        size_t p3 = line.find('|', p2 + 1);
        if (p3 == std::string::npos) continue;

        TaskEntry t;
        try { t.id = std::stoi(line.substr(0, p1)); } catch (...) { continue; }
        t.name      = line.substr(p1 + 1, p2 - p1 - 1);
        t.emoji     = line.substr(p2 + 1, p3 - p2 - 1);
        t.exec_path = line.substr(p3 + 1);

        TASKS.push_back(t);
    }
    
    /* Ensure desktop directory exists and load files from it */
    int r2 = system("mkdir -p data/desktop 2>/dev/null");
    (void)r2;
    DIR *dir = opendir("data/desktop");
    if (dir) {
        struct dirent *ent;
        int virtual_id = 1000;
        while ((ent = readdir(dir))) {
            if (ent->d_name[0] == '.') continue;
            std::string name = ent->d_name;
            std::string path = std::string("data/desktop/") + name;
            
            TaskEntry t;
            t.id = virtual_id++;
            t.name = name;
            t.emoji = ""; // Unused when using GTK icons
            
            if (ent->d_type == DT_DIR) {
                t.exec_path = std::string("xdg-open ") + path; // Simple fallback
            } else {
                t.exec_path = std::string("./build/gui_notepad ") + path;
            }
            TASKS.push_back(t);
        }
        closedir(dir);
    }
    
    TASK_COUNT = (int)TASKS.size();
}

/* ═══════════════════════════════════════════════════════
   Application State
   ═══════════════════════════════════════════════════════ */

struct AppState {
    GtkApplication *app;
    GtkWidget      *clock_label;
    GtkWidget      *progress_bar;
    GtkWidget      *splash_window;
    GtkWidget      *search_entry;
    GtkWidget      *flow_box;
    GtkWidget      *login_error;
    GtkWidget      *desktop_win;
    GtkWidget      *desktop_content;
    int             splash_step;
    int             shutdown_step;
    int             argc;
    char          **argv;
    GtkWidget      *cpu_label;
    GtkWidget      *ram_label;
};

static AppState S = {};

/* Forward declarations */
static void show_login();
static void show_splash();
static void show_desktop();
static void show_mode_selector();
static void populate_grid();
static void refresh_desktop();

/* Real-time Refresh via SIGUSR1 */
static volatile sig_atomic_t g_needs_refresh = 0;
static void handle_sigusr1(int) { g_needs_refresh = 1; }

static gboolean check_refresh_flag(gpointer) {
    if (g_needs_refresh) {
        g_needs_refresh = 0;
        refresh_desktop();
    }
    return G_SOURCE_CONTINUE;
}

/* ═══════════════════════════════════════════════════════
   CSS Theme — Premium dark desktop aesthetic
   ═══════════════════════════════════════════════════════ */

static const char *APP_CSS = R"CSS(

/* ── Login Screen ── */
window.login { background-color: #06060f; }
.login-avatar {
    font-size: 72px;
    margin-top: 20px;
}
.login-title {
    font-size: 22px;
    font-weight: 700;
    color: white;
    margin-top: 8px;
}
.login-entry {
    background-color: rgba(255,255,255,0.08);
    border: 1px solid rgba(255,255,255,0.1);
    border-radius: 10px;
    color: white;
    padding: 10px 16px;
    font-size: 14px;
    min-width: 260px;
    caret-color: white;
}
.login-entry:focus {
    border-color: rgba(139,92,246,0.6);
    background-color: rgba(255,255,255,0.12);
}
.login-btn {
    background-color: rgba(99,102,241,0.5);
    border: 1px solid rgba(99,102,241,0.6);
    border-radius: 10px;
    color: white;
    padding: 10px 40px;
    font-size: 14px;
    font-weight: 600;
    min-width: 260px;
    transition: all 150ms ease-in-out;
}
.login-btn:hover {
    background-color: rgba(99,102,241,0.7);
}
.login-hint {
    font-size: 11px;
    color: rgba(255,255,255,0.25);
    margin-top: 16px;
}
.login-error {
    font-size: 12px;
    color: #f87171;
    margin-top: 4px;
}

/* ── Mode Selector ── */
window.mode-select { background-color: #08081a; }
.mode-title {
    font-size: 48px; font-weight: 800; color: #a78bfa; margin-top: 36px;
}
.mode-subtitle {
    font-size: 13px; color: rgba(255,255,255,0.4); margin-bottom: 32px;
}
.mode-card {
    background-color: rgba(255,255,255,0.05);
    border: 1px solid rgba(255,255,255,0.08);
    border-radius: 22px; padding: 36px 52px;
    margin: 10px; min-width: 180px; min-height: 120px;
    transition: all 200ms ease-in-out;
}
.mode-card:hover {
    background-color: rgba(139,92,246,0.18);
    border-color: rgba(139,92,246,0.45);
    box-shadow: 0 8px 40px rgba(139,92,246,0.25);
}
.mode-emoji { font-size: 52px; }
.mode-label { font-size: 18px; font-weight: 700; color: white; margin-top: 10px; }
.mode-desc  { font-size: 11px; color: rgba(255,255,255,0.45); margin-top: 2px; }

/* ── Splash Screen ── */
window.splash {
    background-color: #06060f;
    animation: fade-in 1s ease-out;
}
.splash-logo {
    margin-top: 50px;
    animation: pulse-logo 2.5s infinite ease-in-out;
}
@keyframes pulse-logo {
    0% { opacity: 0.7; }
    50% { opacity: 1.0; }
    100% { opacity: 0.7; }
}
@keyframes fade-in {
    from { opacity: 0; }
    to { opacity: 1; }
}
.splash-name { font-size: 18px; font-weight: 700; color: rgba(255,255,255,0.75); letter-spacing: 8px; margin-top: 10px; }
.splash-ver  { font-size: 11px; color: rgba(255,255,255,0.25); margin-top: 20px; }
progressbar.splash-bar trough   { background-color: rgba(255,255,255,0.06); border-radius: 3px; min-height: 4px; }
progressbar.splash-bar progress { background-image: linear-gradient(to right, #6366f1, #a855f7, #ec4899); border-radius: 3px; min-height: 4px; }

/* ── Shutdown Screen ── */
window.shutdown {
    background-color: #06060f;
    animation: fade-in 0.8s ease-out;
}
.shutdown-icon { font-size: 56px; margin-bottom: 16px; opacity: 0.6; }
.shutdown-text { font-size: 16px; color: rgba(255,255,255,0.6); font-weight: 500; }
progressbar.shutdown-bar trough   { background-color: rgba(255,255,255,0.06); border-radius: 3px; min-height: 3px; }
progressbar.shutdown-bar progress { background-image: linear-gradient(to right, #6366f1, #a855f7); border-radius: 3px; min-height: 3px; }

/* ── Desktop Wallpaper — Ultra-dark cosmic gradient ── */
window.desktop {
    background-image: linear-gradient(
        145deg,
        #030014 0%,
        #0a0520 15%,
        #10082e 30%,
        #0d0628 45%,
        #120a35 55%,
        #1a0e3a 65%,
        #140830 80%,
        #0a0418 100%
    );
}

/* ── Top Bar — Frosted glass panel ── */
.top-bar {
    background-color: rgba(6, 3, 18, 0.82);
    border-bottom: 1px solid rgba(139, 92, 246, 0.12);
    padding: 4px 20px;
    min-height: 30px;
    box-shadow: 0 2px 20px rgba(0, 0, 0, 0.5);
}
.top-logo {
    color: rgba(255, 255, 255, 0.95);
    font-size: 13px;
    font-weight: 800;
    letter-spacing: 1px;
}
.top-clock {
    color: rgba(200, 180, 255, 0.9);
    font-size: 13px;
    font-weight: 600;
    letter-spacing: 0.5px;
}
.top-user {
    color: rgba(167, 139, 250, 0.7);
    font-size: 12px;
    font-weight: 500;
}

/* ── Search — Glassmorphic pill ── */
.search-box {
    background-color: rgba(255, 255, 255, 0.06);
    border: 1px solid rgba(139, 92, 246, 0.15);
    border-radius: 22px;
    color: white;
    padding: 10px 24px;
    font-size: 14px;
    min-width: 340px;
    margin-top: 28px;
    margin-bottom: 24px;
    caret-color: #a78bfa;
    box-shadow: 0 4px 24px rgba(0, 0, 0, 0.3),
                inset 0 1px 0 rgba(255, 255, 255, 0.04);
    transition: all 200ms ease-in-out;
}
.search-box:focus {
    background-color: rgba(255, 255, 255, 0.1);
    border-color: rgba(139, 92, 246, 0.45);
    box-shadow: 0 4px 30px rgba(139, 92, 246, 0.15),
                0 0 0 3px rgba(139, 92, 246, 0.08),
                inset 0 1px 0 rgba(255, 255, 255, 0.06);
}

/* ── App Icons — Glassmorphic hover with glow ── */
.app-btn {
    background-color: transparent;
    border: none;
    background-image: none;
    border-radius: 20px;
    padding: 8px;
    min-width: 108px;
    min-height: 120px;
    box-shadow: none;
}
.app-icon-wrapper {
    background-color: rgba(255, 255, 255, 0.03);
    border: 1px solid rgba(255, 255, 255, 0.08);
    border-radius: 20px;
    padding: 18px;
    min-width: 52px;
    min-height: 52px;
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.2),
                inset 0 1px 0 rgba(255, 255, 255, 0.04);
    transition: all 250ms cubic-bezier(0.16, 1, 0.3, 1);
}
.app-btn:hover .app-icon-wrapper {
    background-color: rgba(139, 92, 246, 0.15);
    border-color: rgba(139, 92, 246, 0.45);
    box-shadow: 0 12px 32px rgba(139, 92, 246, 0.3),
                inset 0 1px 0 rgba(255, 255, 255, 0.08);
}
.app-btn:active .app-icon-wrapper {
    background-color: rgba(139, 92, 246, 0.28);
    border-color: rgba(139, 92, 246, 0.6);
    box-shadow: 0 6px 16px rgba(139, 92, 246, 0.2);
}
.app-icon-emoji {
    font-size: 38px;
}
.app-icon-name {
    color: rgba(255, 255, 255, 0.9);
    font-size: 11px;
    font-weight: 600;
    margin-top: 8px;
    letter-spacing: 0.3px;
    text-shadow: 0 2px 8px rgba(0, 0, 0, 0.8);
}
.app-btn:hover .app-icon-name {
    color: #c084fc;
}

/* ── Bottom Dock — Frosted glass bar ── */
.dock-outer {
    margin: 0px 0px 14px 0px;
}
.dock {
    background-color: rgba(8, 4, 22, 0.7);
    border: 1px solid rgba(139, 92, 246, 0.15);
    border-radius: 24px;
    padding: 8px 18px;
    box-shadow: 0 12px 40px rgba(0, 0, 0, 0.6),
                inset 0 1px 0 rgba(255, 255, 255, 0.05);
}
.dock-btn {
    background-color: transparent;
    border: none;
    background-image: none;
    border-radius: 16px;
    padding: 2px;
    min-width: 50px;
    min-height: 50px;
    box-shadow: none;
}
.dock-icon-wrapper {
    background-color: rgba(255, 255, 255, 0.04);
    border: 1px solid rgba(255, 255, 255, 0.08);
    border-radius: 14px;
    padding: 8px;
    min-width: 32px;
    min-height: 32px;
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2),
                inset 0 1px 0 rgba(255, 255, 255, 0.05);
    transition: all 250ms cubic-bezier(0.16, 1, 0.3, 1);
}
.dock-btn:hover .dock-icon-wrapper {
    background-color: rgba(139, 92, 246, 0.2);
    border-color: rgba(139, 92, 246, 0.45);
    box-shadow: 0 8px 24px rgba(139, 92, 246, 0.35),
                inset 0 1px 0 rgba(255, 255, 255, 0.1);
}
.dock-btn:active .dock-icon-wrapper {
    background-color: rgba(139, 92, 246, 0.3);
    border-color: rgba(139, 92, 246, 0.6);
}
.dock-emoji {
    font-size: 24px;
}

/* ── Power / Refresh — Pill buttons ── */
.power-btn {
    background-color: rgba(255, 60, 60, 0.08);
    border: 1px solid rgba(255, 60, 60, 0.12);
    padding: 3px 12px;
    border-radius: 10px;
    color: rgba(255, 120, 120, 0.8);
    font-size: 13px;
    transition: all 180ms ease-in-out;
}
.power-btn:hover {
    background-color: rgba(255, 60, 60, 0.25);
    border-color: rgba(255, 60, 60, 0.4);
    color: #ff9999;
    box-shadow: 0 2px 12px rgba(255, 60, 60, 0.2);
}
.refresh-btn {
    background-color: rgba(52, 211, 153, 0.06);
    border: 1px solid rgba(52, 211, 153, 0.1);
    padding: 3px 12px;
    border-radius: 10px;
    color: rgba(52, 211, 153, 0.7);
    font-size: 13px;
    transition: all 180ms ease-in-out;
}
.refresh-btn:hover {
    background-color: rgba(52, 211, 153, 0.2);
    border-color: rgba(52, 211, 153, 0.35);
    color: #6ee7b7;
    box-shadow: 0 2px 12px rgba(52, 211, 153, 0.15);
}

/* ── Shutdown / Power Menu Cards ── */
.shutdown-card {
    background-color: rgba(255, 255, 255, 0.03);
    border: 1px solid rgba(139, 92, 246, 0.1);
    border-radius: 18px;
    padding: 22px 36px;
    margin: 8px;
    min-width: 110px;
    transition: all 200ms ease-in-out;
    box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
}
.shutdown-card:hover {
    background-color: rgba(139, 92, 246, 0.12);
    border-color: rgba(139, 92, 246, 0.4);
    box-shadow: 0 8px 36px rgba(139, 92, 246, 0.2);
}
.shutdown-card-emoji { font-size: 34px; }
.shutdown-card-label {
    font-size: 12px;
    color: rgba(255, 255, 255, 0.65);
    font-weight: 500;
    margin-top: 8px;
    letter-spacing: 0.3px;
}

)CSS";

/* ═══════════════════════════════════════════════════════
   Utilities
   ═══════════════════════════════════════════════════════ */

static std::string get_clock_text() {
    time_t now = time(NULL); struct tm *t = localtime(&now);
    char buf[64]; strftime(buf, sizeof(buf), "%a %b %d   %I:%M %p", t);
    return buf;
}

/* Launch a GUI task directly (no terminal wrapper needed) */
static void launch_gui_task(const char *path) {
    pid_t pid = fork();
    if (pid == 0) { setsid(); execlp(path, path, (char *)NULL); _exit(1); }
}

static GtkCssProvider *global_provider = NULL;

static void apply_css() {
    if (!global_provider) {
        global_provider = gtk_css_provider_new();
        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(global_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    std::ifstream in("data/theme.css");
    if (in.is_open()) {
        std::string css_str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        gtk_css_provider_load_from_data(global_provider, css_str.c_str(), -1, NULL);
        in.close();
    } else {
        std::string dir_cmd = "mkdir -p data";
        int ret = system(dir_cmd.c_str());
        (void)ret;

        std::ofstream out("data/theme.css");
        out << APP_CSS;
        out.close();
        gtk_css_provider_load_from_data(global_provider, APP_CSS, -1, NULL);
    }
}

static void add_class(GtkWidget *w, const char *cls) {
    gtk_style_context_add_class(gtk_widget_get_style_context(w), cls);
}

/* ═══════════════════════════════════════════════════════
   Clock tick
   ═══════════════════════════════════════════════════════ */

static gboolean tick_clock(gpointer) {
    if (S.clock_label && GTK_IS_LABEL(S.clock_label))
        gtk_label_set_text(GTK_LABEL(S.clock_label), get_clock_text().c_str());
    return G_SOURCE_CONTINUE;
}

/* ═══════════════════════════════════════════════════════
   Real-Time Hardware Telemetry
   ═══════════════════════════════════════════════════════ */

static unsigned long long prev_total = 0, prev_idle = 0;

static double get_cpu_usage() {
    std::ifstream stat_file("/proc/stat");
    std::string line;
    if (std::getline(stat_file, line)) {
        if (line.compare(0, 3, "cpu") == 0) {
            std::istringstream iss(line);
            std::string cpu;
            unsigned long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
            if (iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice) {
                unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
                unsigned long long idle_all = idle + iowait;
                double usage = 0.0;
                if (prev_total != 0) {
                    unsigned long long total_d = total - prev_total;
                    unsigned long long idle_d = idle_all - prev_idle;
                    if (total_d > 0) usage = (1.0 - ((double)idle_d / total_d)) * 100.0;
                }
                prev_total = total;
                prev_idle = idle_all;
                return usage;
            }
        }
    }
    return 0.0;
}

static double get_ram_usage() {
    std::ifstream mem_file("/proc/meminfo");
    std::string line;
    double total = 0, avail = 0;
    while (std::getline(mem_file, line)) {
        if (line.compare(0, 8, "MemTotal") == 0) {
            sscanf(line.c_str(), "MemTotal: %lf kB", &total);
        } else if (line.compare(0, 12, "MemAvailable") == 0) {
            sscanf(line.c_str(), "MemAvailable: %lf kB", &avail);
        }
    }
    if (total > 0) return ((total - avail) / total) * 100.0;
    return 0.0;
}

static gboolean tick_telemetry(gpointer) {
    if (S.cpu_label && GTK_IS_LABEL(S.cpu_label)) {
        double cpu = get_cpu_usage();
        char buf[32]; snprintf(buf, sizeof(buf), "⚙ CPU: %.1f%%  ", cpu);
        gtk_label_set_text(GTK_LABEL(S.cpu_label), buf);
    }
    if (S.ram_label && GTK_IS_LABEL(S.ram_label)) {
        double ram = get_ram_usage();
        char buf[32]; snprintf(buf, sizeof(buf), "💾 RAM: %.1f%%  ", ram);
        gtk_label_set_text(GTK_LABEL(S.ram_label), buf);
    }
    return G_SOURCE_CONTINUE;
}

/* ═══════════════════════════════════════════════════════
   Search filter
   ═══════════════════════════════════════════════════════ */

static gboolean filter_func(GtkFlowBoxChild *child, gpointer) {
    if (!S.search_entry) return TRUE;
    const char *query = gtk_entry_get_text(GTK_ENTRY(S.search_entry));
    if (!query || strlen(query) == 0) return TRUE;
    GtkWidget *btn = gtk_bin_get_child(GTK_BIN(child));
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "task_idx"));
    if (idx < 0 || idx >= TASK_COUNT) return TRUE;
    std::string n = TASKS[idx].name;
    std::string q(query);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    return n.find(q) != std::string::npos ? TRUE : FALSE;
}

static void on_search_changed(GtkEntry *, gpointer) {
    if (S.flow_box) gtk_flow_box_invalidate_filter(GTK_FLOW_BOX(S.flow_box));
}

/* ═══════════════════════════════════════════════════════
   Task click — direct launch (no terminal)
   ═══════════════════════════════════════════════════════ */

static void on_app_click(GtkWidget *btn, gpointer) {
    int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "task_idx"));
    if (idx >= 0 && idx < TASK_COUNT)
        launch_gui_task(TASKS[idx].exec_path.c_str());
}

/* ═══════════════════════════════════════════════════════
   Refresh Desktop — reload registry and rebuild grid
   ═══════════════════════════════════════════════════════ */

static void refresh_desktop() {
    if (!S.flow_box) return;

    /* Reload tasks from file */
    load_tasks();

    /* Hot-reload CSS Theme */
    apply_css();

    /* Remove all children from the flow box */
    GList *children = gtk_container_get_children(GTK_CONTAINER(S.flow_box));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);

    /* Re-populate the grid */
    populate_grid();

    /* Re-apply the filter */
    gtk_flow_box_invalidate_filter(GTK_FLOW_BOX(S.flow_box));
}

static void on_refresh_clicked(GtkWidget *, gpointer) {
    refresh_desktop();
}

static gboolean on_desktop_focus(GtkWidget *, GdkEventFocus *, gpointer) {
    refresh_desktop();
    return FALSE;
}

/* ═══════════════════════════════════════════════════════
   Populate Icon Grid
   ═══════════════════════════════════════════════════════ */

static void populate_grid() {
    if (!S.flow_box) return;
    for (int i = 0; i < TASK_COUNT; i++) {
        GtkWidget *btn = gtk_button_new();
        add_class(btn, "app-btn");
        g_object_set_data(G_OBJECT(btn), "task_idx", GINT_TO_POINTER(i));

        GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_widget_set_halign(inner, GTK_ALIGN_CENTER);

        GtkWidget *emoji_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        add_class(emoji_wrapper, "app-icon-wrapper");
        gtk_widget_set_halign(emoji_wrapper, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(emoji_wrapper, GTK_ALIGN_CENTER);

        GdkPixbuf *icon_pb;
        if (TASKS[i].exec_path.find("xdg-open") != std::string::npos) {
            icon_pb = draw_icon_folder();
        } else {
            icon_pb = get_custom_icon(TASKS[i].name);
        }
        GtkWidget *emoji = gtk_image_new_from_pixbuf(icon_pb);
        g_object_unref(icon_pb);
        add_class(emoji, "app-icon-emoji");
        gtk_box_pack_start(GTK_BOX(emoji_wrapper), emoji, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(inner), emoji_wrapper, FALSE, FALSE, 0);

        GtkWidget *name = gtk_label_new(TASKS[i].name.c_str());
        add_class(name, "app-icon-name");
        gtk_label_set_max_width_chars(GTK_LABEL(name), 14);
        gtk_label_set_ellipsize(GTK_LABEL(name), PANGO_ELLIPSIZE_END);
        gtk_box_pack_start(GTK_BOX(inner), name, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(btn), inner);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_app_click), NULL);
        gtk_container_add(GTK_CONTAINER(S.flow_box), btn);
    }
    gtk_widget_show_all(S.flow_box);
}

/* ═══════════════════════════════════════════════════════
   Shutdown Screen
   ═══════════════════════════════════════════════════════ */

static GtkWidget *shutdown_win = NULL;
static GtkWidget *shutdown_bar = NULL;

static gboolean shutdown_tick(gpointer) {
    S.shutdown_step++;
    if (S.shutdown_step >= 25) {
        g_application_quit(G_APPLICATION(S.app));
        return G_SOURCE_REMOVE;
    }
    if (shutdown_bar)
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(shutdown_bar), (double)S.shutdown_step / 25.0);
    return G_SOURCE_CONTINUE;
}

static void do_shutdown(GtkWidget *desktop) {
    gtk_widget_destroy(desktop);
    S.desktop_win = NULL;
    S.shutdown_step = 0;

    shutdown_win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(shutdown_win), "");
    gtk_window_set_default_size(GTK_WINDOW(shutdown_win), 500, 350);
    gtk_window_set_position(GTK_WINDOW(shutdown_win), GTK_WIN_POS_CENTER);
    gtk_window_set_decorated(GTK_WINDOW(shutdown_win), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(shutdown_win), FALSE);
    add_class(shutdown_win, "shutdown");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(shutdown_win), box);

    GtkWidget *icon = gtk_label_new("⏻");
    add_class(icon, "shutdown-icon");
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);

    GtkWidget *txt = gtk_label_new("Shutting down...");
    add_class(txt, "shutdown-text");
    gtk_box_pack_start(GTK_BOX(box), txt, FALSE, FALSE, 0);

    GtkWidget *spinner = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(spinner));
    gtk_box_pack_start(GTK_BOX(box), spinner, FALSE, FALSE, 12);

    shutdown_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(shutdown_bar, 220, -1);
    add_class(shutdown_bar, "shutdown-bar");
    gtk_box_pack_start(GTK_BOX(box), shutdown_bar, FALSE, FALSE, 0);

    gtk_widget_show_all(shutdown_win);
    g_timeout_add(100, shutdown_tick, NULL);
}

/* ═══════════════════════════════════════════════════════
   Power Menu (Shutdown / Log Out / Cancel)
   ═══════════════════════════════════════════════════════ */

static void on_power_clicked(GtkWidget *, gpointer desktop_win) {
    GtkWidget *dlg = gtk_dialog_new_with_buttons("", GTK_WINDOW(desktop_win),
        (GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
        NULL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dlg), 380, 200);
    gtk_window_set_decorated(GTK_WINDOW(dlg), FALSE);
    add_class(dlg, "mode-select");

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dlg));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(content), vbox);

    GtkWidget *title = gtk_label_new("What would you like to do?");
    add_class(title, "mode-label");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 8);

    GtkWidget *cards = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_halign(cards, GTK_ALIGN_CENTER);

    /* Shutdown card */
    GtkWidget *sd_btn = gtk_button_new();
    add_class(sd_btn, "shutdown-card");
    GtkWidget *sd_inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_halign(sd_inner, GTK_ALIGN_CENTER);
    GtkWidget *sd_e = gtk_label_new("⏻"); add_class(sd_e, "shutdown-card-emoji");
    GtkWidget *sd_l = gtk_label_new("Shut Down"); add_class(sd_l, "shutdown-card-label");
    gtk_box_pack_start(GTK_BOX(sd_inner), sd_e, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sd_inner), sd_l, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(sd_btn), sd_inner);

    /* Log Out card */
    GtkWidget *lo_btn = gtk_button_new();
    add_class(lo_btn, "shutdown-card");
    GtkWidget *lo_inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_halign(lo_inner, GTK_ALIGN_CENTER);
    GtkWidget *lo_e = gtk_label_new("🚪"); add_class(lo_e, "shutdown-card-emoji");
    GtkWidget *lo_l = gtk_label_new("Log Out"); add_class(lo_l, "shutdown-card-label");
    gtk_box_pack_start(GTK_BOX(lo_inner), lo_e, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(lo_inner), lo_l, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(lo_btn), lo_inner);

    /* Cancel card */
    GtkWidget *cc_btn = gtk_button_new();
    add_class(cc_btn, "shutdown-card");
    GtkWidget *cc_inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_halign(cc_inner, GTK_ALIGN_CENTER);
    GtkWidget *cc_e = gtk_label_new("✕"); add_class(cc_e, "shutdown-card-emoji");
    GtkWidget *cc_l = gtk_label_new("Cancel"); add_class(cc_l, "shutdown-card-label");
    gtk_box_pack_start(GTK_BOX(cc_inner), cc_e, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cc_inner), cc_l, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(cc_btn), cc_inner);

    gtk_box_pack_start(GTK_BOX(cards), sd_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cards), lo_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cards), cc_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), cards, FALSE, FALSE, 0);

    gtk_widget_show_all(dlg);

    g_signal_connect_swapped(cc_btn, "clicked", G_CALLBACK(gtk_widget_destroy), dlg);
    g_signal_connect(sd_btn, "clicked", G_CALLBACK(+[](GtkWidget *, gpointer data) {
        GtkWidget *dlg = GTK_WIDGET(data);
        GtkWidget *desktop = gtk_widget_get_toplevel(
            GTK_WIDGET(gtk_window_get_transient_for(GTK_WINDOW(dlg))));
        gtk_widget_destroy(dlg);
        do_shutdown(desktop);
    }), dlg);
    g_signal_connect(lo_btn, "clicked", G_CALLBACK(+[](GtkWidget *, gpointer data) {
        GtkWidget *dlg = GTK_WIDGET(data);
        GtkWidget *desktop = gtk_widget_get_toplevel(
            GTK_WIDGET(gtk_window_get_transient_for(GTK_WINDOW(dlg))));
        gtk_widget_destroy(dlg);
        gtk_widget_destroy(desktop);
        S.desktop_win = NULL;
        show_login();
    }), dlg);
}

/* ═══════════════════════════════════════════════════════
   Desktop Window
   ═══════════════════════════════════════════════════════ */

static void on_lock_clicked(GtkWidget *, gpointer win) {
    gtk_widget_hide(GTK_WIDGET(win));
    show_login();
}

static void on_wallpaper_change(GtkWidget*, gpointer) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Choose Wallpaper",
        GTK_WINDOW(S.desktop_win), GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL, "Apply", GTK_RESPONSE_ACCEPT, NULL);
    
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Images");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        /* Rewrite CSS file to include wallpaper */
        std::ifstream in("data/theme.css");
        std::string css_str;
        if (in.is_open()) {
            css_str.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            in.close();
            
            size_t pos = css_str.find("window.desktop { background-image:");
            if (pos != std::string::npos) {
                size_t end = css_str.find("}", pos);
                if (end != std::string::npos) css_str.erase(pos, end - pos + 1);
            }
            
            std::string fpath(filename);
            for(char& c : fpath) { if(c == '\\') c = '/'; }
            
            css_str += "\nwindow.desktop { background-image: url('file:///" + fpath + "'); background-size: cover; background-position: center; }\n";
            
            std::ofstream out("data/theme.css");
            out << css_str;
            out.close();
            apply_css(); /* Hot reload */
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void on_terminal_open(GtkWidget*, gpointer) {
    if (fork() == 0) { setsid(); execlp("./build/gui_terminal", "./build/gui_terminal", (char*)NULL); _exit(1); }
}

static gboolean on_desktop_click(GtkWidget* /*widget*/, GdkEventButton *event, gpointer) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        GtkWidget *menu = gtk_menu_new();
        
        GtkWidget *item1 = gtk_menu_item_new_with_label("🖼️ Change Wallpaper...");
        g_signal_connect(item1, "activate", G_CALLBACK(on_wallpaper_change), NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item1);
        
        GtkWidget *item2 = gtk_menu_item_new_with_label("💻 Open Terminal");
        g_signal_connect(item2, "activate", G_CALLBACK(on_terminal_open), NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item2);
        
        GtkWidget *item3 = gtk_menu_item_new_with_label("🔄 Refresh Desktop");
        g_signal_connect(item3, "activate", G_CALLBACK(on_refresh_clicked), NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item3);
        
        gtk_widget_show_all(menu);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)event);
        return TRUE;
    }
    return FALSE;
}

static void show_desktop() {
    /* Reload tasks fresh */
    load_tasks();

    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(win), "AMS OS");
    gtk_window_set_default_size(GTK_WINDOW(win), 1280, 800);
    gtk_window_maximize(GTK_WINDOW(win));
    add_class(win, "desktop");
    S.desktop_win = win;

    gtk_widget_add_events(win, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(win, "button-press-event", G_CALLBACK(on_desktop_click), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* ── Top Bar ── */
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    add_class(bar, "top-bar");

    GtkWidget *logo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(logo_box, 16);
    
    GdkPixbuf *pixbuf_tb = gdk_pixbuf_new_from_file_at_scale("data/logo.png", 24, 24, TRUE, NULL);
    GtkWidget *logo_img = pixbuf_tb ? gtk_image_new_from_pixbuf(pixbuf_tb) : gtk_image_new_from_icon_name("start-here", GTK_ICON_SIZE_LARGE_TOOLBAR);
    if (pixbuf_tb) g_object_unref(pixbuf_tb);
    
    GtkWidget *logo_lbl = gtk_label_new("AMS OS");
    add_class(logo_lbl, "top-logo");
    gtk_box_pack_start(GTK_BOX(logo_box), logo_img, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(logo_box), logo_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(bar), logo_box, FALSE, FALSE, 0);

    /* Power button */
    GtkWidget *pwr = gtk_button_new_with_label("⏻  ");
    add_class(pwr, "power-btn");
    g_signal_connect(pwr, "clicked", G_CALLBACK(on_power_clicked), win);
    gtk_box_pack_end(GTK_BOX(bar), pwr, FALSE, FALSE, 0);

    /* Lock button */
    GtkWidget *lck = gtk_button_new_with_label("🔒 ");
    add_class(lck, "refresh-btn"); /* Reuse refresh styling for the lock button */
    gtk_widget_set_tooltip_text(lck, "Lock OS");
    g_signal_connect(lck, "clicked", G_CALLBACK(on_lock_clicked), win);
    gtk_box_pack_end(GTK_BOX(bar), lck, FALSE, FALSE, 6);

    /* Refresh button */
    GtkWidget *ref = gtk_button_new_with_label("🔄 ");
    add_class(ref, "refresh-btn");
    gtk_widget_set_tooltip_text(ref, "Refresh desktop apps");
    g_signal_connect(ref, "clicked", G_CALLBACK(on_refresh_clicked), NULL);
    gtk_box_pack_end(GTK_BOX(bar), ref, FALSE, FALSE, 0);

    /* Clock */
    S.clock_label = gtk_label_new(get_clock_text().c_str());
    add_class(S.clock_label, "top-clock");
    gtk_box_pack_end(GTK_BOX(bar), S.clock_label, FALSE, FALSE, 10);

    /* Telemetry */
    S.ram_label = gtk_label_new("💾 RAM: --.-%  ");
    add_class(S.ram_label, "top-clock");
    gtk_box_pack_end(GTK_BOX(bar), S.ram_label, FALSE, FALSE, 0);

    S.cpu_label = gtk_label_new("⚙ CPU: --.-%  ");
    add_class(S.cpu_label, "top-clock");
    gtk_box_pack_end(GTK_BOX(bar), S.cpu_label, FALSE, FALSE, 0);

    /* User label */
    GtkWidget *user = gtk_label_new("👤 admin  ");
    add_class(user, "top-user");
    gtk_box_pack_end(GTK_BOX(bar), user, FALSE, FALSE, 4);

    gtk_box_pack_start(GTK_BOX(vbox), bar, FALSE, FALSE, 0);

    /* ── Scrollable content ── */
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(content, GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(scroll), content);
    S.desktop_content = content;

    /* ── Search ── */
    GtkWidget *search_wrap = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(search_wrap, GTK_ALIGN_CENTER);
    S.search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(S.search_entry), "🔍  Search applications...");
    add_class(S.search_entry, "search-box");
    gtk_box_pack_start(GTK_BOX(search_wrap), S.search_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), search_wrap, FALSE, FALSE, 0);

    /* ── Icon Grid ── */
    S.flow_box = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(S.flow_box), 7);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(S.flow_box), 4);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(S.flow_box), 10);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(S.flow_box), 10);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(S.flow_box), GTK_SELECTION_NONE);
    gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(S.flow_box), TRUE);
    gtk_widget_set_margin_start(S.flow_box, 40);
    gtk_widget_set_margin_end(S.flow_box, 40);

    gtk_flow_box_set_filter_func(GTK_FLOW_BOX(S.flow_box), filter_func, NULL, NULL);
    g_signal_connect(S.search_entry, "changed", G_CALLBACK(on_search_changed), NULL);

    populate_grid();
    gtk_box_pack_start(GTK_BOX(content), S.flow_box, FALSE, FALSE, 0);

    /* ── Bottom Dock ── */
    GtkWidget *dock_outer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_halign(dock_outer, GTK_ALIGN_CENTER);
    add_class(dock_outer, "dock-outer");

    GtkWidget *dock = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    add_class(dock, "dock");

    int pinned[] = {17, 7, 6, 8, 16, 10, 18, 19, 25, 11, 12, 9};
    for (int p : pinned) {
        for (int i = 0; i < TASK_COUNT; i++) {
            if (TASKS[i].id != p) continue;
            GtkWidget *dbtn = gtk_button_new();
            add_class(dbtn, "dock-btn");
            gtk_widget_set_tooltip_text(dbtn, TASKS[i].name.c_str());
            g_object_set_data(G_OBJECT(dbtn), "task_idx", GINT_TO_POINTER(i));

            GtkWidget *de_wrapper = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            add_class(de_wrapper, "dock-icon-wrapper");
            gtk_widget_set_halign(de_wrapper, GTK_ALIGN_CENTER);
            gtk_widget_set_valign(de_wrapper, GTK_ALIGN_CENTER);

            GdkPixbuf *raw_pb = get_custom_icon(TASKS[i].name);
            GdkPixbuf *scaled_pb = gdk_pixbuf_scale_simple(raw_pb, 32, 32, GDK_INTERP_BILINEAR);
            g_object_unref(raw_pb);
            GtkWidget *de = gtk_image_new_from_pixbuf(scaled_pb);
            g_object_unref(scaled_pb);
            add_class(de, "dock-emoji");
            gtk_box_pack_start(GTK_BOX(de_wrapper), de, TRUE, TRUE, 0);

            gtk_container_add(GTK_CONTAINER(dbtn), de_wrapper);
            g_signal_connect(dbtn, "clicked", G_CALLBACK(on_app_click), NULL);
            gtk_box_pack_start(GTK_BOX(dock), dbtn, FALSE, FALSE, 2);
            break;
        }
    }

    gtk_box_pack_start(GTK_BOX(dock_outer), dock, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox), dock_outer, FALSE, FALSE, 0);

    /* Auto-refresh when window gains focus */
    g_signal_connect(win, "focus-in-event", G_CALLBACK(on_desktop_focus), NULL);

    g_timeout_add_seconds(1, tick_clock, NULL);
    g_timeout_add_seconds(1, tick_telemetry, NULL); /* Ultra-smooth 1s updates */
    gtk_widget_show_all(win);
}

/* ═══════════════════════════════════════════════════════
   Splash Screen
   ═══════════════════════════════════════════════════════ */

static gboolean splash_tick(gpointer) {
    S.splash_step++;
    if (S.splash_step >= 30) {
        if (S.splash_window) { gtk_widget_destroy(S.splash_window); S.splash_window = NULL; }
        show_desktop();
        return G_SOURCE_REMOVE;
    }
    if (S.progress_bar)
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(S.progress_bar), (double)S.splash_step / 30.0);
    return G_SOURCE_CONTINUE;
}

static void show_splash() {
    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_default_size(GTK_WINDOW(win), 520, 380);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    add_class(win, "splash");
    S.splash_window = win; S.splash_step = 0;

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(win), box);

    GdkPixbuf *pixbuf_sp = gdk_pixbuf_new_from_file_at_scale("data/logo.png", 128, 128, TRUE, NULL);
    GtkWidget *icon = pixbuf_sp ? gtk_image_new_from_pixbuf(pixbuf_sp) : gtk_label_new("⚛️");
    if (pixbuf_sp) g_object_unref(pixbuf_sp);
    
    add_class(icon, "splash-logo");
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    GtkWidget *name = gtk_label_new("AMS  OS"); add_class(name, "splash-name");
    gtk_box_pack_start(GTK_BOX(box), name, FALSE, FALSE, 0);
    GtkWidget *ver = gtk_label_new("Atomic Management System  ·  v3.0"); add_class(ver, "splash-ver");
    gtk_box_pack_start(GTK_BOX(box), ver, FALSE, FALSE, 0);

    S.progress_bar = gtk_progress_bar_new();
    gtk_widget_set_size_request(S.progress_bar, 240, -1);
    add_class(S.progress_bar, "splash-bar");
    gtk_widget_set_margin_top(S.progress_bar, 30);
    gtk_box_pack_start(GTK_BOX(box), S.progress_bar, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
    g_timeout_add(100, splash_tick, NULL);
}

/* ═══════════════════════════════════════════════════════
   Login Screen
   ═══════════════════════════════════════════════════════ */

static GtkWidget *login_user_entry = NULL, *login_pass_entry = NULL;

static void on_login_clicked(GtkWidget *, gpointer win) {
    const char *user = gtk_entry_get_text(GTK_ENTRY(login_user_entry));
    const char *pass = gtk_entry_get_text(GTK_ENTRY(login_pass_entry));

    if ((strcmp(user, "admin") == 0 && strcmp(pass, "admin") == 0) ||
        (strcmp(user, "saim") == 0 && strcmp(pass, "1234") == 0) ||
        (strcmp(user, "mohsin") == 0 && strcmp(pass, "1234") == 0) ||
        (strcmp(user, "ahmed") == 0 && strcmp(pass, "1234") == 0)) {
        gtk_widget_destroy(GTK_WIDGET(win));
        if (S.desktop_win) {
            gtk_widget_show_all(S.desktop_win);
        } else {
            show_splash();
        }
    } else {
        gtk_label_set_text(GTK_LABEL(S.login_error), "❌ Incorrect username or password");
    }
}

static gboolean on_login_key(GtkWidget *, GdkEventKey *ev, gpointer win) {
    if (ev->keyval == GDK_KEY_Return) { on_login_clicked(NULL, win); return TRUE; }
    return FALSE;
}

static void show_login() {
    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(win), "AMS OS — Login");
    gtk_window_set_default_size(GTK_WINDOW(win), 440, 460);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    add_class(win, "login");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(win), box);

    GtkWidget *avatar = gtk_label_new("👤"); add_class(avatar, "login-avatar");
    gtk_box_pack_start(GTK_BOX(box), avatar, FALSE, FALSE, 0);

    GtkWidget *title = gtk_label_new("AMS OS"); add_class(title, "login-title");
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 0);

    GtkWidget *sub = gtk_label_new("Sign in to continue"); add_class(sub, "mode-desc");
    gtk_box_pack_start(GTK_BOX(box), sub, FALSE, FALSE, 8);

    login_user_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(login_user_entry), "Username");
    gtk_entry_set_text(GTK_ENTRY(login_user_entry), "admin");
    add_class(login_user_entry, "login-entry");
    gtk_box_pack_start(GTK_BOX(box), login_user_entry, FALSE, FALSE, 4);

    login_pass_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(login_pass_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(login_pass_entry), FALSE);
    add_class(login_pass_entry, "login-entry");
    gtk_box_pack_start(GTK_BOX(box), login_pass_entry, FALSE, FALSE, 4);

    S.login_error = gtk_label_new(""); add_class(S.login_error, "login-error");
    gtk_box_pack_start(GTK_BOX(box), S.login_error, FALSE, FALSE, 0);

    GtkWidget *login_btn = gtk_button_new_with_label("Sign In");
    add_class(login_btn, "login-btn");
    g_signal_connect(login_btn, "clicked", G_CALLBACK(on_login_clicked), win);
    gtk_box_pack_start(GTK_BOX(box), login_btn, FALSE, FALSE, 8);

    GtkWidget *hint = gtk_label_new("Credentials: admin/admin  ·  saim/1234  ·  mohsin/1234  ·  ahmed/1234");
    add_class(hint, "login-hint");
    gtk_box_pack_start(GTK_BOX(box), hint, FALSE, FALSE, 0);

    g_signal_connect(win, "key-press-event", G_CALLBACK(on_login_key), win);
    gtk_widget_show_all(win);
}

/* ═══════════════════════════════════════════════════════
   Mode Selector
   ═══════════════════════════════════════════════════════ */

static void on_gui_clicked(GtkWidget *, gpointer w) {
    gtk_widget_destroy(GTK_WIDGET(w));
    show_login();
}

static void on_cmd_clicked(GtkWidget *, gpointer w) {
    std::string cmd = "./OS";
    for (int i = 1; i < S.argc; i++) { cmd += " "; cmd += S.argv[i]; }
    gtk_widget_destroy(GTK_WIDGET(w));
    pid_t pid = fork();
    if (pid == 0) {
        execlp("gnome-terminal", "gnome-terminal", "--title", "AMS OS — Terminal Mode",
               "--geometry", "120x40", "--", "bash", "-c", cmd.c_str(), NULL);
        _exit(1);
    }
    g_application_quit(G_APPLICATION(S.app));
}

static GtkWidget *make_card(const char *emoji, const char *label, const char *desc) {
    GtkWidget *btn = gtk_button_new(); add_class(btn, "mode-card");
    GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_halign(inner, GTK_ALIGN_CENTER);
    GtkWidget *e = gtk_label_new(emoji); add_class(e, "mode-emoji");
    GtkWidget *l = gtk_label_new(label); add_class(l, "mode-label");
    GtkWidget *d = gtk_label_new(desc);  add_class(d, "mode-desc");
    gtk_box_pack_start(GTK_BOX(inner), e, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(inner), l, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(inner), d, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(btn), inner);
    return btn;
}

static void show_mode_selector() {
    GtkWidget *win = gtk_application_window_new(S.app);
    gtk_window_set_title(GTK_WINDOW(win), "AMS OS — Select Mode");
    gtk_window_set_default_size(GTK_WINDOW(win), 560, 380);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);
    add_class(win, "mode-select");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    GtkWidget *title = gtk_label_new("AMS OS"); add_class(title, "mode-title");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    GtkWidget *sub = gtk_label_new("Choose your interface"); add_class(sub, "mode-subtitle");
    gtk_box_pack_start(GTK_BOX(vbox), sub, FALSE, FALSE, 0);

    GtkWidget *cards = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_halign(cards, GTK_ALIGN_CENTER);
    GtkWidget *gui_card = make_card("🖥️", "GUI Mode", "Visual desktop interface");
    GtkWidget *cmd_card = make_card("⌨️",  "CMD Mode", "Classic terminal interface");
    g_signal_connect(gui_card, "clicked", G_CALLBACK(on_gui_clicked), win);
    g_signal_connect(cmd_card, "clicked", G_CALLBACK(on_cmd_clicked), win);
    gtk_box_pack_start(GTK_BOX(cards), gui_card, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(cards), cmd_card, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), cards, FALSE, FALSE, 0);

    gtk_widget_show_all(win);
}

/* ═══════════════════════════════════════════════════════
   Application Entry
   ═══════════════════════════════════════════════════════ */

static void on_activate(GtkApplication *, gpointer) {
    load_tasks();
    apply_css();
    show_mode_selector();
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    signal(SIGUSR1, handle_sigusr1); /* Listen for Copilot app-creation triggers */
    g_timeout_add(500, check_refresh_flag, NULL);

    S.argc = argc; S.argv = argv;
    S.app = gtk_application_new("com.ams.os.desktop", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(S.app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(S.app), 0, NULL);
    g_object_unref(S.app);
    return status;
}
