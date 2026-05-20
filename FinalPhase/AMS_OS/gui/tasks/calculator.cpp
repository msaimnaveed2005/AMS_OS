/* AMS OS — GUI Calculator (iOS-style dark calculator) */
#include "../gui_theme.h"

struct CalcState {
    GtkWidget *result_label;
    GtkWidget *expr_label;
    double current;
    double operand;
    char   op;
    bool   new_input;
    std::string display;
};
static CalcState C = {NULL, NULL, 0, 0, 0, true, "0"};

static void update_display() {
    gtk_label_set_text(GTK_LABEL(C.result_label), C.display.c_str());
}

static double compute(double a, char op, double b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0 ? a / b : 0;
        default:  return b;
    }
}

static void on_digit(GtkWidget *, gpointer data) {
    const char *d = (const char *)data;
    if (C.new_input) { C.display = ""; C.new_input = false; }
    if (C.display == "0" && *d != '.') C.display = "";
    if (*d == '.' && C.display.find('.') != std::string::npos) return;
    C.display += d;
    update_display();
}

static void on_operator(GtkWidget *, gpointer data) {
    char new_op = *(const char *)data;
    double val = std::stod(C.display.empty() ? "0" : C.display);
    if (C.op && !C.new_input) {
        C.operand = compute(C.operand, C.op, val);
    } else {
        C.operand = val;
    }
    C.op = new_op;
    C.new_input = true;
    /* Show intermediate result */
    char buf[64]; snprintf(buf, sizeof(buf), "%g", C.operand);
    std::string expr = std::string(buf) + " " + new_op;
    gtk_label_set_text(GTK_LABEL(C.expr_label), expr.c_str());
}

static void on_equals(GtkWidget *, gpointer) {
    double val = std::stod(C.display.empty() ? "0" : C.display);
    if (C.op) {
        double result = compute(C.operand, C.op, val);
        char buf[64]; snprintf(buf, sizeof(buf), "%g", result);
        C.display = buf;
        C.operand = result;
        C.op = 0;
    }
    C.new_input = true;
    gtk_label_set_text(GTK_LABEL(C.expr_label), "");
    update_display();
}

static void on_clear(GtkWidget *, gpointer) {
    C.display = "0"; C.operand = 0; C.op = 0; C.new_input = true;
    gtk_label_set_text(GTK_LABEL(C.expr_label), "");
    update_display();
}

static void on_negate(GtkWidget *, gpointer) {
    if (C.display != "0" && !C.display.empty()) {
        if (C.display[0] == '-') C.display.erase(0, 1);
        else C.display = "-" + C.display;
        update_display();
    }
}

static void on_percent(GtkWidget *, gpointer) {
    double val = std::stod(C.display.empty() ? "0" : C.display);
    char buf[64]; snprintf(buf, sizeof(buf), "%g", val / 100.0);
    C.display = buf;
    update_display();
}

static GtkWidget* calc_btn(const char *label, const char *css_class,
                            GCallback cb, gpointer data) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    ams_css(btn, css_class);
    g_signal_connect(btn, "clicked", cb, data);
    return btn;
}

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();
    GtkWidget *win = ams_window(app, "Calculator", "accessories-calculator", 320, 520);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_widget_set_margin_start(vbox, 8);
    gtk_widget_set_margin_end(vbox, 8);
    gtk_widget_set_margin_bottom(vbox, 8);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* Display */
    GtkWidget *disp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    ams_css(disp, "calc-display");

    C.expr_label = gtk_label_new("");
    ams_css(C.expr_label, "calc-expr");
    gtk_widget_set_halign(C.expr_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(disp), C.expr_label, FALSE, FALSE, 0);

    C.result_label = gtk_label_new("0");
    ams_css(C.result_label, "calc-result");
    gtk_widget_set_halign(C.result_label, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(disp), C.result_label, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), disp, FALSE, FALSE, 0);

    /* Button grid */
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    /* Static strings for callback data */
    static const char digits[][2] = {"0","1","2","3","4","5","6","7","8","9","."};
    static const char ops[][2] = {"+","-","*","/"};

    /* Row 0: AC  ±  %  ÷ */
    gtk_grid_attach(GTK_GRID(grid), calc_btn("AC","calc-func", G_CALLBACK(on_clear), NULL), 0,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("±","calc-func", G_CALLBACK(on_negate), NULL), 1,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("%","calc-func", G_CALLBACK(on_percent), NULL), 2,0,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("÷","calc-op", G_CALLBACK(on_operator), (gpointer)ops[3]), 3,0,1,1);

    /* Row 1: 7 8 9 × */
    gtk_grid_attach(GTK_GRID(grid), calc_btn("7","calc-num", G_CALLBACK(on_digit), (gpointer)digits[7]), 0,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("8","calc-num", G_CALLBACK(on_digit), (gpointer)digits[8]), 1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("9","calc-num", G_CALLBACK(on_digit), (gpointer)digits[9]), 2,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("×","calc-op", G_CALLBACK(on_operator), (gpointer)ops[2]), 3,1,1,1);

    /* Row 2: 4 5 6 − */
    gtk_grid_attach(GTK_GRID(grid), calc_btn("4","calc-num", G_CALLBACK(on_digit), (gpointer)digits[4]), 0,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("5","calc-num", G_CALLBACK(on_digit), (gpointer)digits[5]), 1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("6","calc-num", G_CALLBACK(on_digit), (gpointer)digits[6]), 2,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("−","calc-op", G_CALLBACK(on_operator), (gpointer)ops[1]), 3,2,1,1);

    /* Row 3: 1 2 3 + */
    gtk_grid_attach(GTK_GRID(grid), calc_btn("1","calc-num", G_CALLBACK(on_digit), (gpointer)digits[1]), 0,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("2","calc-num", G_CALLBACK(on_digit), (gpointer)digits[2]), 1,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("3","calc-num", G_CALLBACK(on_digit), (gpointer)digits[3]), 2,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("+","calc-op", G_CALLBACK(on_operator), (gpointer)ops[0]), 3,3,1,1);

    /* Row 4: 0(wide) . = */
    gtk_grid_attach(GTK_GRID(grid), calc_btn("0","calc-num", G_CALLBACK(on_digit), (gpointer)digits[0]), 0,4,2,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn(".","calc-num", G_CALLBACK(on_digit), (gpointer)digits[10]), 2,4,1,1);
    gtk_grid_attach(GTK_GRID(grid), calc_btn("=","calc-op", G_CALLBACK(on_equals), NULL), 3,4,1,1);

    gtk_widget_show_all(win);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.calculator", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
