#pragma once

#include <windows.h>
#include <wincon.h>
#include <functional>
#include <unordered_set>
#include <chrono>

typedef unsigned long long int_64;

/*

JAFT LIBRARY, VERSION 0.2.0, 3TH JULY 2025
MADE BY G@BY

JAFT IS A LIGHTWEIGHT LIBRARY INTENDED TO BE USED FOR C++ CONSOLE GAMES OR APPLICATIONS THAT USE TEXT BASED USER INTERFACES.
IT WORKS WITH CUSTOM SPRITE OBJECTS THAT CANNOT BE CREATED WITHOUT PROPER SOFTWARE.

JAFT WAS MADE USING ISO C++14 STANDARD

*/

namespace jaft {

//	Constants

constexpr auto WINDOWHEIGHT = 77;
constexpr auto WINDOWLENGTH = 316;

constexpr auto FONT_RATIO_HEIGHT = 72.090909;
constexpr auto FONT_RATIO_LENGTH = 139.636363;

constexpr auto MAXNROFSPRITES = 1000;
constexpr auto BUFFERMULTIPLIER = 29;
//constexpr auto FPSCAP = 300;
constexpr auto MAXNROFCOLORS = 32;

constexpr int_64 MAXSETBIT = (1ULL << 63);
constexpr int global_bitmask_groups = (WINDOWLENGTH + 63) / 64;

void log(int err_code, int sprite_label = -1);

//	Structures

struct COLOR {
    int r, g, b;
};

struct POINT_e {
    unsigned int x = 0, y = 0;
};

struct SBIT {
    POINT_e coords;
    char character;
};

struct CELL {
    int color_id;
    char character;
};

struct VIEW {
    bool visible = true;
    bool transparent_white_spaces = true;
    unsigned int stage = 0;
};

struct ANIMATION {
    bool is_animation_active = false;
    unsigned int nr_of_frames = 0, current_frame = 0, ticks_per_frame = 1, current_tick = 0;
};

struct BUFFER {
    char value[WINDOWHEIGHT * WINDOWLENGTH * BUFFERMULTIPLIER];
    bool modified = false;
    size_t size = 0;
};

struct SPRITESTATUS {
    bool coord_changed = true, modified = false;
};

struct CURSORHOPS {
    size_t* size = nullptr;
    int** indexes = nullptr;
    POINT_e** values = nullptr;
};

struct COLOREDCHUNKS {
    SBIT*** container = nullptr;
    size_t** size = nullptr;
};

struct SRENDERER {
    size_t nr_of_colors = MAXNROFCOLORS;
    size_t* size = nullptr;
    COLOR* palette = nullptr;
    char** value = nullptr;
    COLOREDCHUNKS colored_chunks;
    CURSORHOPS cursor_hops;
};

//  Class declarations

class Sprite
{
private:
    void alocate_memory();
    inline void refresh_colored_chunks(int target_frame);
    inline void cursor_hop(int target_frame, unsigned int x, unsigned int y);
    void color_to_string(COLOR clr, char* target);
    char cursor_pos_placeholder[10] = "\x1b[01;001H";
    void refresh_render_code(COLOR palette_[], int target_frame);
    void refresh_bitmask(int target_frame);
    POINT_e coords;
    ANIMATION animation;
    VIEW view;
    SPRITESTATUS status;
public:
    SRENDERER renderer;
    POINT_e frame_size;
    unsigned int label = 0;
    CELL*** frames = nullptr;
    int_64*** bitmask = nullptr;

    void DEBUG_sprite();
    void DEBUG_render_code();

    void next_game_tick();
    bool is_colliding(const Sprite& sprite) const;
    bool is_colliding(const Sprite& sprite, const char characters[], unsigned int sz) const;
    POINT_e get_coords() const;
    void set_coords(int x, int y);
    void add_x(int delta_x);
    void add_y(int delta_y);
    ANIMATION get_animation() const;
    void start_animation();
    void stop_animation();
    void set_current_frame(int frame);
    void set_ticks_per_frame(int ticks_per_frame);
    void set_current_tick(int current_tick);
    void set_nr_of_frames(int frames);
    VIEW get_view() const;
    SPRITESTATUS& get_status();
    SRENDERER& get_srenderer();
    void hide();
    void show();
    void transparent_space(bool condition);
    void set_stage(int z_index);
    void refresh();
    void init_by_file(const char file_name[]);
    Sprite(unsigned int lbl);
    void sprite_init();
    Sprite();
    Sprite(const char file_name[], unsigned int lbl);
    ~Sprite();
};

struct RENDERER {
    Sprite* container[MAXNROFSPRITES];
    size_t size = 0;
};

class Scene {
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
    void remove_sprites(std::function<bool(Sprite*)> condition);
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
    char clear_array[WINDOWLENGTH];
    BUFFER buffer;
    BUFFER clear_buffer;
    RENDERER renderer;
    int_64 temp_bitmask[WINDOWHEIGHT][global_bitmask_groups];
    int_64 global_bitmask[WINDOWHEIGHT][global_bitmask_groups];
    char cursor_pos_placeholder[10] = "\x1b[00;000H";
    inline void inject_cursor_coords(const Sprite& sprite, int traversed);
    inline void render_all(int c_index, int traversed);
    void init_clear_array();
    inline void update_global_bitmask(const Sprite& sprite);
    void print_buffer();
    inline void clear_screen();
public:
    POINT_e screen_size, font_size;

    void clean_renderer();
    void empty_renderer();
    void update_buffer_from_renderer();
    void add_sprite_to_renderer(Sprite* sprite);
    void add_sprites_to_renderer(Sprite** sprites, unsigned int sz);
    void remove_sprite_from_renderer(Sprite* sprite);
    void remove_sprites_from_renderer(Sprite** sprites, unsigned int sz);
    void remove_sprites_from_renderer(std::function<bool(Sprite*)> condition);

    void DEBUG_fill();
    void DEBUG_buffer();

    std::unordered_set<char> get_keys();
    void empty_keys();
    void gml(std::function<void()> game_logic);
    void game_loop(std::function<void()> game_logic);
    void input();
    void set_font_settings(unsigned int f_height, unsigned int f_width);

    Window();
};

};