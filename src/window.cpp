#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"
#include "../other/third_party/json.hpp"

using json = nlohmann::json;
ofstream lg("log.txt", ios::app);

namespace Game {
    bool running = true;
    unordered_set<char> keys_down;
    mutex key_mutex;
}

namespace Config {
    int FPS;
};

bool compare(Sprite* s1, Sprite* s2) {
    return (s1->view.stage < s2->view.stage);
}

//  Gets a color of numeric value into string, ready to add to the buffer
string num_to_color(int clr) {
    //  Cut if more than 256
    while (clr > 256) clr /= 10;
    //  Build the string
    string out = "";
    if (clr > 99) out += to_string(clr);
    else if (clr > 9) out += "0" + to_string(clr);
    else out += "00" + to_string(clr);
    return out;
}

//  TODO - CLEAN UP THESE CONFIG FUNCTIONS - THEY USE THE SAME DAMNN HANDLE
void fix_console_size() {
    //Config
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD newSize;
    newSize.X = 200;
    newSize.Y = 100; 
    SetConsoleScreenBufferSize(hConsole, newSize);
    SMALL_RECT winSize = { 0, 0, static_cast<SHORT>(newSize.X - 1), static_cast<SHORT>(newSize.Y - 1) };
    SetConsoleWindowInfo(hConsole, TRUE, &winSize);
}

void fix_zoom() {
    //Config
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hInput, &mode);
    mode &= ~ENABLE_MOUSE_INPUT;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(hInput, mode);
}

void disable_text_selection() {
    //Config
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hInput, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT;
    SetConsoleMode(hInput, mode);
}

void enable_virtual_terminal() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void reset_cursor() {
    //Config
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = { 0, 0 };
    SetConsoleCursorPosition(hConsole, topLeft);
}

void disable_console_scroll() {
    //Config
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    COORD newSize = {
        static_cast<SHORT>(csbi.srWindow.Right - csbi.srWindow.Left + 1),
        static_cast<SHORT>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
    };
    SetConsoleScreenBufferSize(hConsole, newSize);
}

void disable_console_resize() {
    //Config
    HWND console = GetConsoleWindow();
    if (!console) return;
    LONG style = GetWindowLong(console, GWL_STYLE);
    style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
    SetWindowLong(console, GWL_STYLE, style);
    SetWindowPos(console, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void hide_console_cursor() {
    //Config
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; 
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void maximize_console() {
    //Config
    HWND hwnd = GetConsoleWindow();
    SetForegroundWindow(hwnd); 
    keybd_event(VK_F11, 0, 0, 0);
    keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
    fix_console_size();
    Sleep(100);
}

void Window::stringcpy(char* s1, char* s2) {
    for (int i = 0; s2[i] != '\0'; i++) 
        s1[i] = s2[i];
}

void log(int err_code, int sprite_label) {
    lg << "[CONSOLE] Program exited with error code: " << err_code << '\n';
    lg << "[CONSOLE] Sprite id: " << sprite_label << '\n';
    lg.flush();
    exit(err_code);
}

void Window::config() {
    //Reading from settings json
    ifstream conf("settings.json");
    json settings;
    screen_size.y = GetSystemMetrics(SM_CYFULLSCREEN); screen_size.x = GetSystemMetrics(SM_CXFULLSCREEN);
    if (!(conf.is_open()) || !(conf >> settings)) {
        if (!(conf.is_open())) lg << "[CONSOLE] Didn't find settings.json. Creating file...\n";
        conf.close();
        ofstream o("settings.json");
        lg << "[CONSOLE] Failed extracting settings.json. Using default settings...\n";
        json out;
        out["FPS"] = 60;
        Config::FPS = 60;
        o << out;
        o.close();
        return;
    }
    if (!settings.contains("FPS") && settings["FPS"].is_number()) log(102);
    else Config::FPS = settings["FPS"];
    conf.close();
}

void Window::empty_buffer() {
    char space[] = "\x1b[38;2;255;000;000m \0";
    char nlchar[] = "\x1b[38;2;255;000;000m\n\0";
    for (int i = 0; i < WINDOWHEIGHT; i++) {
        for (int j = 0; j < WINDOWLENGTH; j++) 
            stringcpy(buffer + (i * (WINDOWLENGTH + 1) * 20 + j * 20), space);
        stringcpy(buffer + (i * (WINDOWLENGTH + 1) * 20 + WINDOWLENGTH * 20), nlchar);
    }
    buffer[WINDOWHEIGHT * (WINDOWLENGTH + 1) * 20 - 1] = '\0';
}

void Window::DEBUG_fill() {
    for (int i = 0; i < WINDOWHEIGHT; i++) {
        for (int j = 0; j < WINDOWLENGTH; j++) cout << '*';
        if (i < WINDOWHEIGHT - 1) cout << '\n';
    }
    cout.flush();
}

void Window::print_buffer() {
    //Prints buffer with fwrite and flushes 
    fwrite(buffer, 1, total_buffer_size, stdout);
}  

Window::Window() {
    //Constructor for window
    if (!(lg.is_open())) {
        cout << 999;
        cin.get();
        exit(999);
    }
    ios_base::sync_with_stdio(false);
    config();
    unsigned int font_h = round((double) screen_size.y / FONT_RATIO_HEIGHT);
    unsigned int font_w = round((double) screen_size.x / FONT_RATIO_LENGTH);
    set_font_settings(font_h, font_w);
    hide_console_cursor();
    maximize_console();
    disable_console_resize();
    disable_console_scroll();
    disable_text_selection();
    fix_zoom();
    enable_virtual_terminal();
    empty_buffer();
}

void Window::set_font_settings(unsigned int f_height, unsigned int f_width) {
    //Vezi documentatia windows pentru consola
    if (f_height < 1 || f_width < 1) log(200);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) log(201);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    if (!GetCurrentConsoleFontEx(hConsole, TRUE, &cfi)) log(201);
    cfi.dwFontSize.X = f_width;
    cfi.dwFontSize.Y = f_height;
    wcscpy(cfi.FaceName, L"Cascadia Mono");
    if (!SetCurrentConsoleFontEx(hConsole, TRUE, &cfi)) log(201);
    font_size.y = f_height;
    font_size.x = f_width;
}

void Window::clean_renderer() {
    //Muta toate pozitiile libere la dreapta pentru un array compact
    unsigned int rmv = 0;
    unsigned int next_valid = 0;
    for (unsigned int i = 0; i < nr_of_sprites_in_renderer; i++)
        if (renderer[i] != nullptr) {
            if (i != next_valid) {
                renderer[next_valid] = renderer[i];
                renderer[i] = nullptr;
            }
            next_valid++;
        } else rmv++;
    nr_of_sprites_in_renderer -= rmv;
}

void Window::empty_renderer() {
    //Seteaza toate pointerele la nullptr
    for (unsigned int i = 0; i < nr_of_sprites_in_renderer; i++)
        renderer[i] = nullptr;
    nr_of_sprites_in_renderer = 0;
}

void Window::add_sprite_to_renderer(Sprite* s1) {
    //Adauga un sprite in renderer
    if (!s1) log(802);
    unsigned int i = nr_of_sprites_in_renderer;
    if (i == MAXNROFSPRITES) log(809, s1->label);
    renderer[i] = s1;
    nr_of_sprites_in_renderer++;
}

void Window::add_sprites_to_renderer(Sprite** s1, unsigned int sz) {
    //Adauga mai multe sprite-uri in renderer
    if (sz < 1) log(802);
    if (!s1) log(802);
    if (MAXNROFSPRITES <= nr_of_sprites_in_renderer + sz) 
        log(809, s1[0]->label);
    for (unsigned int i = nr_of_sprites_in_renderer; i < sz + nr_of_sprites_in_renderer; i++) {
        if (!s1[i]) log(802);
        renderer[i] = s1[i - nr_of_sprites_in_renderer];
    }
    nr_of_sprites_in_renderer += sz;
}

void Window::remove_sprites_from_renderer(Sprite** s1, unsigned int sz) {
    //Sterge mai multe sprite-uri in renderer dupa pointer
    if (sz < 1) log(802);
    if (!s1) log(802);
    for (unsigned int i = 0; i < nr_of_sprites_in_renderer; i++) {
        for (unsigned int j = 0; j < sz; j++) {
            if (renderer[i] == s1[j]) {
                renderer[i] = nullptr;
                break;
            }
        }
    }
    clean_renderer();
}

void Window::remove_sprites_from_renderer(function<bool(Sprite*)> condition) {
    //Sterge mai multe sprite-uri in renderer dupa functia data
    for (unsigned int i = 0; i < nr_of_sprites_in_renderer; i++) {
        if (condition(renderer[i])) {
            renderer[i] = nullptr;
            break;
        }
    }
    clean_renderer();
}

//  Da load la sprite-uri in buffer
void Window::update_buffer_from_renderer() {
    unsigned int i = 0;
    //  Sort by scene number
    sort(renderer, renderer + nr_of_sprites_in_renderer, compare);
    empty_buffer();
    POINT_e current_s;
    //  Iterates trough every sprite
    while (i < nr_of_sprites_in_renderer && renderer[i] != nullptr) {
        Sprite* current_sprite = renderer[i];
        //  If invisible, continue
        if(!current_sprite->view.visible) { i++; continue; }
        POINT_e fs = {current_sprite->frame_size.x, current_sprite->frame_size.y};
        unsigned int cr_frame = current_sprite->animation.current_frame;
        for (unsigned int h = 0; h < fs.y; h++) {
            current_s.y = h + current_sprite->coords.y;
            for (unsigned int w = 0; w < fs.x && current_s.y < WINDOWHEIGHT; w++) {
                current_s.x = w + current_sprite->coords.x;
                if (current_s.x >= WINDOWLENGTH) continue;
                if (current_sprite->view.transparent_white_spaces && current_sprite->frames[cr_frame][h][w].character == ' ') continue;
                //char clr[21] = "\x1b[38;2;<r>;<g>;<b>m<char>";
                CELL current_cell = current_sprite->frames[cr_frame][h][w];
                string clr = "\x1b[38;2;"; 
                clr += num_to_color(current_cell.color.r) + ';' + num_to_color(current_cell.color.g) + ';' + num_to_color(current_cell.color.b) + 'm' + current_cell.character;
                char clr_char[21] = {0};
                strncpy(clr_char, clr.c_str(), clr.size());
                stringcpy(buffer + ((h + current_sprite->coords.y) * (WINDOWLENGTH + 1) * 20 +  (w + current_sprite->coords.x) * 20), clr_char);
            }
        }
        i++;
    }
}

void Window::remove_sprite_from_renderer(Sprite* sprite) {
    //Sterge pointer-ul dat din renderer
    for (unsigned int i = 0; i < nr_of_sprites_in_renderer; i++)
        if (renderer[i] == sprite) {
            renderer[i] = nullptr;
            break;
        }
    clean_renderer();
}

void Window::input() {
    while (Game::running) {
        if (_kbhit()) {
            char ch = _getch();
            lock_guard<mutex> lock(Game::key_mutex);
            Game::keys_down.insert(ch);
        }
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}

void Window::gml(function<void()> game_logic) {
    //Codul game loop-ului.
    using clock = chrono::high_resolution_clock;
    using duration = chrono::duration<double, milli>;
    double frame_time = 1000.0 / Config::FPS; 
    while (Game::running) {
        auto frame_start = clock::now();
        update_buffer_from_renderer();
        game_logic();
        print_buffer();
        reset_cursor();
        auto frame_end = clock::now();
        duration elapsed = frame_end - frame_start;
        if (elapsed.count() < frame_time) 
            this_thread::sleep_for(chrono::milliseconds((int)(frame_time - elapsed.count())));
    }
}

void Window::game_loop(function<void()> game_logic) {
    //Game loop-ul propriu-zis. Primeste ca parametru o functie de tip void care se va executa la fiecare game tick
    thread input_thread(input, this);
    gml(game_logic);
    input_thread.join();
}

unordered_set<char> Window::get_keys_pressed() {
    return Game::keys_down;
}

void Window::empty_keys_pressed() {
    Game::keys_down.clear();
}
