#include "pch.h"
#include "jaft.h"

/*

THIS IS THE SOURCE CODE OF THE JAFT LIBRARY, VERSION 1.0.0, 3TH JULY 2025
MADE BY G@BY

JAFT IS A LIGHTWEIGHT LIBRARY INTENDED TO BE USED FOR C++ CONSOLE GAMES OR APpLICATIONS THAT USE TEXT BASED USER INTERFACES.
IT WORKS WITH CUSTOM SPRITE OBJECTS THAT CANNOT BE CREATED WITHOUT PROPER SOFTWARE.

JAFT IS NOT CROSS-PLATFORM, IT WORKS ONLY ON WINDOWS SYSTEMS.
JAFT WAS MADE USING ISO C++14 STANDARD, AND IT IS NOT COMPATIBLE WITH EARLIER VERSIONS.

DO NOT DISTRIBUTE THIS LIBRARY WITHOUT PERMISSION FROM THE AUTHOR.

*/

void jaft::Sprite::alocate_memory() {
    try {

        renderer.value = new char* [animation.nr_of_frames];
        for (unsigned int i = 0; i < animation.nr_of_frames; i++) renderer.value[i] = new char[BUFFERMULTIPLIER * frame_size.x * frame_size.y + 2];
        renderer.size = new size_t[animation.nr_of_frames];
        for (unsigned int i = 0; i < animation.nr_of_frames; i++) renderer.size[i] = 0;
        renderer.colored_chunks.container = new SBIT * *[renderer.nr_of_colors];
        for (unsigned int i = 0; i < renderer.nr_of_colors; i++) {
            renderer.colored_chunks.container[i] = new SBIT * [animation.nr_of_frames];
            for (unsigned int j = 0; j < animation.nr_of_frames; j++) renderer.colored_chunks.container[i][j] = new SBIT[frame_size.x * frame_size.y + 2];
        }
        renderer.colored_chunks.size = new size_t * [renderer.nr_of_colors];
        for (unsigned int i = 0; i < renderer.nr_of_colors; i++) renderer.colored_chunks.size[i] = new size_t[animation.nr_of_frames];
        for (unsigned int i = 0; i < renderer.nr_of_colors; i++) {
            for (unsigned int j = 0; j < animation.nr_of_frames; j++) {
                renderer.colored_chunks.size[i][j] = 0;
            }
        }
        renderer.cursor_hops.size = new size_t[animation.nr_of_frames];
        for (unsigned int i = 0; i < animation.nr_of_frames; i++) renderer.cursor_hops.size[i] = 0;
        renderer.cursor_hops.indexes = new int* [animation.nr_of_frames];
        renderer.cursor_hops.values = new POINT_e * [animation.nr_of_frames];
        for (unsigned int i = 0; i < animation.nr_of_frames; i++) {
            renderer.cursor_hops.indexes[i] = new int[frame_size.x * frame_size.y + 2];
            renderer.cursor_hops.values[i] = new POINT_e[frame_size.x * frame_size.y + 2];
        }

    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }

    previous_pos = { {1, 1}, {0, 0} };
    refresh();
}

void jaft::Sprite::init_by_file(const char file_name[]) {
    try {

        std::ifstream in(file_name);
        if (!in.is_open()) log(401, label);
        if (!(in >> frame_size.y >> frame_size.x >> animation.nr_of_frames >> renderer.nr_of_colors)) log(402, label);
        if (frame_size.y < 1 || frame_size.x < 1 || animation.nr_of_frames < 1 || renderer.nr_of_colors < 1) log(403, label);
        renderer.pallete = new COLOR[renderer.nr_of_colors];
        for (int i = 0; i < renderer.nr_of_colors; i++) in >> renderer.pallete[i].r >> renderer.pallete[i].g >> renderer.pallete[i].b;
        unsigned int nrf = animation.nr_of_frames;
        frames = new CELL * *[nrf];
        for (unsigned int f = 0; f < nrf; f++) {
            frames[f] = new CELL * [frame_size.y + 1];
            for (unsigned int h = 0; h < frame_size.y; h++) {
                frames[f][h] = new CELL[frame_size.x + 1];
                for (unsigned int w = 0; w < frame_size.x; w++) {
                    int ch;
                    in >> frames[f][h][w].color_id >> ch;
                    frames[f][h][w].character = (char)ch;
                }
            }
        }

    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }

    alocate_memory();
}

void jaft::Sprite::sprite_init() {
    try {

        unsigned int nfr = animation.nr_of_frames;
        POINT_e fsz = frame_size;
        frames = new CELL * *[nfr];
        renderer.pallete = new COLOR[renderer.nr_of_colors];
        for (int i = 0; i < renderer.nr_of_colors; i++) {
            renderer.pallete[i] = { 255, 0, 0 };
        }
        for (unsigned int f = 0; f < nfr; f++) {
            frames[f] = new CELL * [fsz.y + 1];
            for (unsigned int h = 0; h < fsz.y; h++) {
                frames[f][h] = new CELL[fsz.x + 1];
                for (unsigned int w = 0; w < fsz.x; w++) frames[f][h][w] = { 0, ' ' };
            }
        }

    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }

    alocate_memory();
}

jaft::Sprite::~Sprite() {
    unsigned int nfr = animation.nr_of_frames;
    POINT_e fsz = frame_size;

    for (unsigned int f = 0; f < nfr; ++f) {
        for (unsigned int y = 0; y < fsz.y; ++y)
            delete[] frames[f][y];
        delete[] frames[f];
    }
    delete[] frames;

    for (unsigned int i = 0; i < renderer.nr_of_colors; i++) {
        for (unsigned int j = 0; j < animation.nr_of_frames; j++)
            delete[] renderer.colored_chunks.container[i][j];
        delete[] renderer.colored_chunks.size[i];
    }
    delete[] renderer.colored_chunks.container;
    delete[] renderer.colored_chunks.size;

    for (unsigned int i = 0; i < animation.nr_of_frames; i++) {
        delete[] renderer.value[i];
        delete[] renderer.cursor_hops.indexes[i];
        delete[] renderer.cursor_hops.values[i];
    }
    delete[] renderer.value;
    delete[] renderer.size;
    delete[] renderer.cursor_hops.values;
    delete[] renderer.cursor_hops.indexes;
    delete[] renderer.cursor_hops.size;

    delete[] renderer.pallete;
}

jaft::Sprite::Sprite() = default;

jaft::Sprite::Sprite(const char file_name[], unsigned int lbl) : label(lbl) {
    init_by_file(file_name);
}

jaft::Sprite::Sprite(unsigned int lbl) : label(lbl) {}

void jaft::Sprite::DEBUG_sprite() {
    std::cout << "[DEBUG] Debugging sprite with label " << label << '\n';
    std::cout << frame_size.y << ' ' << frame_size.x << ' ' << animation.nr_of_frames << "\n\n";
    for (unsigned int f = 0; f < animation.nr_of_frames; f++) {
        for (unsigned int h = 0; h < frame_size.y; h++) {
            for (unsigned int w = 0; w < frame_size.x; w++) std::cout << frames[f][h][w].character;
            std::cout << '\n';
        }
        std::cout << '\n';
    }
    std::cout.flush();
}

void jaft::Sprite::DEBUG_render_code() {
    std::cout << "[DEBUG] Render code:\n";
    for (int i = 0; i < renderer.size[animation.current_frame]; i++) {
        if (renderer.value[0][i] == '\x1b') {
            std::cout << "ESC";
        }
        else std::cout << renderer.value[0][i];
    }
    std::cout << "\n[DEBUG] Cursor hops:\n";
    for (int i = 0; i < renderer.cursor_hops.size[animation.current_frame]; i++) std::cout << renderer.cursor_hops.indexes[animation.current_frame][i] << ' ';
    std::cout << '\n';
    std::cout.flush();
}

jaft::POINT_e jaft::Sprite::get_coords() const {
    return coords;
}

void jaft::Sprite::set_coords(int x, int y) {
    int x_ = x + frame_size.x;
    int y_ = y + frame_size.y;
    if ((x_ < WINDOWLENGTH && x_ >= 0) && (y_ < WINDOWHEIGHT && y >= 0)) {
        coords.x = x;
        coords.y = y;
    }
    else {
        if (x_ >= WINDOWLENGTH) coords.x = WINDOWLENGTH - frame_size.x;
        if (x_ < 0) coords.x = 0;
        if (y_ >= WINDOWHEIGHT) coords.y = WINDOWHEIGHT - frame_size.y;
        if (y_ < 0) coords.y = 0;
    }
    status.coord_changed = true;
}

void jaft::Sprite::add_x(int x) {
    int x_ = coords.x + x + frame_size.x;
    if (x_ < WINDOWLENGTH && x_ >= 0) coords.x = x + coords.x;
    else {
        if (x_ >= WINDOWLENGTH) coords.x = WINDOWLENGTH - frame_size.x;
        if (x_ < 0) coords.x = 0;
    }
    status.coord_changed = true;
}

void jaft::Sprite::add_y(int y) {
    int y_ = coords.y + y + frame_size.y;
    if (y_ < WINDOWHEIGHT && y_ >= 0) coords.y = y + coords.y;
    else {
        if (y_ >= WINDOWHEIGHT) coords.y = WINDOWHEIGHT - frame_size.y;
        if (y_ < 0) coords.y = 0;
    }
    status.coord_changed = true;
}

jaft::ANIMATION jaft::Sprite::get_animation() const {
    return animation;
}

void jaft::Sprite::start_animation() {
    animation.is_animation_active = true;
}

void jaft::Sprite::stop_animation() {
    animation.is_animation_active = false;
}

void jaft::Sprite::set_current_frame(int frame) {
    animation.current_frame = frame;
    status.modified = true;
}

void jaft::Sprite::set_ticks_per_frame(int ticks_per_frame) {
    animation.ticks_per_frame = ticks_per_frame;
}

void jaft::Sprite::set_current_tick(int current_tick) {
    animation.current_tick = current_tick;
}

jaft::VIEW jaft::Sprite::get_view() const {
    return view;
}

void jaft::Sprite::hide() {
    status.modified = true;
    view.visible = false;
}

void jaft::Sprite::show() {
    status.modified = true;
    view.visible = true;
}

void jaft::Sprite::transparent_space(bool condition) {
    view.transparent_white_spaces = condition;
    refresh();
}

void jaft::Sprite::set_stage(int z_index) {
    view.stage = z_index;
}

jaft::SPRITESTATUS& jaft::Sprite::get_status() {
    return status;
}

void jaft::Sprite::set_nr_of_frames(int frames) {
    animation.nr_of_frames = frames;
}

jaft::SRENDERER& jaft::Sprite::get_srenderer() {
    return renderer;
}

void jaft::Sprite::color_to_string(jaft::COLOR clr, char* target) {
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

void jaft::Sprite::refresh_render_code(COLOR pallete_[], int target_frame)
{
    char ansi_clr[] = "\x1b[38;2;";
    bool consecutive, different_lines;
    renderer.size[target_frame] = 0;
    renderer.cursor_hops.size[target_frame] = 0;
    refresh_colored_chunks(target_frame);
    for (int c = 0; c < renderer.nr_of_colors; c++) {
        if (renderer.colored_chunks.size[c][target_frame] > 0) {
            memcpy(renderer.value[target_frame] + renderer.size[target_frame], ansi_clr, 7);
            renderer.size[target_frame] += 7;
            color_to_string(pallete_[c], renderer.value[target_frame] + renderer.size[target_frame]);
            renderer.size[target_frame] += 12;
            cursor_hop(
                target_frame,
                renderer.colored_chunks.container[c][target_frame][0].coords.x,
                renderer.colored_chunks.container[c][target_frame][0].coords.y
            );
            renderer.value[target_frame][renderer.size[target_frame]++] = renderer.colored_chunks.container[c][target_frame][0].character;
        }
        for (int i = 1; i < renderer.colored_chunks.size[c][target_frame]; i++) {
            consecutive = renderer.colored_chunks.container[c][target_frame][i - 1].coords.x + 1 == renderer.colored_chunks.container[c][target_frame][i].coords.x;
            different_lines = renderer.colored_chunks.container[c][target_frame][i - 1].coords.y != renderer.colored_chunks.container[c][target_frame][i].coords.y;
            if (!consecutive || different_lines)
                cursor_hop(
                    target_frame,
                    renderer.colored_chunks.container[c][target_frame][i].coords.x,
                    renderer.colored_chunks.container[c][target_frame][i].coords.y
                );
            renderer.value[target_frame][renderer.size[target_frame]++] = renderer.colored_chunks.container[c][target_frame][i].character;
        }
    }
}

bool jaft::Sprite::is_colliding(const jaft::Sprite& sprite) const {
    if (coords.x + frame_size.x - 1 < sprite.coords.x) return false;
    if (sprite.coords.x + sprite.frame_size.x - 1 < coords.x) return false;
    if (coords.y + frame_size.y - 1 < sprite.coords.y) return false;
    if (sprite.coords.y + sprite.frame_size.y - 1 < coords.y) return false;
    return true;
}

bool jaft::Sprite::is_colliding(const jaft::Sprite& sprite, const char characters[], unsigned int sz) const {
    if (!is_colliding(sprite)) return false;
    if (sz < 1) log(702, label);
    if (!characters) log(702, label);
    unsigned int x_start = max(coords.x, sprite.coords.x);
    unsigned int x_end = min(coords.x + frame_size.x, sprite.coords.x + sprite.frame_size.x);
    unsigned int y_start = max(coords.y, sprite.coords.y);
    unsigned int y_end = min(coords.y + frame_size.y, sprite.coords.y + sprite.frame_size.y);
    for (unsigned int i = y_start; i < y_end; i++) {
        for (unsigned int j = x_start; j < x_end; j++) {
            char current_pixel = sprite.frames[sprite.animation.current_frame][i - sprite.coords.y][j - sprite.coords.x].character;
            bool found = false;
            for (unsigned int k = 0; k < sz; k++) {
                if (current_pixel == characters[k]) {
                    found = true;
                    break;
                }
            }
            if (!found) return true;
        }
    }
    return false;
}

void jaft::Sprite::next_game_tick() {
    animation.current_tick++;
    if (animation.current_tick >= animation.ticks_per_frame) {
        animation.current_tick = 0;
        animation.current_frame++;
        if (animation.current_frame >= animation.nr_of_frames) animation.current_frame = 0;
    }
}

inline void jaft::Sprite::refresh_colored_chunks(int target_frame) {
    CELL current_cell;
    POINT_e current_coords;
    for (int i = 0; i < renderer.nr_of_colors; i++)
        renderer.colored_chunks.size[i][target_frame] = 0;
    for (unsigned int y = 0; y < frame_size.y; y++) {
        for (unsigned int x = 0; x < frame_size.x; x++) {
            current_cell = frames[target_frame][y][x];
            if (current_cell.character != ' ' || view.transparent_white_spaces == false) {
                current_coords = { x, y };
                renderer.colored_chunks.container[current_cell.color_id][target_frame][renderer.colored_chunks.size[current_cell.color_id][target_frame]++] = {
                    current_coords,
                    current_cell.character
                };
            }
        }
    }
}

inline void jaft::Sprite::cursor_hop(int target_frame, unsigned int x, unsigned int y) {
    renderer.cursor_hops.indexes[target_frame][renderer.cursor_hops.size[target_frame]] = renderer.size[target_frame] + 2;
    renderer.cursor_hops.values[target_frame][renderer.cursor_hops.size[target_frame]] = { x, y };
    renderer.cursor_hops.size[target_frame]++;
    memcpy(renderer.value[target_frame] + renderer.size[target_frame], cursor_pos_placeholder, 9);
    renderer.size[target_frame] += 9;
}

void jaft::Sprite::refresh() {
    for (unsigned int i = 0; i < animation.nr_of_frames; i++)
        refresh_render_code(renderer.pallete, i);
    status.modified = true;
    status.coord_changed = true;
}

void jaft::Scene::clean() {
    unsigned int rmv = 0;
    unsigned int next_valid = 0;
    for (unsigned int i = 0; i < nr_of_sprites; i++) {
        if (sprites[i] != nullptr) {
            if (i != next_valid) {
                sprites[next_valid] = sprites[i];
                sprites[i] = nullptr;
            }
            next_valid++;
        }
        else rmv++;
    }
    nr_of_sprites -= rmv;
}

void jaft::Scene::hide_scene() {
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        sprites[i]->hide();
}

void jaft::Scene::show_scene() {
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        sprites[i]->show();
}

void jaft::Scene::add_sprite(Sprite* sprite) {
    if (MAXNROFSPRITES <= nr_of_sprites + 1)
        log(309, sprite->label);
    sprites[nr_of_sprites] = sprite;
    nr_of_sprites++;
}

void jaft::Scene::add_sprites(Sprite** s1, unsigned int sz) {
    if (sz < 1) log(802);
    if (!s1) log(802);
    if (MAXNROFSPRITES <= nr_of_sprites + sz)
        log(309, s1[0]->label);
    for (unsigned int i = 0; i < sz; i++) {
        if (!s1[i]) log(802);
        sprites[nr_of_sprites + i] = s1[i];
    }
    nr_of_sprites += sz;
}

void jaft::Scene::remove_sprites(Sprite** s1, unsigned int sz) {
    if (sz < 1) log(802);
    if (!s1) log(802);
    for (unsigned int i = 0; i < nr_of_sprites; i++) {
        for (unsigned int j = 0; j < sz; j++) {
            if (sprites[i] == s1[j]) {
                sprites[i] = nullptr;
                break;
            }
        }
    }
    clean();
}

void jaft::Scene::init_by_file(const char file_name[]) {
    std::ifstream in(file_name);
    if (!in.is_open()) log(301);
    if (!(in >> nr_of_sprites)) log(302);
    sprites = new Sprite * [nr_of_sprites];
    for (unsigned int i = 0; i < nr_of_sprites; i++) {
        char temp[100];
        if (!(in >> temp)) log(302);
        sprites[i] = new Sprite;
        sprites[i]->init_by_file(temp);
    }
}

void jaft::Scene::set_nr_of_sprites(unsigned int nr) {
    if (nr_of_sprites) {
        for (unsigned int i = 0; i < nr_of_sprites; i++) sprites[i] = nullptr;
        delete[] sprites;
        sprites = nullptr;
    }
    sprites = new Sprite * [nr];
    for (unsigned int i = 0; i < nr; i++) sprites[i] = nullptr;
    nr_of_sprites = nr;
}

void jaft::Scene::remove_sprite(Sprite* sprite) {
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        if (sprites[i] == sprite) {
            sprites[i] = nullptr;
            break;
        }
    clean();
}

void jaft::Scene::remove_sprites(std::function<bool(Sprite*)> condition) {
    unsigned int rmv = 0;
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        if (condition(sprites[i]))
            sprites[i] = nullptr;
    clean();
}

jaft::Scene::~Scene() {
    if (sprites) {
        for (unsigned int i = 0; i < nr_of_sprites; i++) sprites[i] = nullptr;
        delete[] sprites;
        sprites = nullptr;
    }
}

void jaft::Scene::remove_all_sprites() {
    if (nr_of_sprites) {
        for (unsigned int i = 0; i < nr_of_sprites; i++) {
            delete sprites[i];
        }
        delete[] sprites;
        sprites = nullptr;
        nr_of_sprites = 0;
    }
}

jaft::Scene& jaft::Scene::operator=(const Scene& obj1) {
    if (this == &obj1) return *this;
    set_nr_of_sprites(obj1.nr_of_sprites);
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        sprites[i] = obj1.sprites[i];
    return *this;
}

void jaft::Scene::DEBUG_scene() {
    for (unsigned int i = 0; i < nr_of_sprites; i++) {
        if (sprites[i] == nullptr) std::cout << "nullptr\n\n";
        else {
            sprites[i]->DEBUG_sprite();
            std::cout << '\n';
        }
    }
}

jaft::Scene::Scene(const char file_name[]) {
    init_by_file(file_name);
}

jaft::Scene::Scene() = default;

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

void jaft::Window::init_clear_buffer() {
    for (int i = 0; i < WINDOWLENGTH * WINDOWHEIGHT; ++i) {
        clear_buffer[i].Char.AsciiChar = ' ';
        clear_buffer[i].Attributes = 0;
    }
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
    init_clear_buffer();
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

inline void jaft::Window::clear_rectangle(POINT_e upper_left, POINT_e lower_right) {
    width = lower_right.x - upper_left.x + 1;
    height = lower_right.y - upper_left.y + 1;
    if (width <= 0 || height <= 0) return;
    write_region = {
        static_cast<SHORT>(upper_left.x),
        static_cast<SHORT>(upper_left.y),
        static_cast<SHORT>(lower_right.x),
        static_cast<SHORT>(lower_right.y)
    };
    buf_size = { width, height };
    WriteConsoleOutput(
        h_out,
        clear_buffer,
        buf_size,
        { 0, 0 },
        &write_region
    );
}

inline void jaft::Window::clear_screen() {
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

inline void jaft::Window::render_all(int c_index, int traversed) {
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

inline void jaft::Window::update_buffer_from_renderer() {
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
        sprite->previous_pos = { sprite->get_coords(), temp_lower_right };
        traversed += sprite->renderer.size[sprite->get_animation().current_frame];
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
        Sleep(1);
    }
}

void jaft::Window::gml(std::function<void()> game_logic) {
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

void jaft::Window::game_loop(std::function<void()> game_logic) {
    std::thread input_thread([this]() { this->input(); });
    input_thread.detach();
    gml(game_logic);
}

std::unordered_set<char> jaft::Window::get_keys() {
    std::lock_guard<std::mutex> lock(Game::key_mutex);
    return Game::keys_down;
}

void jaft::Window::empty_keys() {
    Game::keys_down.clear();
}