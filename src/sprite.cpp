#include "classes.h"
#include "libraries.h"
#include "defines.h"

//   ________________________________________________________
//  | ALOCATING MEMORY / CONSTRUCTORS / INITIALISING THINGS |

//  Made this to not repeat myself
void Sprite::alocate_memory() {
    //  We are alocating memory for the render code of the sprite
    try {

    renderer.value = new char*[animation.nr_of_frames];
    for (int i = 0; i < animation.nr_of_frames; i++) renderer.value[i] = new char[BUFFERMULTIPLIER * frame_size.x * frame_size.y + 2];
    renderer.size = new size_t[animation.nr_of_frames];
    for (int i = 0; i < animation.nr_of_frames; i++) renderer.size[i] = 0;
    //  Alocate memory for colored chunks
    renderer.colored_chunks.container = new SBIT**[renderer.nr_of_colors];
    for (int i = 0; i < renderer.nr_of_colors; i++) {
        renderer.colored_chunks.container[i] = new SBIT*[animation.nr_of_frames];
        for (int j = 0; j < animation.nr_of_frames; j++) renderer.colored_chunks.container[i][j] = new SBIT[frame_size.x * frame_size.y + 2];
    }
    renderer.colored_chunks.size = new size_t*[renderer.nr_of_colors];
    for (int i = 0; i < renderer.nr_of_colors; i++) renderer.colored_chunks.size[i] = new size_t[animation.nr_of_frames];
    for (int i = 0; i < renderer.nr_of_colors; i++) {
        for (int j = 0; j < animation.nr_of_frames; j++) {
            renderer.colored_chunks.size[i][j] = 0;
        }
    }
    //  Alocate memory for cursor hops
    renderer.cursor_hops.size = new size_t[animation.nr_of_frames];
    for (int i = 0; i < animation.nr_of_frames; i++) renderer.cursor_hops.size[i] = 0;
    renderer.cursor_hops.indexes = new int*[animation.nr_of_frames];
    renderer.cursor_hops.values = new POINT_e*[animation.nr_of_frames];
    for (int i = 0; i < animation.nr_of_frames; i++) {
        renderer.cursor_hops.indexes[i] = new int[frame_size.x * frame_size.y + 2];
        renderer.cursor_hops.values[i] = new POINT_e[frame_size.x * frame_size.y + 2];
    }

    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }
    
    previous_pos = {{1, 1}, {0, 0}};
    refresh();
}

void Sprite::init_by_file(const char file_name[]) {
    try {

    std::ifstream in(file_name);
    //  Check if file is open and the first values are valid.
    if (!in.is_open()) log(401, label);
    if (!(in >> frame_size.y >> frame_size.x >> animation.nr_of_frames >> renderer.nr_of_colors)) log(402, label);
    if (frame_size.y < 1 || frame_size.x < 1 || animation.nr_of_frames < 1 || renderer.nr_of_colors < 1) log(403, label);
    //  Alocating and setting the values for pallete.
    renderer.pallete = new COLOR[renderer.nr_of_colors];
    for (int i = 0; i < renderer.nr_of_colors; i++) in >> renderer.pallete[i].r >> renderer.pallete[i].g >> renderer.pallete[i].b;
    //  We are reading the characters and color ids while also alocating memory
    unsigned int nrf = animation.nr_of_frames;
    frames = new CELL**[nrf];
    for (unsigned int f = 0; f < nrf; f++) {
        frames[f] = new CELL*[frame_size.y + 1];
        for (unsigned int h = 0; h < frame_size.y; h++) {
            frames[f][h] = new CELL[frame_size.x + 1]; 
            for (unsigned int w = 0; w < frame_size.x; w++) {
                int ch; 
                in >> frames[f][h][w].color_id >> ch;
                frames[f][h][w].character = (char) ch;
            }
        }
    }

    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }

    alocate_memory();
}

void Sprite::sprite_init() {
    try {

    unsigned int nfr = animation.nr_of_frames;
    POINT_e fsz = frame_size;
    frames = new CELL**[nfr];
    renderer.pallete = new COLOR[renderer.nr_of_colors];
    for (int i = 0; i < renderer.nr_of_colors; i++) {
        renderer.pallete[i] = {255, 0, 0};
    }
    for (unsigned int f = 0; f < nfr; f++) {
        frames[f] = new CELL*[fsz.y + 1];
        for (unsigned int h = 0; h < fsz.y; h++) {
            frames[f][h] = new CELL[fsz.x + 1];
            for (unsigned int w = 0; w < fsz.x; w++) frames[f][h][w] = {0, ' '};
        }
    }

    }

    catch (const std::bad_alloc& e) {
        std::cout << "[JAFT] Failed alocating memory for sprite with label " << label << ". " << e.what() << std::endl;
    }

    alocate_memory();
}

//  Destructor
Sprite::~Sprite () {
    //  Stuff for loops
    unsigned int nfr = animation.nr_of_frames;
    POINT_e fsz = frame_size;

    //  Delete frames
    for (unsigned int f = 0; f < nfr; ++f) {
        for (unsigned int y = 0; y < fsz.y; ++y)
            delete[] frames[f][y];
        delete[] frames[f];
    }
    delete[] frames;

    //  Delete colored chunks
    for (int i = 0; i < renderer.nr_of_colors; i++) {
        for (int j = 0; j < animation.nr_of_frames; j++)
            delete[] renderer.colored_chunks.container[i][j];
        delete[] renderer.colored_chunks.size[i]; 
    } 
    delete[] renderer.colored_chunks.container;
    delete[] renderer.colored_chunks.size;
    
    //  Delete render code
    for (int i = 0; i < animation.nr_of_frames; i++) {
        delete[] renderer.value[i];
        delete[] renderer.cursor_hops.indexes[i];
        delete[] renderer.cursor_hops.values[i];
    }
    delete[] renderer.value;
    delete[] renderer.size;
    delete[] renderer.cursor_hops.values;
    delete[] renderer.cursor_hops.indexes;
    delete[] renderer.cursor_hops.size;
    
    //  This one is lonely
    delete[] renderer.pallete;
}

//  Constructors

Sprite::Sprite() = default;

Sprite::Sprite(const char file_name[], unsigned int lbl) : label(lbl) {
    init_by_file(file_name);
}

Sprite::Sprite(unsigned int lbl) : label(lbl) {}

//   __________________________________________
//  | DEBUGGING / SHOWING STUFF TO THE CONSOLE |

//  Prints the characters from the raw matrix (a bit outdated but works)
void Sprite::DEBUG_sprite() {
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

//  Prints the render code (frame 1) and cursor hops
void Sprite::DEBUG_render_code() {
    std::cout << "[DEBUG] Render code:\n";
    for (int i = 0; i < renderer.size[animation.current_frame]; i++) {
        if (renderer.value[0][i] == '\x1b') {
            std::cout << "ESC";
        } else std::cout << renderer.value[0][i];
    }
    std::cout << "\n[DEBUG] Cursor hops:\n";
    for (int i = 0; i < renderer.cursor_hops.size[animation.current_frame]; i++) std::cout << renderer.cursor_hops.indexes[animation.current_frame][i] << ' ';
    std::cout << '\n';
    std::cout.flush();
}

//   ______________________
//  | UTILITIES / FUNCTIONS|

//  TODO : add a sprite lock, where you cut off memory you dont actually use.

POINT_e Sprite::get_coords() const {
    return coords;
}

void Sprite::set_coords(int x, int y) {
    int x_ = x + frame_size.x;
    int y_ = y + frame_size.y;
    if ((x_ < WINDOWLENGTH && x_ >= 0) && (y_ < WINDOWHEIGHT && y >= 0)) {
        coords.x = x;
        coords.y = y;
    } else {
        if (x_ >= WINDOWLENGTH) coords.x = WINDOWLENGTH - frame_size.x;
        if (x_ < 0) coords.x = 0;
        if (y_ >= WINDOWHEIGHT) coords.y = WINDOWHEIGHT - frame_size.y;
        if (y_ < 0) coords.y = 0;
    }
    status.coord_changed = true;
}

void Sprite::add_x(int x) {
    int x_ = coords.x + x + frame_size.x;
    if (x_ < WINDOWLENGTH && x_ >= 0) coords.x = x + coords.x;
    else {
        if (x_ >= WINDOWLENGTH) coords.x = WINDOWLENGTH - frame_size.x;
        if (x_ < 0) coords.x = 0;
    } 
    status.coord_changed = true;
}

void Sprite::add_y(int y) {
    int y_ = coords.y + y + frame_size.y;
    if (y_ < WINDOWHEIGHT && y_ >= 0) coords.y = y + coords.y;
    else {
        if (y_ >= WINDOWHEIGHT) coords.y = WINDOWHEIGHT - frame_size.y;
        if (y_ < 0) coords.y = 0;
    }
    status.coord_changed = true;
}

ANIMATION Sprite::get_animation() const {
    return animation;
}

void Sprite::start_animation() {
    animation.is_animation_active = true;
}

void Sprite::stop_animation() {
    animation.is_animation_active = false;
}

void Sprite::set_current_frame(int frame) {
    animation.current_frame = frame;
    status.modified = true;
}

void Sprite::set_ticks_per_frame(int ticks_per_frame) {
    animation.ticks_per_frame = ticks_per_frame;
}

void Sprite::set_current_tick(int current_tick) {
    animation.current_tick = current_tick;
}

VIEW Sprite::get_view() const {
    return view;
}

void Sprite::hide() {
    status.modified = true;
    view.visible = false;
}

void Sprite::show() {
    status.modified = true;
    view.visible = true;
}

void Sprite::transparent_space(bool condition) {
    view.transparent_white_spaces = condition;
    refresh();
}

void Sprite::set_stage(int z_index) {
    view.stage = z_index;
}

SPRITESTATUS& Sprite::get_status() {
    return status;
}

void Sprite::set_nr_of_frames(int frames) {
    animation.nr_of_frames = frames;
}

SRENDERER& Sprite::get_srenderer() {
    return renderer;
}

void Sprite::color_to_string(COLOR clr, char* target) {
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

bool Sprite::is_colliding(const Sprite& sprite) const {
    if (coords.x + frame_size.x - 1 < sprite.coords.x) return false;
    if (sprite.coords.x + sprite.frame_size.x - 1 < coords.x) return false;
    if (coords.y + frame_size.y - 1 < sprite.coords.y) return false;
    if (sprite.coords.y + sprite.frame_size.y - 1 < coords.y) return false;
    return true;
}

bool Sprite::is_colliding(const Sprite& sprite, const char characters[], unsigned int sz) const {
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

void Sprite::next_game_tick() {
    animation.current_tick++;
    if (animation.current_tick >= animation.ticks_per_frame) {
        animation.current_tick = 0;
        animation.current_frame++;
        if (animation.current_frame >= animation.nr_of_frames) animation.current_frame = 0;
    }
}

//   __________________________________________
//  | RENDERING / COLORED CHUNKS / CURSOR HOPS |

inline void Sprite::refresh_colored_chunks(int target_frame) {
    CELL current_cell;
    POINT_e current_coords;
    //  Set the sizes of the colored chunks to 0
    for (int i = 0; i < renderer.nr_of_colors; i++)
        renderer.colored_chunks.size[i][target_frame] = 0;
    //  Loop through the character matrix
    for (unsigned int y = 0; y < frame_size.y; y++) {
        for (unsigned int x = 0; x < frame_size.x; x++) {
            current_cell = frames[target_frame][y][x];
            if (current_cell.character != ' ' || view.transparent_white_spaces == false) {
                current_coords = {x, y};
                renderer.colored_chunks.container[current_cell.color_id][target_frame][renderer.colored_chunks.size[current_cell.color_id][target_frame]++] = {
                    current_coords,
                    current_cell.character
                };
            }
        }
    }
}

inline void Sprite::cursor_hop(int target_frame, unsigned int x, unsigned int y) {
    renderer.cursor_hops.indexes[target_frame][renderer.cursor_hops.size[target_frame]] = renderer.size[target_frame] + 2;
    renderer.cursor_hops.values[target_frame][renderer.cursor_hops.size[target_frame]] = { x, y };
    renderer.cursor_hops.size[target_frame]++;
    //  CHANGE - If coords are bigger than triple digits
    memcpy(renderer.value[target_frame] + renderer.size[target_frame], cursor_pos_placeholder, 9);
    renderer.size[target_frame] += 9;
}

//  Refreshes render code, commiting previous changes made to the sprite (including changing pallete and characters)
void Sprite::refresh_render_code(COLOR* pallete_, int target_frame) {
    //  TODO - Maybe replace the overlaping characters with ghost one if you get flickers when two sprites overlap
    char ansi_clr[] = "\x1b[38;2;";
    bool consecutive, different_lines;
    renderer.size[target_frame] = 0;
    renderer.cursor_hops.size[target_frame] = 0;
    //  Updating colored chunks
    refresh_colored_chunks(target_frame);
    //  Build render code
    for (int c = 0; c < renderer.nr_of_colors; c++) {
        //  Grouping same-colored chars together
        if (renderer.colored_chunks.size[c][target_frame] > 0) {
            //  Cursor hop on the first character automatically
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
            //  Cursor hop if characters not consecutive
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

void Sprite::refresh() {
    for (int i = 0; i < animation.nr_of_frames; i++)
        refresh_render_code(renderer.pallete, i);
    status.modified = true;
    status.coord_changed = true;
}