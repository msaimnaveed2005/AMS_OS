/*
AMS OS — AI Copilot
Intelligent AI assistant powered by Google Gemini API (online)
or a built-in rule engine (offline).  Can launch apps via natural language.
*/

#include "../gui_theme.h"
#include <signal.h>
#include <map>

/* ══════════════════════════════════════════════════════════════
   Constants & Configuration
   ══════════════════════════════════════════════════════════════ */

static const char *SYSTEM_PROMPT =
    "You are AMS Copilot, the built-in AI assistant for AMS Operating System — "
    "a university project OS simulator built with C++ and GTK3. "
    "You help users navigate the OS, answer questions about operating systems, "
    "and can launch applications.\n\n"
    "IMPORTANT RULES:\n"
    "1. When the user asks you to open, launch, or start an application, "
    "include the tag <<LAUNCH:app_name>> at the END of your response. "
    "Use ONLY these exact app names: calculator, notepad, snake, minesweeper, "
    "clock, calendar, music_player, system_info, task_manager, "
    "download_simulator, create_file, delete_file, file_copy, move_file, "
    "file_info, process_killer, sudoku, chess, ai_copilot\n"
    "2. Keep responses concise (2-4 sentences max).\n"
    "3. Be friendly and helpful. Use emojis occasionally.\n"
    "4. If asked about yourself, say you are AMS Copilot, built into AMS OS.\n"
    "5. If asked about OS concepts (scheduling, deadlock, memory, etc.), "
    "give clear, educational explanations.\n";

static const std::map<std::string, std::string> APP_MAP = {
    {"calculator",          "./build/gui_calculator"},
    {"notepad",             "./build/gui_notepad"},
    {"snake",               "./build/gui_snake"},
    {"minesweeper",         "./build/gui_minesweeper"},
    {"clock",               "./build/gui_clock"},
    {"calendar",            "./build/gui_calendar"},
    {"music_player",        "./build/gui_music_player"},
    {"system_info",         "./build/gui_system_info"},
    {"task_manager",        "./build/gui_task_manager"},
    {"download_simulator",  "./build/gui_download_simulator"},
    {"create_file",         "./build/gui_create_file"},
    {"delete_file",         "./build/gui_delete_file"},
    {"file_copy",           "./build/gui_file_copy"},
    {"move_file",           "./build/gui_move_file"},
    {"file_info",           "./build/gui_file_info"},
    {"process_killer",      "./build/gui_process_killer"},
    {"sudoku",              "./build/gui_sudoku"},
    {"chess",               "./build/gui_chess"},
};

/* ══════════════════════════════════════════════════════════════
   Application State
   ══════════════════════════════════════════════════════════════ */

struct ChatMsg {
    std::string role;   /* "user" or "model" */
    std::string text;
};

static std::vector<ChatMsg> history;
static GtkWidget *chat_box      = NULL;   /* VBox inside scroll */
static GtkWidget *chat_scroll   = NULL;
static GtkWidget *input_entry   = NULL;
static GtkWidget *typing_box    = NULL;

enum Provider { PROVIDER_NONE, PROVIDER_GEMINI, PROVIDER_DEEPSEEK };
static Provider current_provider = PROVIDER_NONE;
static std::string api_key;
static bool online_mode = false;
static bool busy        = false;

/* ══════════════════════════════════════════════════════════════
   JSON Helpers
   ══════════════════════════════════════════════════════════════ */

static std::string json_escape(const std::string &s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

static std::string extract_text_field(const std::string &json, const std::string &field_name) {
    size_t pos = json.find(field_name);
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos + field_name.size());
    if (pos == std::string::npos) return "";
    pos++; /* skip opening quote */

    std::string result;
    while (pos < json.size()) {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            switch (json[pos + 1]) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                default:   result += json[pos + 1]; break;
            }
            pos += 2;
        } else if (json[pos] == '"') {
            break;
        } else {
            result += json[pos];
            pos++;
        }
    }
    return result;
}

/* ══════════════════════════════════════════════════════════════
   Offline Rule Engine
   ══════════════════════════════════════════════════════════ */

static std::string to_lower(const std::string &s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

static std::string offline_respond(const std::string &input) {
    std::string q = to_lower(input);

    /* ── App launch ── */
    struct AppAlias { const char *pattern; const char *app; const char *name; };
    static const AppAlias aliases[] = {
        {"calculator",  "calculator",         "Calculator"},
        {"calc",        "calculator",         "Calculator"},
        {"notepad",     "notepad",            "Notepad"},
        {"editor",      "notepad",            "Notepad"},
        {"snake",       "snake",              "Snake Game"},
        {"minesweeper", "minesweeper",        "Minesweeper"},
        {"mines",       "minesweeper",        "Minesweeper"},
        {"clock",       "clock",              "Digital Clock"},
        {"time",        "clock",              "Digital Clock"},
        {"calendar",    "calendar",           "Calendar"},
        {"date",        "calendar",           "Calendar"},
        {"music",       "music_player",       "Music Player"},
        {"song",        "music_player",       "Music Player"},
        {"system info", "system_info",        "System Info"},
        {"sysinfo",     "system_info",        "System Info"},
        {"task manager","task_manager",       "Task Manager"},
        {"processes",   "task_manager",       "Task Manager"},
        {"download",    "download_simulator", "Downloads"},
        {"create file", "create_file",        "Create File"},
        {"delete file", "delete_file",        "Delete File"},
        {"copy file",   "file_copy",          "File Copy"},
        {"move file",   "move_file",          "Move File"},
        {"file info",   "file_info",          "File Info"},
        {"process kill","process_killer",     "Process Killer"},
        {"kill process","process_killer",     "Process Killer"},
        {"sudoku",      "sudoku",             "Sudoku"},
        {"chess",       "chess",              "Chess"},
    };

    if (q.find("open") != std::string::npos || q.find("launch") != std::string::npos ||
        q.find("start") != std::string::npos || q.find("run") != std::string::npos) {
        for (auto &a : aliases) {
            if (q.find(a.pattern) != std::string::npos) {
                return std::string("Sure! Launching ") + a.name + " for you! 🚀 <<LAUNCH:" + a.app + ">>";
            }
        }
    }
    /* Also handle "play chess/snake/sudoku" */
    if (q.find("play") != std::string::npos) {
        for (auto &a : aliases) {
            if (q.find(a.pattern) != std::string::npos) {
                return std::string("Let's play! Opening ") + a.name + " 🎮 <<LAUNCH:" + a.app + ">>";
            }
        }
    }

    /* ── Identity ── */
    if (q.find("who are you") != std::string::npos || q.find("your name") != std::string::npos)
        return "I'm ✦ AMS Copilot, the built-in AI assistant for AMS Operating System! "
               "I can launch apps, answer OS questions, and chat with you. 😊";

    if (q.find("what can you do") != std::string::npos || q.find("help") != std::string::npos)
        return "I can help you with:\n"
               "• Launch any app (try \"open calculator\")\n"
               "• Answer OS concepts (scheduling, deadlock, memory)\n"
               "• General questions and chat\n"
               "• File operations and system info";

    /* ── OS Concepts ── */
    if (q.find("deadlock") != std::string::npos)
        return "🔒 A deadlock occurs when two or more processes are blocked forever, "
               "each waiting for a resource held by the other. The four necessary conditions "
               "are: Mutual Exclusion, Hold & Wait, No Preemption, and Circular Wait. "
               "Breaking any one condition prevents deadlock.";

    if (q.find("scheduling") != std::string::npos || q.find("scheduler") != std::string::npos)
        return "📋 CPU scheduling determines which process runs on the CPU. Common algorithms:\n"
               "• FCFS — First Come First Serve (simple, non-preemptive)\n"
               "• SJF — Shortest Job First (optimal avg waiting time)\n"
               "• Round Robin — Time quantum based (fair, preemptive)\n"
               "• Priority — Based on priority values";

    if (q.find("semaphore") != std::string::npos || q.find("mutex") != std::string::npos)
        return "🔐 Semaphores and mutexes are synchronization primitives:\n"
               "• Mutex — Binary lock (0 or 1), only one thread enters critical section\n"
               "• Semaphore — Counting variable, can allow N threads simultaneously\n"
               "• Both use wait() and signal() (P and V) operations";

    if (q.find("virtual memory") != std::string::npos || q.find("paging") != std::string::npos)
        return "💾 Virtual memory separates logical from physical addresses:\n"
               "• Paging divides memory into fixed-size frames\n"
               "• Page table maps virtual → physical addresses\n"
               "• Page faults occur when a page isn't in RAM\n"
               "• Replacement algorithms: FIFO, LRU, Optimal";

    if (q.find("process") != std::string::npos && q.find("thread") != std::string::npos)
        return "🧵 Processes vs Threads:\n"
               "• Process — Independent, own memory space, heavy context switch\n"
               "• Thread — Shares process memory, lightweight, faster switching\n"
               "• Threads within a process share heap but have separate stacks";

    if (q.find("memory management") != std::string::npos || q.find("memory alloc") != std::string::npos)
        return "🧠 Memory management strategies:\n"
               "• Contiguous: First Fit, Best Fit, Worst Fit\n"
               "• Non-contiguous: Paging, Segmentation\n"
               "• Fragmentation: Internal (wasted inside block) vs External (scattered holes)";

    /* ── Greetings ── */
    if (q.find("hello") != std::string::npos || q.find("hi") != std::string::npos ||
        q.find("hey") != std::string::npos)
        return "Hello! 👋 I'm AMS Copilot. How can I help you today? "
               "You can ask me to open apps, explain OS concepts, or just chat!";

    if (q.find("thank") != std::string::npos)
        return "You're welcome! 😊 Let me know if you need anything else.";

    if (q.find("bye") != std::string::npos || q.find("goodbye") != std::string::npos)
        return "Goodbye! 👋 Have a great day! I'll be here whenever you need me.";

    /* ── Default ── */
    return "I'm currently running in offline mode 🔴 so I can help with:\n"
           "• Launching apps (e.g., \"open calculator\")\n"
           "• OS concepts (e.g., \"explain deadlock\")\n"
           "Set the GEMINI_API_KEY environment variable to enable full AI mode! 🚀";
}

/* ══════════════════════════════════════════════════════════════
   Action Parser — extract and execute <<LAUNCH:xxx>> tags
   ══════════════════════════════════════════════════════════════ */

static std::string parse_and_execute_actions(const std::string &text) {
    std::string display = text;
    size_t pos = 0;

    while ((pos = display.find("<<LAUNCH:", pos)) != std::string::npos) {
        size_t end = display.find(">>", pos);
        if (end == std::string::npos) break;

        std::string app = display.substr(pos + 9, end - pos - 9);
        display.erase(pos, end - pos + 2);  /* remove the tag */

        auto it = APP_MAP.find(app);
        if (it != APP_MAP.end()) {
            pid_t pid = fork();
            if (pid == 0) {
                setsid();
                execlp(it->second.c_str(), it->second.c_str(), (char *)NULL);
                _exit(1);
            }
        }
    }

    /* Trim trailing whitespace */
    while (!display.empty() && (display.back() == ' ' || display.back() == '\n'))
        display.pop_back();

    return display;
}

/* ══════════════════════════════════════════════════════════════
   Gemini API Call (runs in background thread)
   ══════════════════════════════════════════════════════════════ */

struct ApiRequest {
    std::string response;
};

static std::string build_gemini_json() {
    std::string json = "{\"contents\":[";

    /* Include last 10 exchanges from history */
    int start = (int)history.size() - 20;
    if (start < 0) start = 0;

    for (int i = start; i < (int)history.size(); i++) {
        if (i > start) json += ",";
        json += "{\"role\":\"" + history[i].role + "\",\"parts\":[{\"text\":\"";
        json += json_escape(history[i].text);
        json += "\"}]}";
    }

    json += "],\"systemInstruction\":{\"parts\":[{\"text\":\"";
    json += json_escape(SYSTEM_PROMPT);
    json += "\"}]},\"generationConfig\":{\"maxOutputTokens\":512,\"temperature\":0.7}}";

    return json;
}

static std::string build_deepseek_json() {
    std::string json = "{\"model\":\"deepseek-chat\",\"messages\":[";
    json += "{\"role\":\"system\",\"content\":\"" + json_escape(SYSTEM_PROMPT) + "\"}";

    int start = (int)history.size() - 20;
    if (start < 0) start = 0;

    for (int i = start; i < (int)history.size(); i++) {
        json += ",";
        std::string role = (history[i].role == "model") ? "assistant" : "user";
        json += "{\"role\":\"" + role + "\",\"content\":\"" + json_escape(history[i].text) + "\"}";
    }

    json += "],\"stream\":false,\"max_tokens\":512,\"temperature\":0.7}";

    return json;
}

/* (Gemini API call logic is in online_thread_func below) */

/* ══════════════════════════════════════════════════════════════
   UI Helpers
   ══════════════════════════════════════════════════════════════ */

static void scroll_to_bottom() {
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(chat_scroll));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj));
}

static gboolean scroll_to_bottom_idle(gpointer) {
    scroll_to_bottom();
    return G_SOURCE_REMOVE;
}

static void add_bubble(const std::string &sender, const std::string &text, bool is_user) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_margin_start(row, is_user ? 80 : 8);
    gtk_widget_set_margin_end(row, is_user ? 8 : 80);
    gtk_widget_set_margin_top(row, 4);
    gtk_widget_set_margin_bottom(row, 4);

    /* Sender label */
    GtkWidget *sender_lbl = gtk_label_new(sender.c_str());
    ams_css(sender_lbl, is_user ? "chat-sender-user" : "chat-sender-ai");
    gtk_widget_set_halign(sender_lbl, is_user ? GTK_ALIGN_END : GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(row), sender_lbl, FALSE, FALSE, 0);

    /* Message bubble */
    GtkWidget *bubble = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    ams_css(bubble, is_user ? "chat-bubble-user" : "chat-bubble-ai");

    GtkWidget *msg_lbl = gtk_label_new(text.c_str());
    ams_css(msg_lbl, "chat-text");
    gtk_label_set_line_wrap(GTK_LABEL(msg_lbl), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(msg_lbl), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_xalign(GTK_LABEL(msg_lbl), 0);
    gtk_label_set_selectable(GTK_LABEL(msg_lbl), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(msg_lbl), 50);
    gtk_box_pack_start(GTK_BOX(bubble), msg_lbl, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(row), bubble, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(chat_box), row, FALSE, FALSE, 0);
    gtk_widget_show_all(row);

    /* Scroll to bottom after layout settles */
    g_idle_add(scroll_to_bottom_idle, NULL);
}

static void show_typing(bool show) {
    if (typing_box) {
        if (show)
            gtk_widget_show_all(typing_box);
        else
            gtk_widget_hide(typing_box);
    }
}

/* ══════════════════════════════════════════════════════════════
   API Response Handler (called on main thread via g_idle_add)
   ══════════════════════════════════════════════════════════════ */

static gboolean on_api_response(gpointer data) {
    ApiRequest *req = (ApiRequest *)data;

    show_typing(false);
    busy = false;

    /* Parse actions and display */
    std::string display = parse_and_execute_actions(req->response);
    add_bubble("✦ AMS Copilot", display, false);

    /* Add to history */
    history.push_back({"model", req->response});

    delete req;
    return G_SOURCE_REMOVE;
}

/* ══════════════════════════════════════════════════════════════
   Send Message — properly threaded for online mode
   ══════════════════════════════════════════════════════════════ */
static gpointer online_thread_func(gpointer data) {
    ApiRequest *req = (ApiRequest *)data;

    /* Write JSON body to temp file */
    std::string json_body = (current_provider == PROVIDER_DEEPSEEK) ? build_deepseek_json() : build_gemini_json();
    std::string tmp_path = "/tmp/ams_copilot_req.json";
    {
        std::ofstream tmp(tmp_path);
        tmp << json_body;
    }

    std::string cmd;
    if (current_provider == PROVIDER_DEEPSEEK) {
        cmd = "curl -s -m 30 -X POST \"https://api.deepseek.com/chat/completions\" "
              "-H \"Content-Type: application/json\" "
              "-H \"Authorization: Bearer " + api_key + "\" "
              "-d @" + tmp_path;
    } else {
        cmd = "curl -s -m 30 -X POST \"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" +
              api_key + "\" "
              "-H \"Content-Type: application/json\" "
              "-d @" + tmp_path;
    }

    fprintf(stderr, "[AMS Copilot] Sending API request...\n");

    FILE *fp = popen(cmd.c_str(), "r");
    if (fp) {
        char buf[4096];
        std::string raw;
        while (fgets(buf, sizeof(buf), fp))
            raw += buf;
        int status = pclose(fp);

        fprintf(stderr, "[AMS Copilot] curl exit code: %d\n", WEXITSTATUS(status));
        fprintf(stderr, "[AMS Copilot] Raw response (first 500 chars): %.500s\n", raw.c_str());

        if (raw.empty()) {
            req->response = "⚠️ No response from API. Check your internet connection.";
        } else if (raw.find("\"error\"") != std::string::npos) {
            /* Try to extract the error message */
            std::string err_msg;
            size_t msg_pos = raw.find("\"message\"");
            if (msg_pos != std::string::npos) {
                size_t q1 = raw.find('"', msg_pos + 10);
                size_t q2 = raw.find('"', q1 + 1);
                if (q1 != std::string::npos && q2 != std::string::npos)
                    err_msg = raw.substr(q1 + 1, q2 - q1 - 1);
            }
            if (err_msg.empty()) {
                if (current_provider == PROVIDER_DEEPSEEK)
                    req->response = "⚠️ API Error. Please check your DEEPSEEK_API_KEY.";
                else
                    req->response = "⚠️ API Error. Please check your GEMINI_API_KEY.";
            } else {
                req->response = "⚠️ API Error: " + err_msg;
            }
        } else {
            if (current_provider == PROVIDER_DEEPSEEK) {
                req->response = extract_text_field(raw, "\"content\"");
            } else {
                req->response = extract_text_field(raw, "\"text\"");
            }
            if (req->response.empty())
                req->response = "🤔 I received an empty response. Please try again.";
        }
    } else {
        req->response = "⚠️ Could not run curl. Make sure it's installed.";
    }

    std::remove(tmp_path.c_str());
    g_idle_add(on_api_response, req);
    return NULL;
}

static void send_message() {
    if (busy) return;
    const char *raw = gtk_entry_get_text(GTK_ENTRY(input_entry));
    if (!raw || strlen(raw) == 0) return;

    std::string user_text(raw);
    gtk_entry_set_text(GTK_ENTRY(input_entry), "");

    add_bubble("You", user_text, true);
    history.push_back({"user", user_text});

    if (online_mode) {
        busy = true;
        show_typing(true);
        ApiRequest *req = new ApiRequest();
        g_thread_unref(g_thread_new("gemini", online_thread_func, req));
    } else {
        std::string response = offline_respond(user_text);
        std::string display = parse_and_execute_actions(response);
        add_bubble("✦ AMS Copilot", display, false);
        history.push_back({"model", response});
    }
}

static void on_send_clicked(GtkWidget *, gpointer) { send_message(); }

static gboolean on_key_press(GtkWidget *, GdkEventKey *ev, gpointer) {
    if (ev->keyval == GDK_KEY_Return || ev->keyval == GDK_KEY_KP_Enter) {
        send_message();
        return TRUE;
    }
    return FALSE;
}

/* ══════════════════════════════════════════════════════════════
   Additional Chat CSS (loaded on top of shared theme)
   ══════════════════════════════════════════════════════════════ */

static const char *CHAT_CSS = R"CSS(

.chat-area {
    background-color: rgba(0,0,0,0.3);
    border-radius: 12px;
}

.chat-bubble-user {
    background-color: rgba(99,102,241,0.3);
    border: 1px solid rgba(99,102,241,0.2);
    border-radius: 16px 16px 4px 16px;
    padding: 10px 14px;
}

.chat-bubble-ai {
    background-color: rgba(255,255,255,0.06);
    border: 1px solid rgba(255,255,255,0.06);
    border-radius: 16px 16px 16px 4px;
    padding: 10px 14px;
}

.chat-sender-user {
    color: #818cf8;
    font-size: 10px;
    font-weight: 700;
    margin-bottom: 2px;
}

.chat-sender-ai {
    color: #34d399;
    font-size: 10px;
    font-weight: 700;
    margin-bottom: 2px;
}

.chat-text {
    color: rgba(255,255,255,0.9);
    font-size: 13px;
}

.chat-input {
    background-color: rgba(255,255,255,0.08);
    border: 1px solid rgba(255,255,255,0.1);
    border-radius: 24px;
    color: white;
    padding: 10px 18px;
    font-size: 14px;
    caret-color: white;
}
.chat-input:focus {
    border-color: rgba(139,92,246,0.5);
    background-color: rgba(255,255,255,0.12);
}

.chat-send {
    background-color: rgba(99,102,241,0.5);
    border: none;
    border-radius: 50px;
    color: white;
    font-size: 18px;
    min-width: 44px;
    min-height: 44px;
    padding: 0;
}
.chat-send:hover {
    background-color: rgba(99,102,241,0.7);
}

.chat-typing {
    color: rgba(255,255,255,0.35);
    font-size: 12px;
    font-style: italic;
    padding: 4px 12px;
}

.chat-status-online  { color: #34d399; font-size: 11px; }
.chat-status-offline { color: #f87171; font-size: 11px; }

.chat-welcome {
    color: rgba(255,255,255,0.4);
    font-size: 12px;
    padding: 8px 16px;
}

)CSS";

/* ══════════════════════════════════════════════════════════════
   Application UI
   ══════════════════════════════════════════════════════════════ */

static void on_activate(GtkApplication *app, gpointer) {
    ams_apply_theme();

    /* Load chat-specific CSS */
    GtkCssProvider *cp = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cp, CHAT_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(cp),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
    g_object_unref(cp);

    /* Check API key */
    const char *deepseek_key = getenv("DEEPSEEK_API_KEY");
    const char *gemini_key = getenv("GEMINI_API_KEY");

    if (deepseek_key && strlen(deepseek_key) > 0) {
        api_key = deepseek_key;
        current_provider = PROVIDER_DEEPSEEK;
        online_mode = true;
        fprintf(stderr, "[AMS Copilot] DeepSeek API key found, online mode enabled.\n");
    } else if (gemini_key && strlen(gemini_key) > 0) {
        api_key = gemini_key;
        current_provider = PROVIDER_GEMINI;
        online_mode = true;
        fprintf(stderr, "[AMS Copilot] Gemini API key found, online mode enabled.\n");
    } else {
        current_provider = PROVIDER_NONE;
        online_mode = false;
        fprintf(stderr, "[AMS Copilot] No API key set (GEMINI_API_KEY or DEEPSEEK_API_KEY), running in offline mode.\n");
    }

    /* Verify curl is available for online mode */
    if (online_mode) {
        bool curl_ok = (access("/usr/bin/curl", X_OK) == 0);
        if (!curl_ok) {
            /* Fallback: try which command */
            FILE *fp = popen("which curl 2>/dev/null", "r");
            if (fp) {
                char buf[256];
                if (fgets(buf, sizeof(buf), fp) && strlen(buf) > 0)
                    curl_ok = true;
                pclose(fp);
            }
        }
        if (!curl_ok) {
            online_mode = false;
            fprintf(stderr, "[AMS Copilot] curl not found, falling back to offline mode.\n");
        }
    }

    /* ── Window ── */
    GtkWidget *win = ams_window(app, "AMS Copilot", "accessories-character-map", 520, 650);

    /* Update subtitle with status */
    GtkWidget *hbar = gtk_window_get_titlebar(GTK_WINDOW(win));
    std::string subtitle;
    if (online_mode) {
        if (current_provider == PROVIDER_DEEPSEEK)
            subtitle = "🟢  Online — DeepSeek API";
        else
            subtitle = "🟢  Online — Gemini API";
    } else {
        subtitle = "🔴  Offline — Local Engine";
    }
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(hbar), subtitle.c_str());

    /* ── Main layout ── */
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(win), vbox);

    /* ── Chat area (scrollable) ── */
    chat_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(chat_scroll),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    ams_css(chat_scroll, "chat-area");
    gtk_box_pack_start(GTK_BOX(vbox), chat_scroll, TRUE, TRUE, 0);

    chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_top(chat_box, 8);
    gtk_widget_set_margin_bottom(chat_box, 8);
    gtk_container_add(GTK_CONTAINER(chat_scroll), chat_box);

    /* ── Typing indicator ── */
    typing_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(typing_box, 12);
    gtk_widget_set_margin_top(typing_box, 4);
    gtk_widget_set_margin_bottom(typing_box, 4);

    GtkWidget *spinner = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(spinner));
    gtk_widget_set_size_request(spinner, 16, 16);
    gtk_box_pack_start(GTK_BOX(typing_box), spinner, FALSE, FALSE, 0);

    GtkWidget *typing_lbl = gtk_label_new("AMS Copilot is thinking...");
    ams_css(typing_lbl, "chat-typing");
    gtk_box_pack_start(GTK_BOX(typing_box), typing_lbl, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), typing_box, FALSE, FALSE, 0);
    gtk_widget_set_no_show_all(typing_box, TRUE);
    gtk_widget_hide(typing_box);

    /* ── Input area ── */
    GtkWidget *input_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_margin_start(input_bar, 8);
    gtk_widget_set_margin_end(input_bar, 8);
    gtk_widget_set_margin_top(input_bar, 8);
    gtk_widget_set_margin_bottom(input_bar, 8);

    input_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_entry), "Ask me anything...");
    ams_css(input_entry, "chat-input");
    gtk_box_pack_start(GTK_BOX(input_bar), input_entry, TRUE, TRUE, 0);

    GtkWidget *send_btn = gtk_button_new_with_label("➤");
    ams_css(send_btn, "chat-send");
    gtk_box_pack_start(GTK_BOX(input_bar), send_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), input_bar, FALSE, FALSE, 0);

    /* ── Signals ── */
    g_signal_connect(send_btn, "clicked", G_CALLBACK(on_send_clicked), NULL);
    g_signal_connect(win, "key-press-event", G_CALLBACK(on_key_press), NULL);

    gtk_widget_show_all(win);
    gtk_widget_hide(typing_box);

    /* ── Welcome message ── */
    std::string welcome =
        "Welcome to AMS Copilot! ✦\n\n"
        "I can help you with:\n"
        "• Launch apps → \"open calculator\"\n"
        "• OS concepts → \"explain deadlock\"\n"
        "• Play games → \"play chess\"\n"
        "• General chat → ask me anything!\n\n";
    if (online_mode) {
        if (current_provider == PROVIDER_DEEPSEEK)
            welcome += "Status: 🟢 Online — powered by DeepSeek AI";
        else
            welcome += "Status: 🟢 Online — powered by Gemini AI";
    } else {
        welcome += "Status: 🔴 Offline — set GEMINI_API_KEY or DEEPSEEK_API_KEY for full AI mode";
    }

    add_bubble("✦ AMS Copilot", welcome, false);
}

/* ══════════════════════════════════════════════════════════════
   Entry Point
   ══════════════════════════════════════════════════════════════ */

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    GtkApplication *app = gtk_application_new("com.ams.task.ai_copilot", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int s = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return s;
}
