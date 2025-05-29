#pragma once

#include "defines.h"
#include "libraries.h"

void log(int err_code, int sprite_label = 0);

class Sprite
{
public:
    bool visible = true, is_animation_active = false, transparent_white_spaces = true;
    unsigned int x = 0, y = 0;
    unsigned int frame_height = 0, frame_width = 0;
    unsigned int stage = 0;
    unsigned int nr_of_frames = 0, current_frame = 0, ticks_per_frame = 1, current_tick = 0;
    unsigned int label = 0;
    char*** sprite_frames = nullptr;
    unsigned short int*** r = nullptr;
    unsigned short int*** g = nullptr;
    unsigned short int*** b = nullptr;

    void DEBUG_sprite(); // A bit outdated

    void next_game_tick();
    bool is_colliding(Sprite* sprite);
    bool is_colliding(Sprite* sprite, const char characters[], unsigned int sz);

    void init_by_file(const char file_name[]);
    Sprite(unsigned int lbl);
    Sprite();
    Sprite(const char file_name[], unsigned int lbl);
    ~Sprite();
};


class Scene{
public:
    unsigned int nr_of_sprites = 0;
    Sprite** sprites;

    void DEBUG_scene(); //  A bit outdated
    void clean();
    void hide_scene();
    void show_scene();

    void remove_all_sprites();
    void add_sprite(Sprite* sprite);
    void add_sprites(Sprite** sprites, unsigned int sz);
    void remove_sprite(Sprite* sprite);
    void remove_sprites(function<bool(Sprite*)> condition);
    void remove_sprites(Sprite** sprites, unsigned int sz);

    void set_nr_of_sprites(unsigned int nr);
    void init_by_file(const char file_name[]);

    Scene& operator=(const Scene& obj1);
    Scene(const char file_name[]);
    Scene();
    ~Scene();
};


class Window
{
private:
    const unsigned int total_buffer_size = WINDOWHEIGHT * (WINDOWLENGTH + 1) * 20;
public:
    unsigned int screen_height, screen_width;
    unsigned int nr_of_sprites_in_renderer = 0;
    double font_size_height, font_size_width;
    char buffer[WINDOWHEIGHT * (WINDOWLENGTH + 1) * 20];
    Sprite* renderer[MAXNROFSPRITES];

    void stringcpy(char* s1, char* s2);
    void clean_renderer();
    void empty_renderer();
    void update_buffer_from_renderer();
    void add_sprite_to_renderer(Sprite* sprite);
    void add_sprites_to_renderer(Sprite** sprites, unsigned int sz);
    void remove_sprite_from_renderer(Sprite* sprite);
    void remove_sprites_from_renderer(Sprite** sprites, unsigned int sz);
    void remove_sprites_from_renderer(function<bool(Sprite*)> condition);

    void print_buffer();
    void empty_buffer();

    void DEBUG_fill(); //   A bit outdated
    
    unordered_set<char> get_keys_pressed();
    void empty_keys_pressed();
    void gml(function<void()> game_logic);
    void game_loop(function<void()> game_logic);
    void input();
    void set_font_settings(unsigned int f_height, unsigned int f_width);
    void config();

    Window();
};