#include "libraries.h"
#include "../lib/jaft.h"

void jaft::Sprite::init_memory() {
    for (int f = 0; f < animation.nr_of_frames; f++)
        for (int y = 0; y < frame_size.y; y++)
            for (int x = 0; x < frame_size.x; x++)
                frames[f][y][x] = {0, ' '};
    
    for (int i = 0; i < animation.nr_of_frames; i++) 
        renderer.size[i] = 0;

    for (int i = 0; i < renderer.nr_of_colors; i++) {
        for (int j = 0; j < animation.nr_of_frames; j++) 
            renderer.colored_chunks.size[i][j] = 0;  
    }

    for (int i = 0; i < animation.nr_of_frames; i++) 
        renderer.cursor_hops.size[i] = 0;

    int size_bitmask_x = (frame_size.x + 63) / 64;
    for (int i = 0; i < animation.nr_of_frames; i++) 
        for (int j = 0; j < frame_size.y; j++) 
            for (int k = 0; k < size_bitmask_x; k++) 
                bitmask[i][j][k] = 0;
}

void jaft::Sprite::alocate_memory() {
    try {

        renderer.palette = (COLOR*) allocate_vector(
            renderer.nr_of_colors,
            sizeof(COLOR)
        );

        frames = (CELL***) allocate_tensor(
            animation.nr_of_frames,
            frame_size.y,
            frame_size.x,
            sizeof(CELL)
        );

        renderer.value = (char**) allocate_matrix(
            animation.nr_of_frames, 
            BUFFERMULTIPLIER * frame_size.x * frame_size.y + 2,
            sizeof(char)
        );
       
        renderer.size = (size_t*) allocate_vector(
            animation.nr_of_frames, 
            sizeof(size_t)
        );

        renderer.colored_chunks.container = (SBIT***) allocate_tensor(
            renderer.nr_of_colors, 
            animation.nr_of_frames, 
            frame_size.x * frame_size.y + 2, 
            sizeof(SBIT)
        );

        renderer.colored_chunks.size = (size_t**) allocate_matrix(
            renderer.nr_of_colors,
            animation.nr_of_frames,
            sizeof(size_t)
        );

        renderer.cursor_hops.size = (size_t*) allocate_vector(
            animation.nr_of_frames,
            sizeof(size_t)
        );

        renderer.cursor_hops.indexes = (int**) allocate_matrix(
            animation.nr_of_frames,
            frame_size.x * frame_size.y + 2,
            sizeof(int)
        );

        renderer.cursor_hops.values = (POINT_e**) allocate_matrix(
            animation.nr_of_frames,
            frame_size.x * frame_size.y + 2,
            sizeof(POINT_e)
        );

        bitmask = (int_64***) allocate_tensor(
            animation.nr_of_frames,
            frame_size.y,
            (frame_size.x + 63) / 64,
            sizeof(int_64)
        );

    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }

    init_memory();

    refresh();
}

void jaft::Sprite::init_by_file(const char file_name[]) {
    try {

        std::ifstream in(file_name);
        if (!in.is_open()) log(401, label);
        if (!(in >> frame_size.y >> frame_size.x >> animation.nr_of_frames >> renderer.nr_of_colors)) log(402, label);
        if (frame_size.y < 1 || frame_size.x < 1 || animation.nr_of_frames < 1 || renderer.nr_of_colors < 1) log(403, label);
        alocate_memory();
        int ch;
        for (int i = 0; i < renderer.nr_of_colors; i++) in >> renderer.palette[i].r >> renderer.palette[i].g >> renderer.palette[i].b;
        for (unsigned int f = 0; f < animation.nr_of_frames; f++) 
            for (unsigned int h = 0; h < frame_size.y; h++) 
                for (unsigned int w = 0; w < frame_size.x; w++) {
                    in >> frames[f][h][w].color_id >> ch;
                    frames[f][h][w].character = (char)ch;
                }

    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }

}

void jaft::Sprite::sprite_init() {
    try {
        alocate_memory();
    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }
    
}

void jaft::Sprite::free_memory() {

    free_tensor(
        frames,
        animation.nr_of_frames,
        frame_size.y
    );

    free_tensor(
        bitmask,
        animation.nr_of_frames,
        frame_size.y
    );

    free_tensor(
        renderer.colored_chunks.container,
        renderer.nr_of_colors,
        animation.nr_of_frames
    );

    free_matrix(
        renderer.colored_chunks.size,
        renderer.nr_of_colors
    );
    
    free_matrix(
        renderer.value,
        animation.nr_of_frames
    );

    free_matrix(
        renderer.cursor_hops.indexes,
        animation.nr_of_frames
    );

    free_matrix(
        renderer.cursor_hops.values,
        animation.nr_of_frames
    );

    free_vector(
        renderer.size
    );

    free_vector(
        renderer.cursor_hops.size
    );

    free_vector(
        renderer.palette
    );
    
}

jaft::Sprite::~Sprite() {
    free_memory();
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
    int x_end = x + frame_size.x - 1;
    int y_end = y + frame_size.y - 1;
    if (x_end >= WINDOWLENGTH) x = WINDOWLENGTH - frame_size.x;
    if (x < 0) x = 0;
    if (y_end >= WINDOWHEIGHT) y = WINDOWHEIGHT - frame_size.y;
    if (y < 0) y = 0;
    coords.x = x;
    coords.y = y;
    status.coord_changed = true;
}

void jaft::Sprite::add_x(int delta_x) {
    set_coords(coords.x + delta_x, coords.y);
}

void jaft::Sprite::add_y(int delta_y) {
    set_coords(coords.x, coords.y + delta_y);
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

void jaft::Sprite::refresh_bitmask(int target_frame) {
    for (int i = 0; i < frame_size.y; i++) {
        for (int j = 0; j < (frame_size.x + 63) / 64; j++) {
            bitmask[target_frame][i][j] = 0;
        }
    }
    int_64** current_bitmask = bitmask[target_frame];
    CELL** current_frame = frames[target_frame];
    int_64 set_bit;
    for (int y = 0; y < frame_size.y; y++) {
        set_bit = 1;
        for (int x = 0; x < frame_size.x; x++) {
            if (current_frame[y][x].character != ' ' || !(view.transparent_white_spaces)) {
                current_bitmask[y][x / 64] |= set_bit;
            }
            if (set_bit == MAXSETBIT) set_bit = 1;
            else set_bit <<=  1;
        }
    }
}

void jaft::Sprite::refresh_render_code(COLOR palette_[], int target_frame) {
    char ansi_clr[] = "\x1b[38;2;";
    bool consecutive, different_lines;
    renderer.size[target_frame] = 0;
    renderer.cursor_hops.size[target_frame] = 0;
    refresh_colored_chunks(target_frame);
    for (int c = 0; c < renderer.nr_of_colors; c++) {
        if (renderer.colored_chunks.size[c][target_frame] > 0) {
            memcpy(renderer.value[target_frame] + renderer.size[target_frame], ansi_clr, 7);
            renderer.size[target_frame] += 7;
            color_to_string(palette_[c], renderer.value[target_frame] + renderer.size[target_frame]);
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
    unsigned int x_start = (std::max)(coords.x, sprite.coords.x);
    unsigned int x_end = (std::min)(coords.x + frame_size.x, sprite.coords.x + sprite.frame_size.x);
    unsigned int y_start = (std::max)(coords.y, sprite.coords.y);
    unsigned int y_end = (std::min)(coords.y + frame_size.y, sprite.coords.y + sprite.frame_size.y);
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
    for (unsigned int i = 0; i < animation.nr_of_frames; i++) {
        refresh_bitmask(i);
        refresh_render_code(renderer.palette, i);
    }
    status.modified = true;
    status.coord_changed = true;
}

void jaft::Sprite::resize(POINT_e frame_sizes, int nr_of_frames) {
    CELL*** new_frames = (CELL***) allocate_tensor(
        nr_of_frames,
        frame_sizes.y,
        frame_sizes.x,
        sizeof(CELL)
    ); 
    for (int f = 0; f < nr_of_frames; f++)
        for (int y = 0; y < frame_sizes.y; y++) 
            for (int x = 0; x < frame_sizes.x; x++) {
                if (x < frame_size.x && y < frame_size.y && f < animation.nr_of_frames) new_frames[f][y][x] = frames[f][y][x];
            }
    COLOR* palette = (COLOR*) allocate_vector(
        renderer.nr_of_colors,
        sizeof(COLOR)
    ); 
    for (int c = 0; c < renderer.nr_of_colors; c++) palette[c] = renderer.palette[c];
    free_memory();
    frame_size = frame_sizes;
    animation.nr_of_frames = nr_of_frames;
    alocate_memory();
    free_tensor(
        frames,
        animation.nr_of_frames,
        frame_size.y
    );
    frames = new_frames;
    for (int c = 0; c < renderer.nr_of_colors; c++)  renderer.palette[c] = palette[c];
    refresh();
    free_vector(palette);
    return;
}