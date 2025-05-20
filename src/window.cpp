#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"
#include "../third_party/json.hpp"

using json = nlohmann::json;
ofstream lg("log.txt", ios::app);

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
        cout << '\n';
    }
}

void Window::print_buffer() {
    for (int i = 0; i < WINDOWHEIGHT; i++) {
        for (int j = 0; j < WINDOWLENGTH; j++) cout << buffer[i][j];
        cout << '\n';
    }
}

Window::Window() {
    //Constructor for window
    if (!(lg.is_open())) {
        cout << 999;
        cin.get();
        exit(999);
    }
    config();
    ios_base::sync_with_stdio(false);
    empty_buffer();
    unsigned int font_h = round((double) screen_height / FONT_RATIO_HEIGHT);
    unsigned int font_w = round((double) screen_width / FONT_RATIO_LENGTH);
    set_font_settings(font_h, font_w);
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
    for (unsigned int i = 0; i < MAXNROFSPRITES; i++)
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
    unsigned int i = nr_of_sprites_in_renderer;
    if (i == MAXNROFSPRITES) log(809);
    for (unsigned int j = i; j < sz + i; j++) {
        if (!s1[j]) log(802);
        if (j >= MAXNROFSPRITES) log(809, s1[j - i]->label);
        renderer[j] = s1[j - i];
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

void Window::remove_sprites_from_renderer(unsigned int lbl) {
    //Sterge mai multe sprite-uri in renderer dupa label
    for (unsigned int i = 0; i < nr_of_sprites_in_renderer; i++) {
        if (renderer[i]->label == lbl) {
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