#include "libraries.h"
#include "../lib/jaft.h"

std::ofstream lg("log.txt", std::ios::app);

namespace Game {
    bool running = true;
    std::unordered_set<char> keys_down;
    std::mutex key_mutex;
}

namespace Config {
    int FPS;
};

bool compare(jaft::Sprite* s1, jaft::Sprite* s2) {
    return (s1->get_view().stage < s2->get_view().stage);
}

HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE h_in = GetStdHandle(STD_INPUT_HANDLE);
CONSOLE_SCREEN_BUFFER_INFO csbi;
DWORD consoleSize, charsWritten;
COORD topLeft = { 0, 0 };
COORD temp_coords;
SMALL_RECT write_clear_rect;
SHORT height, width;
COORD buf_size;

static void fix_console_size() {
    COORD newSize;
    newSize.X = 200;
    newSize.Y = 100;
    SetConsoleScreenBufferSize(h_out, newSize);
    SMALL_RECT winSize = { 0, 0, static_cast<SHORT>(newSize.X - 1), static_cast<SHORT>(newSize.Y - 1) };
    SetConsoleWindowInfo(h_out, TRUE, &winSize);
}

static void fix_zoom() {
    DWORD mode;
    GetConsoleMode(h_out, &mode);
    mode &= ~ENABLE_MOUSE_INPUT;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(h_out, mode);
}

static void disable_text_selection() {
    DWORD mode = 0;
    GetConsoleMode(h_in, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT;
    SetConsoleMode(h_in, mode);
}

static void enable_virtual_terminal() {
    DWORD dwMode = 0;
    GetConsoleMode(h_out, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(h_out, dwMode);
}

static void disable_console_scroll() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h_out, &csbi);
    COORD newSize = {
        static_cast<SHORT>(csbi.srWindow.Right - csbi.srWindow.Left + 1),
        static_cast<SHORT>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
    };
    SetConsoleScreenBufferSize(h_out, newSize);
}

static void disable_console_resize() {
    HWND console = GetConsoleWindow();
    if (!console) return;
    LONG style = GetWindowLong(console, GWL_STYLE);
    style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
    SetWindowLong(console, GWL_STYLE, style);
    SetWindowPos(console, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

static void hide_console_cursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(h_out, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(h_out, &cursorInfo);
}

static void maximize_console() {
    HWND hwnd = GetConsoleWindow();
    SetForegroundWindow(hwnd);
    keybd_event(VK_F11, 0, 0, 0);
    keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
    fix_console_size();
    Sleep(100);
}

void jaft::Window::init_clear_array() {
    for (int i = 0; i < WINDOWLENGTH; i++) clear_array[i] = ' ';
}

void jaft::log(int err_code, int sprite_label) {
    lg << "[CONSOLE] Program exited with error code: " << err_code << '\n';
    lg << "[CONSOLE] Sprite id: " << sprite_label << '\n';
    lg.flush();
    exit(err_code);
}

void jaft::Window::DEBUG_fill() {
    for (int i = 0; i < WINDOWHEIGHT; i++) {
        for (int j = 0; j < WINDOWLENGTH; j++) std::cout << '*';
        if (i < WINDOWHEIGHT - 1) std::cout << '\n';
    }
    std::cout.flush();
}

void jaft::Window::DEBUG_buffer() {
    std::cout << "[DEBUG] Buffer size: " << buffer.size;
    std::cout << "\n[DEBUG] Buffer value: \n";
    for (int i = 0; i < buffer.size; i++) {
        if (buffer.value[i] == '\x1b') {
            std::cout << "ESC";
        }
        else std::cout << buffer.value[i];
    }
    std::cout << '\n';
}

jaft::Window::Window() {
    if (!(lg.is_open())) {
        std::cout << 999;
        std::cin.get();
        exit(999);
    }
    for (int i = 0; i < WINDOWHEIGHT; i++) {
        for (int j = 0; j < (WINDOWLENGTH + 63) / 64; j++) {
            global_bitmask[i][j] = 0;
            temp_bitmask[i][j] = 0;
        }
    }
    std::ios_base::sync_with_stdio(false);
    screen_size.y = GetSystemMetrics(SM_CYFULLSCREEN); screen_size.x = GetSystemMetrics(SM_CXFULLSCREEN);
    unsigned int font_h = round((double)screen_size.y / FONT_RATIO_HEIGHT);
    unsigned int font_w = round((double)screen_size.x / FONT_RATIO_LENGTH);
    set_font_settings(font_h, font_w);
    maximize_console();
    disable_console_resize();
    disable_console_scroll();
    disable_text_selection();
    fix_zoom();
    enable_virtual_terminal();
    hide_console_cursor();
    init_clear_array();
    GetConsoleScreenBufferInfo(h_out, &csbi);
    consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
}

void jaft::Window::set_font_settings(unsigned int f_height, unsigned int f_width) {
    if (f_height < 1 || f_width < 1) log(200);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) log(201);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    if (!GetCurrentConsoleFontEx(hOut, TRUE, &cfi)) log(201);
    cfi.dwFontSize.X = f_width;
    cfi.dwFontSize.Y = f_height;
    wcscpy_s(cfi.FaceName, L"Cascadia Mono");
    if (!SetCurrentConsoleFontEx(hOut, TRUE, &cfi)) log(201);
    font_size.y = f_height;
    font_size.x = f_width;
}

void jaft::Window::clean_renderer() {
    unsigned int rmv = 0;
    unsigned int next_valid = 0;
    for (unsigned int i = 0; i < renderer.size; i++)
        if (renderer.container[i] != nullptr) {
            if (i != next_valid) {
                renderer.container[next_valid] = renderer.container[i];
                renderer.container[i] = nullptr;
            }
            next_valid++;
        }
        else rmv++;
    renderer.size -= rmv;
}

void jaft::Window::empty_renderer() {
    for (unsigned int i = 0; i < renderer.size; i++)
        renderer.container[i] = nullptr;
    renderer.size = 0;
}

void jaft::Window::add_sprite_to_renderer(Sprite* s1) {
    if (!s1) log(802);
    if (renderer.size == MAXNROFSPRITES) log(809, s1->label);
    renderer.container[renderer.size] = s1;
    renderer.size++;
    std::sort(renderer.container, renderer.container + renderer.size, compare);
}

void jaft::Window::add_sprites_to_renderer(Sprite** s1, unsigned int sz) {
    if (sz < 1) log(802);
    if (!s1) log(802);
    if (MAXNROFSPRITES <= renderer.size + sz)
        log(809, s1[0]->label);
    for (unsigned int j = 0; j < sz; j++) {
        if (!s1[j]) log(802);
        renderer.container[renderer.size + j] = s1[j];
    }
    renderer.size += sz;

    std::sort(renderer.container, renderer.container + renderer.size, compare);
}

void jaft::Window::remove_sprites_from_renderer(Sprite** s1, unsigned int sz) {
    if (sz < 1) log(802);
    if (!s1) log(802);
    for (unsigned int i = 0; i < renderer.size; i++) {
        for (unsigned int j = 0; j < sz; j++) {
            if (renderer.container[i] == s1[j]) {
                renderer.container[i] = nullptr;
                break;
            }
        }
    }
    clean_renderer();
    std::sort(renderer.container, renderer.container + renderer.size, compare);
}

void jaft::Window::remove_sprites_from_renderer(std::function<bool(Sprite*)> condition) {
    for (unsigned int i = 0; i < renderer.size; i++) {
        if (condition(renderer.container[i])) {
            renderer.container[i] = nullptr;
        }
    }
    clean_renderer();
    std::sort(renderer.container, renderer.container + renderer.size, compare);
}

void jaft::Window::remove_sprite_from_renderer(Sprite* sprite) {
    for (unsigned int i = 0; i < renderer.size; i++)
        if (renderer.container[i] == sprite) {
            renderer.container[i] = nullptr;
            break;
        }
    clean_renderer();
    std::sort(renderer.container, renderer.container + renderer.size, compare);
}

static void color_to_string(jaft::COLOR clr, char* target) {
    for (int i = 2; i >= 0; i--) {
        target[i] = (clr.r % 10) + '0';
        clr.r /= 10;
    }
    target[3] = ';';
    for (int i = 6; i >= 4; i--) {
        target[i] = (clr.g % 10) + '0';
        clr.g /= 10;
    }
    target[7] = ';';
    for (int i = 10; i >= 8; i--) {
        target[i] = (clr.b % 10) + '0';
        clr.b /= 10;
    }
    target[11] = 'm';
}

inline void jaft::Window::update_global_bitmask(const Sprite& sprite) {
    int x_offset = sprite.get_coords().x % 64;
    int bitmask_groups = (sprite.frame_size.x + 63) / 64;
    POINT_e sprite_coords = sprite.get_coords();
    int global_bitmask_index = sprite_coords.x / 64;
    int current_frame = sprite.get_animation().current_frame;
    for (int y = 0; y < sprite.frame_size.y; y++) {
        for (int x = 0; x < bitmask_groups; x++) {
            global_bitmask[sprite_coords.y + y][global_bitmask_index + x] |= (sprite.bitmask[current_frame][y][x] << x_offset);
            if (x_offset != 0) global_bitmask[sprite_coords.y + y][global_bitmask_index + x + 1] |= (sprite.bitmask[current_frame][y][x] >> (64 -x_offset));
        }
    }
}

inline void jaft::Window::clear_screen() {
    clear_buffer.size = 0;
    int consecutive, x_;
    int_64 get_bit;
    for (int y = 0; y < WINDOWHEIGHT; y++) {
        consecutive = 0;
        for (int x = 0; x < global_bitmask_groups; x++) {
            temp_bitmask[y][x] &= ~global_bitmask[y][x];
            get_bit = temp_bitmask[y][x];
            for (int i = 0; i < 64; i++) {
                if (get_bit & 1) {
                    if (!consecutive) {
                        memcpy(clear_buffer.value + clear_buffer.size, "\x1b[", 2);
                        clear_buffer.size += 2;
                        clear_buffer.value[clear_buffer.size++] = '0' + ((y + 1) / 10) % 10; 
                        clear_buffer.value[clear_buffer.size++] = '0' + (y + 1) % 10;
                        clear_buffer.value[clear_buffer.size++] = ';';
                        x_ = x * 64 + i + 1;
                        clear_buffer.value[clear_buffer.size++] = '0' + (x_ / 100) % 10;
                        clear_buffer.value[clear_buffer.size++] = '0' + (x_ / 10) % 10;
                        clear_buffer.value[clear_buffer.size++] = '0' + x_ % 10;
                        clear_buffer.value[clear_buffer.size++] = 'H';
                    }
                    consecutive++;
                } else if (consecutive) {
                    memcpy(clear_buffer.value + clear_buffer.size, clear_array, consecutive);
                    clear_buffer.size += consecutive;
                    consecutive = 0;
                }
                get_bit >>= 1;
            }
            temp_bitmask[y][x] = global_bitmask[y][x];
            global_bitmask[y][x] = 0;
        }
        if (consecutive > 0) {
            memcpy(clear_buffer.value + clear_buffer.size, clear_array, consecutive);
            clear_buffer.size += consecutive;
            consecutive = 0;
        }
    } 
    _write(1, clear_buffer.value, clear_buffer.size); 
    if (clear_buffer.size > 0) {
        clear_buffer.value[clear_buffer.size] = '\0';
    }
}

inline void jaft::Window::inject_cursor_coords(const Sprite& sprite, int traversed) {
    unsigned int replace, frame = sprite.get_animation().current_frame, xc, yc;
    size_t size = sprite.renderer.cursor_hops.size[frame];
    for (size_t i = 0; i < size; i++) {
        replace = traversed + sprite.renderer.cursor_hops.indexes[frame][i];
        xc = sprite.get_coords().x + sprite.renderer.cursor_hops.values[frame][i].x + 1;
        yc = sprite.get_coords().y + sprite.renderer.cursor_hops.values[frame][i].y + 1;
        buffer.value[replace] = '0' + (yc / 10) % 10;
        buffer.value[replace + 1] = '0' + yc % 10;
        buffer.value[replace + 3] = '0' + (xc / 100) % 10;
        buffer.value[replace + 4] = '0' + (xc / 10) % 10;
        buffer.value[replace + 5] = '0' + xc % 10;
    }
}

inline void jaft::Window::print_buffer() {
    _write(1, buffer.value, buffer.size);
    Sleep(FPSCAP);
}

inline void jaft::Window::render_all(int c_index, int traversed) {
    buffer.size = traversed;
    Sprite* sprite;
    while (c_index < renderer.size && (sprite = renderer.container[c_index]) != nullptr) {
        if (sprite->get_view().visible) {
            memcpy(buffer.value + buffer.size, sprite->renderer.value[sprite->get_animation().current_frame], sprite->renderer.size[sprite->get_animation().current_frame]);
            buffer.size += sprite->renderer.size[sprite->get_animation().current_frame];
        }
        sprite->get_status().modified = false;
        c_index++;
    }
}

inline void jaft::Window::update_buffer_from_renderer() {
    unsigned int index = 0, traversed = 0;
    bool rendered_all = false;
    Sprite* sprite;
    while (index < renderer.size && renderer.container[index] != nullptr) {
        sprite = renderer.container[index];
        if (sprite->get_view().visible) update_global_bitmask(*sprite);
        if (sprite->get_status().modified || !(rendered_all)) {
            render_all(index, traversed);
            rendered_all = true;
            buffer.modified = true;
        }
        if (sprite->get_view().visible) {
                if (sprite->get_status().coord_changed || rendered_all) {
                inject_cursor_coords(*sprite, traversed);
                sprite->get_status().coord_changed = false;
                buffer.modified = true;
            }
            traversed += sprite->renderer.size[sprite->get_animation().current_frame];
        }
        index++;
    }
}

void jaft::Window::input() {
    while (Game::running) {
        if (_kbhit()) {
            char ch = _getch();
            std::lock_guard<std::mutex> lock(Game::key_mutex);
            Game::keys_down.insert(ch);
        }
    }
}

void jaft::Window::gml(std::function<void()> game_logic) {
    while (Game::running) {
        game_logic();
        update_buffer_from_renderer();
        clear_screen();
        if (buffer.modified) {
            print_buffer();
            buffer.modified = false;
        }
    }
}

void jaft::Window::game_loop(std::function<void()> game_logic) {
    std::thread input_thread([this]() { this->input(); });

    gml(game_logic);

    if (input_thread.joinable()) {
        input_thread.join(); 
    }
}

std::unordered_set<char> jaft::Window::get_keys() {
    std::lock_guard<std::mutex> lock(Game::key_mutex);
    return Game::keys_down;
}

void jaft::Window::empty_keys() {
    std::lock_guard<std::mutex> lock(Game::key_mutex);
    Game::keys_down.clear();
}