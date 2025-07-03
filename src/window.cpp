#include "libraries.h"
#include "defines.h"
#include "classes.h"

//   ____________________
//  | VARIABLES / OTHERS |

std::ofstream lg("log.txt", std::ios::app);

using highres_clock = std::chrono::high_resolution_clock;
using duration = std::chrono::duration<double, std::milli>;

namespace Game {
    bool running = true;
    std::unordered_set<char> keys_down;
    std::mutex key_mutex;
}

namespace Config {
    int FPS;
};

bool compare(Sprite* s1, Sprite* s2) {
    return (s1->get_view().stage < s2->get_view().stage);
}

HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE h_in = GetStdHandle(STD_INPUT_HANDLE);
CONSOLE_SCREEN_BUFFER_INFO csbi;
DWORD consoleSize, charsWritten;
COORD topLeft = {0, 0};
COORD temp_coords;
SMALL_RECT write_clear_rect;
SHORT height, width;
COORD buf_size;

//   ______________________
//  | CONFIG / CONSTRUCTOR |

//Config
void fix_console_size() {
    COORD newSize;
    newSize.X = 200;
    newSize.Y = 100; 
    SetConsoleScreenBufferSize(h_out, newSize);
    SMALL_RECT winSize = { 0, 0, static_cast<SHORT>(newSize.X - 1), static_cast<SHORT>(newSize.Y - 1) };
    SetConsoleWindowInfo(h_out, TRUE, &winSize);
}

//Config
void fix_zoom() {
    DWORD mode;
    GetConsoleMode(h_out, &mode);
    mode &= ~ENABLE_MOUSE_INPUT;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(h_out, mode);
}

//Config
void disable_text_selection() {
    DWORD mode = 0;
    GetConsoleMode(h_in, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT;
    SetConsoleMode(h_in, mode);
}

//Config
void enable_virtual_terminal() {
    DWORD dwMode = 0;
    GetConsoleMode(h_out, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(h_out, dwMode);
}

//Config
void disable_console_scroll() {   
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h_out, &csbi);
    COORD newSize = {
        static_cast<SHORT>(csbi.srWindow.Right - csbi.srWindow.Left + 1),
        static_cast<SHORT>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1)
    };
    SetConsoleScreenBufferSize(h_out, newSize);
}

//Config
void disable_console_resize() {
    HWND console = GetConsoleWindow();
    if (!console) return;
    LONG style = GetWindowLong(console, GWL_STYLE);
    style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
    SetWindowLong(console, GWL_STYLE, style);
    SetWindowPos(console, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}

//Config
void hide_console_cursor() { 
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(h_out, &cursorInfo);
    cursorInfo.bVisible = FALSE; 
    SetConsoleCursorInfo(h_out, &cursorInfo);
}

//Config
void maximize_console() {
    HWND hwnd = GetConsoleWindow();
    SetForegroundWindow(hwnd); 
    keybd_event(VK_F11, 0, 0, 0);
    keybd_event(VK_F11, 0, KEYEVENTF_KEYUP, 0);
    fix_console_size();
    Sleep(100);
}

//Config
void Window::init_clear_buffer() {
    for (int i = 0; i < WINDOWLENGTH * WINDOWHEIGHT; ++i) {
        clear_buffer[i].Char.AsciiChar = ' ';
        clear_buffer[i].Attributes = 0; 
    }
}


//   ____________________________________
//  | DEBUG / PRINT STUFF TO THE CONSOLE |

//  Prints errors to log file and ends the program upon an error
void log(int err_code, int sprite_label) {
    lg << "[CONSOLE] Program exited with error code: " << err_code << '\n';
    lg << "[CONSOLE] Sprite id: " << sprite_label << '\n';
    lg.flush();
    exit(err_code);
}

void Window::DEBUG_fill() {
    for (int i = 0; i < WINDOWHEIGHT; i++) {
        for (int j = 0; j < WINDOWLENGTH; j++) std::cout << '*';
        if (i < WINDOWHEIGHT - 1) std::cout << '\n';
    }
    std::cout.flush();
}

void Window::DEBUG_buffer() {
    std::cout << "[DEBUG] Buffer size: " << buffer.size;
    std::cout << "\n[DEBUG] Buffer value: \n";
    for (int i = 0; i < buffer.size; i++) {
        if (buffer.value[i] == '\x1b') {
            std::cout << "ESC";
        } else std::cout << buffer.value[i];
    }
    std::cout << '\n';
}

//  Constructor for window
Window::Window() {
    if (!(lg.is_open())) {
        std::cout << 999;
        std::cin.get();
        exit(999);
    }
    std::ios_base::sync_with_stdio(false);
    screen_size.y = GetSystemMetrics(SM_CYFULLSCREEN); screen_size.x = GetSystemMetrics(SM_CXFULLSCREEN);
    unsigned int font_h = round((double) screen_size.y / FONT_RATIO_HEIGHT);
    unsigned int font_w = round((double) screen_size.x / FONT_RATIO_LENGTH);
    set_font_settings(font_h, font_w);
    maximize_console();
    disable_console_resize();
    disable_console_scroll();
    disable_text_selection();
    fix_zoom();
    enable_virtual_terminal();
    hide_console_cursor();
    init_clear_buffer();
    GetConsoleScreenBufferInfo(h_out, &csbi);
    consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
}

//  Sets font size
void Window::set_font_settings(unsigned int f_height, unsigned int f_width) {
    //  See windows documentation if you wanna understand whatever is this
    if (f_height < 1 || f_width < 1) log(200);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) log(201);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    if (!GetCurrentConsoleFontEx(hOut, TRUE, &cfi)) log(201);
    cfi.dwFontSize.X = f_width;
    cfi.dwFontSize.Y = f_height;
    wcscpy(cfi.FaceName, L"Cascadia Mono");
    if (!SetCurrentConsoleFontEx(hOut, TRUE, &cfi)) log(201);
    font_size.y = f_height;
    font_size.x = f_width;
}

//   _______________________________________________________
//  | RENDERING / SHOWING TO THE SCREEN / DIVERSE FUNCTIONS |

//  Leaves no empty spaces between sprites
void Window::clean_renderer() {
    unsigned int rmv = 0;
    unsigned int next_valid = 0;
    for (unsigned int i = 0; i < renderer.size; i++)
        if (renderer.container[i] != nullptr) {
            if (i != next_valid) {
                renderer.container[next_valid] = renderer.container[i];
                renderer.container[i] = nullptr;
            }
            next_valid++;
        } else rmv++;
    renderer.size -= rmv;
}

//  Sets every pointer to nullptr
void Window::empty_renderer() {
    for (unsigned int i = 0; i < renderer.size; i++)
        renderer.container[i] = nullptr;
    renderer.size = 0;
}

//  Adds a sprite to renderer
void Window::add_sprite_to_renderer(Sprite* s1) {
    if (!s1) log(802);
    if (renderer.size == MAXNROFSPRITES) log(809, s1->label);
    renderer.container[renderer.size] = s1;
    renderer.size++;
    std::sort(renderer.container, renderer.container + renderer.size, compare);
}

//  Adds multiple sprites to renderer
void Window::add_sprites_to_renderer(Sprite** s1, unsigned int sz) {
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

//  Deletes multiple sprites from the renderer
void Window::remove_sprites_from_renderer(Sprite** s1, unsigned int sz) {
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

//  Deletes multiple sprites from the renderer by the condition given
void Window::remove_sprites_from_renderer(std::function<bool(Sprite*)> condition) {
    for (unsigned int i = 0; i < renderer.size; i++) {
        if (condition(renderer.container[i])) {
            renderer.container[i] = nullptr;
        }
    }
    clean_renderer();
    std::sort(renderer.container, renderer.container + renderer.size, compare);
}

//  Deletes the given sprite from the renderer
void Window::remove_sprite_from_renderer(Sprite* sprite) {
    for (unsigned int i = 0; i < renderer.size; i++)
        if (renderer.container[i] == sprite) {
            renderer.container[i] = nullptr;
            break;
        }
    clean_renderer();
    std::sort(renderer.container, renderer.container + renderer.size, compare);
}

//  Copies a color with the apropriate format directly into the target
void color_to_string(COLOR clr, char* target) {
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

//  Replaces placeholder values directly in the buffer
inline void Window::inject_cursor_coords(const Sprite& sprite, int traversed) {
    unsigned int replace, frame = sprite.get_animation().current_frame, xc, yc;
    size_t size = sprite.renderer.cursor_hops.size[frame];
    //  Looping through the cursor hops
    for (size_t i = 0; i < size; i++) {
        //  Calculating the index for replacement
        replace = traversed + sprite.renderer.cursor_hops.indexes[frame][i];
        //  Calculating values 
        xc = sprite.get_coords().x + sprite.renderer.cursor_hops.values[frame][i].x + 1;
        yc = sprite.get_coords().y + sprite.renderer.cursor_hops.values[frame][i].y + 1;
        //  Filling buffer with apropiate coordonates converted into chars
        //  Adding 4 because of the previous values (3) and to get rid of the ";"
        //  CHANGE - if using triple digit coords
        buffer.value[replace] = '0' + (yc / 10) % 10;
        buffer.value[replace + 1] = '0' + yc % 10;
        buffer.value[replace + 3] = '0' + (xc / 100) % 10;
        buffer.value[replace + 4] = '0' + (xc / 10) % 10;
        buffer.value[replace + 5] = '0' + xc % 10;
    }
}

//Prints buffer
inline void Window::print_buffer() {
    write(1, buffer.value, buffer.size);
    //  Caps at 60 fps to avoid flicker
    Sleep(FPSCAP);
}  

/*
//  Clears the whole screen
inline void Window::clear_screen() {
    FillConsoleOutputCharacter(h_out, ' ', consoleSize, topLeft, &charsWritten);
    // system("cls");
    // write(1, "\x1b[2J", 4);
}
*/


inline void Window::clear_rectangle(POINT_e upper_left, POINT_e lower_right) {
    width = lower_right.x - upper_left.x + 1;
    height = lower_right.y - upper_left.y + 1;  
    if (width <= 0 || height <= 0) return;
    write_region = {
        static_cast<SHORT>(upper_left.x),
        static_cast<SHORT>(upper_left.y),
        static_cast<SHORT>(lower_right.x),
        static_cast<SHORT>(lower_right.y)
    };  
    buf_size = {width, height};
    WriteConsoleOutput(
        h_out, 
        clear_buffer, 
        buf_size, 
        {0, 0}, 
        &write_region
    );
}

inline void Window::clear_screen() {
    int index = 0;
    Sprite* sprite;
    while (index < renderer.size && (sprite = renderer.container[index]) != nullptr) {
        if (sprite->get_status().coord_changed || sprite->get_status().modified) {
            if (sprite->previous_pos.upper_left.x <= sprite->previous_pos.lower_right.x) {
                clear_rectangle(
                    sprite->previous_pos.upper_left, 
                    sprite->previous_pos.lower_right
                );
            }
        }
        index++;
    }
}

//  Re-writes buffer with the new values
inline void Window::render_all(int c_index, int traversed){
    buffer.size = traversed;
    Sprite* sprite;
    while (c_index < renderer.size && (sprite = renderer.container[c_index]) != nullptr) {
        if (sprite->get_view().visible) {
            memcpy(buffer.value + buffer.size, sprite->renderer.value[sprite->get_animation().current_frame], sprite->renderer.size[sprite->get_animation().current_frame]);
            inject_cursor_coords(*sprite, buffer.size);
            buffer.size += sprite->renderer.size[sprite->get_animation().current_frame];
            sprite->get_status().coord_changed = false;
        }
        sprite->get_status().modified = false;
        c_index++;
    }
}

//  Loads sprites from renderer
inline void Window::update_buffer_from_renderer() {
    unsigned int index = 0, traversed = 0;
    bool rendered_all = false;
    Sprite* sprite;
    while (index < renderer.size && renderer.container[index] != nullptr) {
        sprite = renderer.container[index];
        if (sprite->get_status().modified && !rendered_all) {
            render_all(index, traversed);
            sprite->get_status().modified = false;
            buffer.modified = true;
            rendered_all = true;
        }
        if (sprite->get_status().coord_changed) {
            inject_cursor_coords(*sprite, traversed);
            sprite->get_status().coord_changed = false;
            buffer.modified = true;
        }   
        POINT_e temp_lower_right = {
            sprite->get_coords().x + sprite->frame_size.x - 1,
            sprite->get_coords().y + sprite->frame_size.y - 1
        };
        sprite->previous_pos = {sprite->get_coords(), temp_lower_right};
        traversed += sprite->renderer.size[sprite->get_animation().current_frame];
        index++;
    }
}

//  TODO : maybe make the input better? Meaning more responsive

//  Handle input
void Window::input() {
    while (Game::running) {
        if (_kbhit()) {
            char ch = _getch();
            std::lock_guard<std::mutex> lock(Game::key_mutex);
            Game::keys_down.insert(ch);
        }
        Sleep(1);
    }
}

//  Game loop
void Window::gml(std::function<void()> game_logic) {
    int counter = 0;
    while (Game::running) {
        game_logic();
        clear_screen();
        update_buffer_from_renderer();
        if (buffer.modified) {
            print_buffer();
            buffer.modified = false;
        }
    }
}

//  The game loop. Gets a void function that will execute every game tick
void Window::game_loop(std::function<void()> game_logic) {
    std::thread input_thread([this]() { this->input(); });
    input_thread.detach();
    gml(game_logic);
}

//  Get the keys pressed
std::unordered_set<char> Window::get_keys() {
    std::lock_guard<std::mutex> lock(Game::key_mutex);
    return Game::keys_down;
}

//  Empties the keys pressed
void Window::empty_keys() {
    Game::keys_down.clear();
}