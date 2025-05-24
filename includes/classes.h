#pragma once

#include "defines.h"
#include "libraries.h"

void log(int err_code, int sprite_label = 0);

class Sprite
{
public:
    bool visible = true, is_animation_active = true, transparent_white_spaces = true;
    unsigned int x = 0, y = 0;
    unsigned int frame_height = 0, frame_width = 0;
    unsigned int stage = 0;
    unsigned int nr_of_frames = 0, current_frame = 0, ticks_per_frame = 1, current_tick = 0;
    unsigned int label = 0;
    char*** sprite_frames = nullptr;

    void DEBUG_sprite();

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

    void DEBUG_scene();
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
public:
    unsigned int screen_height, screen_width;
    unsigned int nr_of_sprites_in_renderer = 0;
    double font_size_height, font_size_width;
    char buffer[WINDOWHEIGHT][WINDOWLENGTH];
    Sprite* renderer[MAXNROFSPRITES];

    void empty_renderer();
    void clean_renderer();
    void update_buffer_from_renderer();
    void add_sprite_to_renderer(Sprite* sprite);
    void add_sprites_to_renderer(Sprite** sprites, unsigned int sz);
    void remove_sprite_from_renderer(Sprite* sprite);
    void remove_sprites_from_renderer(Sprite** sprites, unsigned int sz);
    void remove_sprites_from_renderer(function<bool(Sprite*)> condition);

    void print_buffer();
    void empty_buffer();

    void DEBUG_fill();
    
    void disable_text_selection();
    void fix_zoom();
    void fix_console_size();
    bool get_key_pressed(char& ch);
    void gml(function<void()> game_logic);
    void game_loop(function<void()> game_logic);
    void input();
    void maximize_console();
    void hide_console_cursor();
    void disable_console_resize();
    void disable_console_scroll();
    void set_font_settings(unsigned int f_height, unsigned int f_width);
    void reset_cursor();
    void config();

    Window();
};