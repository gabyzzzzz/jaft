#pragma once

#include "defines.h"
#include "libraries.h"

void log(int err_code, int sprite_label = 0);

class Sprite
{
public:
    bool visible = true, is_animation_active = true;
    unsigned int x = 0, y = 0;
    unsigned int frame_height = 0, frame_width = 0;
    unsigned int stage = 0;
    unsigned int nr_of_frames = 0, current_frame = 0, ticks_per_frame = 1, current_tick = 0;
    unsigned int label = 0;
    char*** sprite_frames = nullptr;

    void init_by_file(const char file_name[]);
    void DEBUG_sprite();
    ~Sprite();
};


class Scene{
public:
    unsigned int nr_of_sprites;
    Sprite** sprites;

    void delete_sprites();
    Scene& operator=(const Scene& obj1);
    void DEBUG_scene();
    void clean();
    void hide_scene();
    void show_scene();
    void add_sprite(Sprite* sprite);
    void remove_sprite(Sprite* sprite);
    void remove_sprite(unsigned int lbl);
    void set_nr_of_sprites(unsigned int nr);
    void init_by_file(const char file_name[]);
    ~Scene();
};


class Window
{
public:
    unsigned int screen_height = GetSystemMetrics(SM_CYFULLSCREEN), screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
    unsigned int nr_of_sprites_in_renderer = 0;
    double font_size_height, font_size_width;
    char buffer[WINDOWHEIGHT][WINDOWLENGTH];
    Sprite* renderer[MAXNROFSPRITES];
    void clean_renderer();
    void update_buffer_from_renderer();
    void add_sprite_to_renderer(Sprite* s1);
    void add_sprites_to_renderer(Sprite** s1, unsigned int sz);
    void delete_sprites_in_renderer(Sprite** s1, unsigned int sz);
    void delete_sprites_in_renderer(unsigned int lbl);
    void empty_renderer();
    void print_buffer();
    void empty_buffer();
    void DEBUG_fill();
    void set_font_settings(unsigned int f_height, unsigned int f_width);
    void config();
    Window();
};
