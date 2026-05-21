/*
AMS OS — AI Copilot
Intelligent AI assistant powered by Groq/DeepSeek/Gemini API (online)
or a built-in rule engine (offline).  Can launch apps via natural language
and DYNAMICALLY CREATE new GUI applications on demand.
*/

#include "../gui_theme.h"
#include <signal.h>
#include <map>
#include <sys/wait.h>
#include <dirent.h>

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
    "give clear, educational explanations.\n"
    "6. If the user asks to play a specific song, you must launch the music player with the requested song title as a parameter. "
    "The official playlist is:\n"
    "   - 'Midnight Drive' by Neon Pulse\n"
    "   - 'Electric Dreams' by Synthwave Radio\n"
    "   - 'Starlight Sonata' by Luna Echo\n"
    "   - 'Digital Rain' by Cyber Horizon\n"
    "   - 'Atomic Heartbeat' by AMS Band\n"
    "   - 'Velvet Cascade' by Dream Weaver\n"
    "To play a specific song, append `<<LAUNCH:music_player:Song Title>>` to your reply (e.g. `<<LAUNCH:music_player:Midnight Drive>>`). "
    "If they ask to 'play music' generally without specifying a song, play 'Midnight Drive' by default: `<<LAUNCH:music_player:Midnight Drive>>`.\n\n"
    "7. **DYNAMIC APP CREATION** — You can CREATE brand-new GTK3 GUI applications!\n"
    "When the user asks you to create a program, game, tool, or any application, "
    "you MUST generate a complete, compilable C++ GTK3 source file and wrap it in a special XML block.\n\n"
    "FORMAT:\n"
    "```\n"
    "<create_app id=\"app_id\" emoji=\"🎯\" name=\"My App\">\n"
    "// COMPLETE C++ source code here\n"
    "</create_app>\n"
    "```\n\n"
    "RULES FOR GENERATED CODE:\n"
    "- `id` must be a lowercase_snake_case identifier (e.g. tic_tac_toe, paint_canvas, color_picker)\n"
    "- The code MUST start with: `#include \"../gui_theme.h\"`\n"
    "- The code MUST use `signal(SIGCHLD, SIG_IGN);` in main()\n"
    "- The code MUST follow this exact pattern:\n"
    "```cpp\n"
    "#include \"../gui_theme.h\"\n"
    "#include <signal.h>\n"
    "// ... other standard includes as needed\n"
    "\n"
    "// Application state and logic here\n"
    "\n"
    "static void on_activate(GtkApplication *app, gpointer) {\n"
    "    ams_apply_theme();\n"
    "    GtkWidget *win = ams_window(app, \"Title\", \"icon-name\", WIDTH, HEIGHT);\n"
    "    // Build the full UI here\n"
    "    gtk_widget_show_all(win);\n"
    "}\n"
    "\n"
    "int main(int argc, char *argv[]) {\n"
    "    signal(SIGCHLD, SIG_IGN);\n"
    "    GtkApplication *app = gtk_application_new(\"com.ams.task.APPID\", G_APPLICATION_FLAGS_NONE);\n"
    "    g_signal_connect(app, \"activate\", G_CALLBACK(on_activate), NULL);\n"
    "    int s = g_application_run(G_APPLICATION(app), argc, argv);\n"
    "    g_object_unref(app);\n"
    "    return s;\n"
    "}\n"
    "```\n"
    "- Use the shared theme: ams_apply_theme(), ams_window(), ams_css(), ams_card(), ams_info_row()\n"
    "- Available CSS classes: .card, .accent, .dim, .success, .error-text, .title-lg, .title-xl, .subtitle-lbl\n"
    "- For games, use GtkDrawingArea with cairo for 2D rendering\n"
    "- For forms/tools, use GtkEntry, GtkButton, GtkLabel, GtkTextView, etc.\n"
    "- Make apps premium and polished — proper spacing, dark theme, great UX\n"
    "- The code MUST be 100% complete and compilable. NO placeholders, NO TODOs.\n"
    "- Always include ALL necessary #include headers\n"
    "- CRITICAL C++ RULES TO AVOID COMPILATION ERRORS:\n"
    "  * Use std::string for ALL string variables, never raw char arrays for mutable strings\n"
    "  * gtk_button_get_label() returns const gchar* (const char*). Compare with strcmp(), do NOT assign to std::string directly in comparisons\n"
    "  * gtk_entry_get_text() returns const gchar*. Use std::string(gtk_entry_get_text(...)) to convert\n"
    "  * When setting labels: gtk_label_set_text(GTK_LABEL(lbl), str.c_str()) — always use .c_str()\n"
    "  * Use g_signal_connect with G_CALLBACK() wrapper for all signal handlers\n"
    "  * Use GINT_TO_POINTER/GPOINTER_TO_INT for passing integer data through gpointer\n"
    "  * Always use G_APPLICATION_NON_UNIQUE instead of G_APPLICATION_FLAGS_NONE\n"
    "  * For Tic-Tac-Toe and grid games: use a static char array for game state, NOT button labels\n"
    "  * For drawing games: use gtk_widget_queue_draw() to trigger redraws\n"
    "  * For key events: use key-press-event signal on the window widget\n"
    "  * Always cast callback functions: G_CALLBACK(+[](GtkWidget*, gpointer) { ... })\n"
    "- BEFORE the <create_app> block, write a SHORT friendly message (1-2 sentences) about the app you're creating.\n"
    "- AFTER the </create_app> closing tag, do NOT write anything else.\n\n"
    "EXAMPLE — if user says 'create a color picker app':\n"
    "Sure! I'm creating a Color Picker app for you! 🎨\n"
    "<create_app id=\"color_picker\" emoji=\"🎨\" name=\"Color Picker\">\n"
    "#include \"../gui_theme.h\"\n"
    "#include <signal.h>\n"
    "// ... complete code ...\n"
    "</create_app>\n\n"
    "8. **ADVANCED FILESYSTEM OPERATIONS** — You can create folders, files, and update existing files.\n"
    "To create a folder: `<create_folder path=\"folder/name\"></create_folder>`\n"
    "To create a file: `<create_file path=\"path/to/file.txt\">file content here</create_file>`\n"
    "To update a file: \n"
    "`<update_file path=\"path/to/file.ext\">\n"
    "===SEARCH===\n"
    "old content to find\n"
    "===REPLACE===\n"
    "new content to replace with\n"
    "</update_file>`\n"
    "For updates, you MUST match the SEARCH block exactly with the file's current contents (including whitespace).\n"
    "You can chain multiple operations together in your response.\n\n";

/* ══════════════════════════════════════════════════════════════
   Dynamic App Registry — loaded from data/desktop_apps.txt
   ══════════════════════════════════════════════════════════════ */

struct AppEntry {
    std::string id;
    std::string exec_path;
};

static std::vector<AppEntry> dynamic_apps;

static void load_app_registry() {
    dynamic_apps.clear();
    std::ifstream f("data/desktop_apps.txt");
    if (!f.is_open()) return;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        /* Format: id|name|emoji|exec_path */
        size_t p1 = line.find('|');
        if (p1 == std::string::npos) continue;
        size_t p2 = line.find('|', p1 + 1);
        if (p2 == std::string::npos) continue;
        size_t p3 = line.find('|', p2 + 1);
        if (p3 == std::string::npos) continue;

        std::string name = line.substr(p1 + 1, p2 - p1 - 1);
        std::string exec_path = line.substr(p3 + 1);

        /* Derive app_id from exec_path: ./build/gui_xxx → xxx */
        std::string app_id;
        size_t gui_pos = exec_path.find("gui_");
        if (gui_pos != std::string::npos) {
            app_id = exec_path.substr(gui_pos + 4);
        } else {
            /* Fallback: use name lowercased with spaces→underscores */
            app_id = name;
            std::transform(app_id.begin(), app_id.end(), app_id.begin(), ::tolower);
            std::replace(app_id.begin(), app_id.end(), ' ', '_');
        }

        dynamic_apps.push_back({app_id, exec_path});
    }
}

static std::map<std::string, std::string> get_app_map() {
    std::map<std::string, std::string> m;
    for (auto &a : dynamic_apps) {
        m[a.id] = a.exec_path;
    }
    return m;
}

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

enum Provider { PROVIDER_NONE, PROVIDER_GEMINI, PROVIDER_DEEPSEEK, PROVIDER_GROQ };
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
            if (json[pos + 1] == 'u' && pos + 5 < json.size()) {
                /* Parse 4 hex digits: json[pos+2..pos+5] */
                std::string hex_str = json.substr(pos + 2, 4);
                try {
                    unsigned long codepoint = std::stoul(hex_str, nullptr, 16);
                    if (codepoint <= 0x7f) {
                        result += (char)codepoint;
                    } else if (codepoint <= 0x7ff) {
                        result += (char)(0xc0 | ((codepoint >> 6) & 0x1f));
                        result += (char)(0x80 | (codepoint & 0x3f));
                    } else if (codepoint <= 0xffff) {
                        result += (char)(0xe0 | ((codepoint >> 12) & 0x0f));
                        result += (char)(0x80 | ((codepoint >> 6) & 0x3f));
                        result += (char)(0x80 | (codepoint & 0x3f));
                    }
                } catch (...) {
                    /* Fallback if parsing fails: just retain raw chars */
                    result += "\\u" + hex_str;
                }
                pos += 6;
            } else {
                switch (json[pos + 1]) {
                    case '"':  result += '"';  break;
                    case '\\': result += '\\'; break;
                    case 'n':  result += '\n'; break;
                    case 'r':  result += '\r'; break;
                    case 't':  result += '\t'; break;
                    default:   result += json[pos + 1]; break;
                }
                pos += 2;
            }
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
   Dynamic App Creator Engine
   ══════════════════════════════════════════════════════════════ */

static std::string create_new_program(const std::string &id, const std::string &emoji,
                                       const std::string &name, const std::string &code) {
    /* 1. Write source file */
    std::string src_path = "gui/tasks/" + id + ".cpp";
    {
        std::ofstream f(src_path);
        if (!f.is_open()) {
            return "❌ Failed to write source file: " + src_path;
        }
        f << code;
    }
    fprintf(stderr, "[AMS Copilot] Wrote source: %s\n", src_path.c_str());

    /* 2. Check if already in makefile's GUI_TASK_NAMES, if not add it */
    {
        std::ifstream mf("makefile");
        std::string makefile_content((std::istreambuf_iterator<char>(mf)),
                                      std::istreambuf_iterator<char>());
        mf.close();

        /* Check if the id is already listed */
        if (makefile_content.find("\t" + id) == std::string::npos &&
            makefile_content.find(" " + id) == std::string::npos) {
            
            size_t gui_pos = makefile_content.find("GUI_TASK_NAMES :=");
            if (gui_pos != std::string::npos) {
                // Find the first newline after GUI_TASK_NAMES :=
                size_t eol = makefile_content.find('\n', gui_pos);
                if (eol != std::string::npos) {
                    // Insert the new app immediately under the GUI_TASK_NAMES := \ declaration
                    std::string new_content = makefile_content.substr(0, eol) + "\n\t" + id + " \\";
                    new_content += makefile_content.substr(eol);
                    std::ofstream out("makefile");
                    out << new_content;
                }
            }
        }
    }
    fprintf(stderr, "[AMS Copilot] Updated makefile for: %s\n", id.c_str());

    /* 3. Compile the new app */
    std::string build_cmd = "make build/gui_" + id + " 2>&1";
    fprintf(stderr, "[AMS Copilot] Compiling: %s\n", build_cmd.c_str());
    FILE *fp = popen(build_cmd.c_str(), "r");
    std::string compile_output;
    if (fp) {
        char buf[1024];
        while (fgets(buf, sizeof(buf), fp))
            compile_output += buf;
        int status = pclose(fp);
        fprintf(stderr, "[AMS Copilot] Compile exit: %d\n", WEXITSTATUS(status));
        fprintf(stderr, "[AMS Copilot] Compile output: %s\n", compile_output.c_str());

        if (WEXITSTATUS(status) != 0) {
            /* Compilation failed — remove the source file to keep things clean */
            std::remove(src_path.c_str());
            return "❌ Compilation failed:\n" + compile_output.substr(0, 300);
        }
    } else {
        return "❌ Could not run make. Is it installed?";
    }

    /* 4. Compute next ID */
    int next_id = 20;
    {
        std::ifstream rf("data/desktop_apps.txt");
        std::string line;
        while (std::getline(rf, line)) {
            if (line.empty()) continue;
            size_t p = line.find('|');
            if (p != std::string::npos) {
                try {
                    int n = std::stoi(line.substr(0, p));
                    if (n >= next_id) next_id = n + 1;
                } catch (...) {}
            }
        }
    }

    /* 5. Register in desktop_apps.txt */
    std::string exec_path = "./build/gui_" + id;
    {
        std::ofstream rf("data/desktop_apps.txt", std::ios::app);
        rf << next_id << "|" << name << "|" << emoji << "|" << exec_path << "\n";
    }
    fprintf(stderr, "[AMS Copilot] Registered app: %s (#%d)\n", name.c_str(), next_id);

    /* 6. Reload our own app registry */
    load_app_registry();

    return "✅ Successfully created **" + name + "** " + emoji + "!\n"
           "The app has been compiled and added to your desktop.\n"
           "Switch to the Desktop and it will appear automatically, or click the 🔄 Refresh button.\n"
           "You can also say \"open " + id + "\" to launch it right now!";
}

/* ══════════════════════════════════════════════════════════════
   Offline Rule Engine
   ══════════════════════════════════════════════════════════════ */

static std::string to_lower(const std::string &s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

static std::string offline_respond(const std::string &input) {
    std::string q = to_lower(input);

    /* ── Play specific song offline (check first!) ── */
    if (q.find("play") != std::string::npos || q.find("song") != std::string::npos || q.find("music") != std::string::npos) {
        struct SongAlias { const char *pattern; const char *title; };
        static const SongAlias songs[] = {
            {"midnight drive",   "Midnight Drive"},
            {"electric dreams",  "Electric Dreams"},
            {"starlight sonata", "Starlight Sonata"},
            {"digital rain",     "Digital Rain"},
            {"atomic heartbeat", "Atomic Heartbeat"},
            {"velvet cascade",   "Velvet Cascade"},
        };
        for (auto &s : songs) {
            if (q.find(s.pattern) != std::string::npos) {
                return std::string("Sure! Playing \"") + s.title + "\" in the Music Player! 🎵 <<LAUNCH:music_player:" + s.title + ">>";
            }
        }

        /* ── Also scan data/music/ folder for matching real audio files ── */
        {
            DIR *dir = opendir("data/music");
            if (dir) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    std::string fname = entry->d_name;
                    if (fname == "." || fname == "..") continue;
                    size_t dot = fname.rfind('.');
                    if (dot == std::string::npos) continue;
                    std::string ext = fname.substr(dot);
                    std::string ext_lower = ext;
                    std::transform(ext_lower.begin(), ext_lower.end(), ext_lower.begin(), ::tolower);
                    if (ext_lower != ".wav" && ext_lower != ".mp3" && ext_lower != ".ogg") continue;

                    /* Get title from filename without extension */
                    std::string title = fname.substr(0, dot);
                    std::string title_lower = to_lower(title);

                    if (q.find(title_lower) != std::string::npos) {
                        closedir(dir);
                        return "🎵 Playing \"" + title + "\" from your music library! <<LAUNCH:music_player:" + title + ">>";
                    }
                }
                closedir(dir);
            }
        }
        
        if (q.find("play music") != std::string::npos || q.find("play song") != std::string::npos || 
            q.find("play a song") != std::string::npos || q.find("play some music") != std::string::npos) {
            return "Playing Midnight Drive by Neon Pulse in the Music Player! 🎵 <<LAUNCH:music_player:Midnight Drive>>";
        }
    }

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
        /* Also check dynamic apps */
        for (auto &da : dynamic_apps) {
            if (q.find(da.id) != std::string::npos) {
                return std::string("Sure! Launching ") + da.id + " for you! 🚀 <<LAUNCH:" + da.id + ">>";
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

    /* ── Create app offline ── */
    if (q.find("create") != std::string::npos && (q.find("app") != std::string::npos ||
        q.find("program") != std::string::npos || q.find("game") != std::string::npos ||
        q.find("tool") != std::string::npos || q.find("application") != std::string::npos)) {
        return "I can create custom applications for you, but I need to be in online mode 🌐 to generate the code.\n"
               "Please set one of these environment variables:\n"
               "• `export GROQ_API_KEY=your_key`\n"
               "• `export DEEPSEEK_API_KEY=your_key`\n"
               "• `export GEMINI_API_KEY=your_key`\n"
               "Then restart AI Copilot to enable app creation! 🚀";
    }

    /* ── Identity ── */
    if (q.find("who are you") != std::string::npos || q.find("your name") != std::string::npos)
        return "I'm ✦ AMS Copilot, the built-in AI assistant for AMS Operating System! "
               "I can launch apps, answer OS questions, and chat with you. 😊";

    if (q.find("what can you do") != std::string::npos || q.find("help") != std::string::npos)
        return "I can help you with:\n"
               "• Launch any app (try \"open calculator\")\n"
               "• Answer OS concepts (scheduling, deadlock, memory)\n"
               "• Create new apps (online mode) → \"create a tic-tac-toe game\"\n"
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
           "Set the GROQ_API_KEY, DEEPSEEK_API_KEY, or GEMINI_API_KEY environment variable to enable full AI mode! 🚀";
}

/* ══════════════════════════════════════════════════════════════
   Action Parser — extract and execute <<LAUNCH:xxx>> tags
                   and <create_app> blocks
   ══════════════════════════════════════════════════════════════ */

static std::string parse_and_execute_actions(const std::string &text) {
    std::string display = text;

    /* ── Handle <create_app> blocks first ── */
    {
        size_t ca_start = display.find("<create_app");
        if (ca_start != std::string::npos) {
            size_t ca_end = display.find("</create_app>");
            if (ca_end != std::string::npos) {
                size_t tag_end = display.find('>', ca_start);
                if (tag_end != std::string::npos && tag_end < ca_end) {
                    /* Parse attributes from the opening tag */
                    std::string tag = display.substr(ca_start, tag_end - ca_start + 1);
                    std::string code = display.substr(tag_end + 1, ca_end - tag_end - 1);

                    /* Extract id="..." */
                    std::string app_id, app_emoji, app_name;
                    auto extract_attr = [&tag](const std::string &attr) -> std::string {
                        std::string search = attr + "=\"";
                        size_t p = tag.find(search);
                        if (p == std::string::npos) return "";
                        p += search.size();
                        size_t q = tag.find('"', p);
                        if (q == std::string::npos) return "";
                        return tag.substr(p, q - p);
                    };

                    app_id = extract_attr("id");
                    app_emoji = extract_attr("emoji");
                    app_name = extract_attr("name");

                    if (app_id.empty()) app_id = "custom_app";
                    if (app_emoji.empty()) app_emoji = "🆕";
                    if (app_name.empty()) app_name = "Custom App";

                    /* Trim leading/trailing whitespace from code */
                    size_t code_start = code.find_first_not_of(" \t\n\r");
                    size_t code_end_pos = code.find_last_not_of(" \t\n\r");
                    if (code_start != std::string::npos && code_end_pos != std::string::npos) {
                        code = code.substr(code_start, code_end_pos - code_start + 1);
                    }

                    /* Remove the <create_app>...</create_app> block from display */
                    std::string before = display.substr(0, ca_start);
                    std::string after = (ca_end + 13 < display.size()) ? display.substr(ca_end + 13) : "";

                    /* Trim before text */
                    while (!before.empty() && (before.back() == ' ' || before.back() == '\n'))
                        before.pop_back();

                    /* Execute the creation */
                    std::string result = create_new_program(app_id, app_emoji, app_name, code);

                    display = before;
                    if (!display.empty()) display += "\n\n";
                    display += result;

                    /* Trim after text */
                    while (!after.empty() && (after.front() == ' ' || after.front() == '\n'))
                        after.erase(after.begin());
                    if (!after.empty()) display += "\n" + after;
                }
            }
        }
    }

    /* ── Handle Advanced Filesystem Actions ── */
    
    // <create_folder>
    size_t cf_pos = 0;
    while ((cf_pos = display.find("<create_folder")) != std::string::npos) {
        size_t cf_end = display.find("</create_folder>", cf_pos);
        if (cf_end == std::string::npos) break;
        
        std::string tag = display.substr(cf_pos, display.find('>', cf_pos) - cf_pos + 1);
        std::string path;
        size_t p = tag.find("path=\"");
        if (p != std::string::npos) {
            p += 6;
            size_t q = tag.find('"', p);
            if (q != std::string::npos) path = tag.substr(p, q - p);
        }
        
        if (!path.empty()) {
            std::string cmd = "mkdir -p \"" + path + "\"";
            int ret = system(cmd.c_str());
            (void)ret; // Ignore return value warning
            fprintf(stderr, "[AMS Copilot] Created folder: %s\n", path.c_str());
            display.replace(cf_pos, cf_end - cf_pos + 16, "📁 Created folder: " + path);
        } else {
            display.erase(cf_pos, cf_end - cf_pos + 16);
        }
    }

    // <create_file>
    size_t file_pos = 0;
    while ((file_pos = display.find("<create_file")) != std::string::npos) {
        size_t file_end = display.find("</create_file>", file_pos);
        if (file_end == std::string::npos) break;
        
        size_t tag_end = display.find('>', file_pos);
        if (tag_end == std::string::npos || tag_end > file_end) break;
        
        std::string tag = display.substr(file_pos, tag_end - file_pos + 1);
        std::string content = display.substr(tag_end + 1, file_end - tag_end - 1);
        
        if (!content.empty() && content.front() == '\n') content.erase(0, 1);
        if (!content.empty() && content.back() == '\n') content.pop_back();

        std::string path;
        size_t p = tag.find("path=\"");
        if (p != std::string::npos) {
            p += 6;
            size_t q = tag.find('"', p);
            if (q != std::string::npos) path = tag.substr(p, q - p);
        }
        
        if (!path.empty()) {
            std::ofstream out(path);
            out << content;
            out.close();
            fprintf(stderr, "[AMS Copilot] Created file: %s\n", path.c_str());
            display.replace(file_pos, file_end - file_pos + 14, "📄 Created file: " + path);
        } else {
            display.erase(file_pos, file_end - file_pos + 14);
        }
    }

    // <update_file>
    size_t up_pos = 0;
    while ((up_pos = display.find("<update_file")) != std::string::npos) {
        size_t up_end = display.find("</update_file>", up_pos);
        if (up_end == std::string::npos) break;
        
        size_t tag_end = display.find('>', up_pos);
        if (tag_end == std::string::npos || tag_end > up_end) break;
        
        std::string tag = display.substr(up_pos, tag_end - up_pos + 1);
        std::string inner = display.substr(tag_end + 1, up_end - tag_end - 1);
        
        std::string path;
        size_t p = tag.find("path=\"");
        if (p != std::string::npos) {
            p += 6;
            size_t q = tag.find('"', p);
            if (q != std::string::npos) path = tag.substr(p, q - p);
        }
        
        size_t s_pos = inner.find("===SEARCH===");
        size_t r_pos = inner.find("===REPLACE===");
        
        if (!path.empty() && s_pos != std::string::npos && r_pos != std::string::npos && r_pos > s_pos) {
            std::string search_text = inner.substr(s_pos + 12, r_pos - (s_pos + 12));
            std::string replace_text = inner.substr(r_pos + 13);
            
            if (!search_text.empty() && search_text.front() == '\n') search_text.erase(0, 1);
            if (!search_text.empty() && search_text.back() == '\n') search_text.pop_back();
            if (!replace_text.empty() && replace_text.front() == '\n') replace_text.erase(0, 1);
            if (!replace_text.empty() && replace_text.back() == '\n') replace_text.pop_back();
            
            std::ifstream in(path);
            if (in.is_open()) {
                std::string file_content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                in.close();
                
                size_t rep_idx = file_content.find(search_text);
                if (rep_idx != std::string::npos) {
                    file_content.replace(rep_idx, search_text.size(), replace_text);
                    std::ofstream out(path);
                    out << file_content;
                    out.close();
                    fprintf(stderr, "[AMS Copilot] Updated file: %s\n", path.c_str());
                    display.replace(up_pos, up_end - up_pos + 14, "✏️ Updated file: " + path);
                } else {
                    fprintf(stderr, "[AMS Copilot] Update failed (search text not found): %s\n", path.c_str());
                    display.replace(up_pos, up_end - up_pos + 14, "❌ Update failed for " + path + ": search text not found in file.");
                }
            } else {
                display.replace(up_pos, up_end - up_pos + 14, "❌ Update failed: could not open " + path);
            }
        } else {
            display.erase(up_pos, up_end - up_pos + 14);
        }
    }

    /* ── Handle <<LAUNCH:xxx>> tags ── */
    size_t pos = 0;
    auto app_map = get_app_map();

    while ((pos = display.find("<<LAUNCH:", pos)) != std::string::npos) {
        size_t end = display.find(">>", pos);
        if (end == std::string::npos) break;

        std::string app = display.substr(pos + 9, end - pos - 9);
        display.erase(pos, end - pos + 2);  /* remove the tag */

        std::string app_name = app;
        std::string app_arg = "";
        size_t colon = app.find(':');
        if (colon != std::string::npos) {
            app_name = app.substr(0, colon);
            app_arg = app.substr(colon + 1);
        }

        auto it = app_map.find(app_name);
        if (it != app_map.end()) {
            pid_t pid = fork();
            if (pid == 0) {
                setsid();
                if (!app_arg.empty()) {
                    execlp(it->second.c_str(), it->second.c_str(), app_arg.c_str(), (char *)NULL);
                } else {
                    execlp(it->second.c_str(), it->second.c_str(), (char *)NULL);
                }
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
    json += "\"}]},\"generationConfig\":{\"maxOutputTokens\":4096,\"temperature\":0.7}}";

    return json;
}

static std::string build_openai_compatible_json(const std::string &model) {
    std::string json = "{\"model\":\"" + model + "\",\"messages\":[";
    json += "{\"role\":\"system\",\"content\":\"" + json_escape(SYSTEM_PROMPT) + "\"}";

    int start = (int)history.size() - 20;
    if (start < 0) start = 0;

    for (int i = start; i < (int)history.size(); i++) {
        json += ",";
        std::string role = (history[i].role == "model") ? "assistant" : "user";
        json += "{\"role\":\"" + role + "\",\"content\":\"" + json_escape(history[i].text) + "\"}";
    }

    json += "],\"stream\":false,\"max_tokens\":4096,\"temperature\":0.7}";

    return json;
}

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
    std::string json_body;
    if (current_provider == PROVIDER_DEEPSEEK) {
        json_body = build_openai_compatible_json("deepseek-chat");
    } else if (current_provider == PROVIDER_GROQ) {
        json_body = build_openai_compatible_json("llama-3.3-70b-versatile");
    } else {
        json_body = build_gemini_json();
    }
    
    std::string tmp_path = "/tmp/ams_copilot_req.json";
    {
        std::ofstream tmp(tmp_path);
        tmp << json_body;
    }

    std::string cmd;
    if (current_provider == PROVIDER_DEEPSEEK) {
        cmd = "curl -s -m 60 -X POST \"https://api.deepseek.com/chat/completions\" "
              "-H \"Content-Type: application/json\" "
              "-H \"Authorization: Bearer " + api_key + "\" "
              "-d @" + tmp_path;
    } else if (current_provider == PROVIDER_GROQ) {
        cmd = "curl -s -m 60 -X POST \"https://api.groq.com/openai/v1/chat/completions\" "
              "-H \"Content-Type: application/json\" "
              "-H \"Authorization: Bearer " + api_key + "\" "
              "-d @" + tmp_path;
    } else {
        cmd = "curl -s -m 60 -X POST \"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" +
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
                else if (current_provider == PROVIDER_GROQ)
                    req->response = "⚠️ API Error. Please check your GROQ_API_KEY.";
                else
                    req->response = "⚠️ API Error. Please check your GEMINI_API_KEY.";
            } else {
                req->response = "⚠️ API Error: " + err_msg;
            }
        } else {
            if (current_provider == PROVIDER_DEEPSEEK || current_provider == PROVIDER_GROQ) {
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

    /* Load dynamic app registry */
    load_app_registry();

    /* Load chat-specific CSS */
    GtkCssProvider *cp = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cp, CHAT_CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(cp),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);
    g_object_unref(cp);

    /* Check API key — priority: GROQ > DEEPSEEK > GEMINI */
    const char *groq_key = getenv("GROQ_API_KEY");
    const char *deepseek_key = getenv("DEEPSEEK_API_KEY");
    const char *gemini_key = getenv("GEMINI_API_KEY");

    if (groq_key && strlen(groq_key) > 0) {
        api_key = groq_key;
        current_provider = PROVIDER_GROQ;
        online_mode = true;
        fprintf(stderr, "[AMS Copilot] Groq API key found, online mode enabled.\n");
    } else if (deepseek_key && strlen(deepseek_key) > 0) {
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
        fprintf(stderr, "[AMS Copilot] No API key set (GROQ_API_KEY, DEEPSEEK_API_KEY, or GEMINI_API_KEY), running in offline mode.\n");
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
        else if (current_provider == PROVIDER_GROQ)
            subtitle = "🟢  Online — Groq API";
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
        "• Play music → \"play Midnight Drive\"\n"
        "• Create apps → \"create a tic-tac-toe game\" (online mode)\n\n";
    if (online_mode) {
        if (current_provider == PROVIDER_DEEPSEEK)
            welcome += "Status: 🟢 Online — powered by DeepSeek AI";
        else if (current_provider == PROVIDER_GROQ)
            welcome += "Status: 🟢 Online — powered by Groq (Llama 3)";
        else
            welcome += "Status: 🟢 Online — powered by Gemini AI";
    } else {
        welcome += "Status: 🔴 Offline — set GROQ_API_KEY, DEEPSEEK_API_KEY, or GEMINI_API_KEY for full AI mode";
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
