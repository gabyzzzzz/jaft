#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"
#include "../third_party/json.hpp"

using json = nlohmann::json;
ofstream lg("log.txt", ios::app);

namespace Game {
    bool running = true;
    queue<char> key_pressed;
    mutex key_mutex;

    void push_key(char k) {
        lock_guard<mutex> lock(key_mutex);
        if (key_pressed.size() > 7) {
            key_pressed.pop();  
        }
        key_pressed.push(k);
    }

    bool pop_key(char &k) {
        lock_guard<mutex> lock(key_mutex);
        if (key_pressed.empty()) return false;
        k = key_pressed.front();
        key_pressed.pop();
        return true;
    }
}

namespace Config {
    int FPS;
};

bool compare(Sprite* s1, Sprite* s2) {
    return (s1->stage < s2->stage);
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
    screen_height = GetSystemMetrics(SM_CYFULLSCREEN); screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
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
    for (int i = 0; i < WINDOWHEIGHT; i++)
        for (int j = 0; j < WINDOWLENGTH; j++)
            buffer[i][j] = ' ';
}

void Window::DEBUG_fill() {
    for (int i = 0; i < WINDOWHEIGHT; i++) {
        for (int j = 0; j < WINDOWLENGTH; j++) cout << '*';
        if (i < WINDOWHEIGHT - 1) cout << '\n';
    }
    cout.flush();
}

void Window::print_buffer() {
    for (int i = 0; i < WINDOWHEIGHT; i++) {
        string to_print;
        for (int j = 0; j < WINDOWLENGTH; j++) to_print += buffer[i][j];
        cout << to_print;
        if (i < WINDOWHEIGHT - 1) cout << '\n';
    }
    cout.flush();
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
    unsigned int font_h = round((double) screen_height / FONT_RATIO_HEIGHT);
    unsigned int font_w = round((double) screen_width / FONT_RATIO_LENGTH);
    set_font_settings(font_h, font_w);
    hide_console_cursor();
    maximize_console();
    disable_console_resize();
    disable_console_scroll();
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
    font_size_height = f_height;
    font_size_width = f_width;
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

void Window::update_buffer_from_renderer() {
    //Da load la sprite-uri in buffer
    unsigned int i = 0;
    sort(renderer, renderer + nr_of_sprites_in_renderer, compare);
    empty_buffer();
    while (i < nr_of_sprites_in_renderer && renderer[i] != nullptr) {
        Sprite* current_sprite = renderer[i];
        if(current_sprite->visible) {
            unsigned int fh = current_sprite->frame_height;
            unsigned int fw = current_sprite->frame_width;
            for (unsigned int h = 0; h < fh; h++) {
                if (!(h + current_sprite->y < WINDOWLENGTH)) break;
                for (unsigned int w = 0; w < fw; w++) {
                    if (!(w + current_sprite->x < WINDOWLENGTH)) break;
                    unsigned int cr_frame = current_sprite->current_frame;
                    buffer[h + current_sprite->y][w + current_sprite->x] = current_sprite->sprite_frames[cr_frame][h][w];
                }
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

void Window::reset_cursor() {
    //Config
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = { 0, 0 };
    SetConsoleCursorPosition(hConsole, topLeft);
}

void Window::disable_console_scroll() {
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

void Window::disable_console_resize() {
    //Config
    HWND console = GetConsoleWindow();
    if (!console) return;
    LONG style = GetWindowLong(console, GWL_STYLE);
    style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
    SetWindowLong(console, GWL_STYLE, style);
    SetWindowPos(console, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

void Window::hide_console_cursor() {
    //Config
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; 
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}


void Window::maximize_console() {
    //Config
    HWND hwnd = GetConsoleWindow();
    SetForegroundWindow(hwnd); 
    keybd_event(VK_F11, 0, 0, 0);
    keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
    fix_console_size();
    Sleep(100);
}

void Window::input() {
    //Codul pentru input-handeling. Foloseste game_loop pentru multithreading (recomandat)
     while (Game::running) {
        if (_kbhit()) {
            char ch = _getch(); 
            if (ch == 27) { Game::running = false; return; }
            Game::push_key(ch);
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void Window::gml(function<void()> game_logic) {
    //Codul game loop-ului. Foloseste game_loop pentru multithreading (recomandat)
    using clock = chrono::high_resolution_clock;
    using duration = chrono::duration<double, milli>;
    double frame_time = 1000.0 / Config::FPS; 
    while (Game::running) {
        auto frame_start = clock::now();
        game_logic();
        update_buffer_from_renderer();
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
    thread game_thread(gml, this, game_logic);
    thread input_thread(input, this);
    input_thread.join();
    game_thread.join();
}

bool Window::get_key_pressed(char& ch) {
    char key;
    if (!(Game::pop_key(key))) return false;
    ch = key;
    return true;
}

void Window::fix_console_size() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD newSize;
    newSize.X = 200;
    newSize.Y = 100; 
    SetConsoleScreenBufferSize(hConsole, newSize);
    SMALL_RECT winSize = { 0, 0, static_cast<SHORT>(newSize.X - 1), static_cast<SHORT>(newSize.Y - 1) };
    SetConsoleWindowInfo(hConsole, TRUE, &winSize);
}