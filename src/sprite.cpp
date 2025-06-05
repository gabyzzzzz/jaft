#include "../includes/classes.h"
#include "../includes/libraries.h"
#include "../includes/defines.h"

void Sprite::init_by_file(const char file_name[]) {
    std::ifstream in(file_name);
    if (!in.is_open()) log(401, label);
    if (!(in >> frame_size.y >> frame_size.x >> animation.nr_of_frames)) log(402, label);
    if (frame_size.y < 1 || frame_size.x < 1 || animation.nr_of_frames < 1) log(403, label);
    unsigned int nrf = animation.nr_of_frames;
    frames = new CELL**[nrf];
    for (unsigned int f = 0; f < nrf; f++) {
        frames[f] = new CELL*[frame_size.y];
        for (unsigned int h = 0; h < frame_size.y; h++) {
            frames[f][h] = new CELL[frame_size.x]; 
            for (unsigned int w = 0; w < frame_size.x; w++) {
                int ch; 
                in >> frames[f][h][w].color.r
                   >> frames[f][h][w].color.g
                   >> frames[f][h][w].color.b
                   >> ch;
                frames[f][h][w].character = (char) ch;
            }
            frames[f][h][frame_size.x].character = '\0';
        }
    }
}

void Sprite::DEBUG_sprite() {
    //Printeaza fisierul citit (fara culori)
    cout << frame_size.y << ' ' << frame_size.x << ' ' << animation.nr_of_frames << "\n\n";
    for (unsigned int f = 0; f < animation.nr_of_frames; f++) {
        for (unsigned int h = 0; h < frame_size.y; h++) {
            for (unsigned int w = 0; w < frame_size.x; w++) cout << frames[f][h][w].character;
            cout << '\n';
        }
        if (f < animation.nr_of_frames - 1) cout << '\n';
    }
}

Sprite::~Sprite () {
    unsigned int nfr = animation.nr_of_frames;
    POINT_e fsz = frame_size;
    for (unsigned int f = 0; f < nfr; ++f) {
        for (unsigned int y = 0; y < fsz.y; ++y) delete[] frames[f][y];
        delete[] frames[f];
    }
    delete[] frames;
}

Sprite::Sprite() = default;

Sprite::Sprite(const char file_name[], unsigned int lbl) : label(lbl) {
    init_by_file(file_name);
}

Sprite::Sprite(unsigned int lbl) : label(lbl) {}

bool Sprite::is_colliding(Sprite* sprite) {
    if (coords.x + frame_size.x - 1 < sprite->coords.x) return false;
    if (sprite->coords.x + sprite->frame_size.x - 1 < coords.x) return false;
    if (coords.y + frame_size.y - 1 < sprite->coords.y) return false;
    if (sprite->coords.y + sprite->frame_size.y - 1 < coords.y) return false;
    return true;
}

bool Sprite::is_colliding(Sprite* sprite, const char characters[], unsigned int sz) {
    if (!is_colliding(sprite)) return false;
    if (sz < 1) log(702, label);
    if (!characters) log(702, label);
    unsigned int x_start = max(coords.x, sprite->coords.x);
    unsigned int x_end = min(coords.x + frame_size.x, sprite->coords.x + sprite->frame_size.x);
    unsigned int y_start = max(coords.y, sprite->coords.y);
    unsigned int y_end = min(coords.y + frame_size.y, sprite->coords.y + sprite->frame_size.y);
    for (unsigned int i = y_start; i < y_end; i++) {
        for (unsigned int j = x_start; j < x_end; j++) {
            char current_pixel = sprite->frames[sprite->animation.current_frame][i - sprite->coords.y][j - sprite->coords.x].character;
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

void Sprite::sprite_init() {
    unsigned int nfr = animation.nr_of_frames;
    POINT_e fsz = frame_size;
    frames = new CELL**[nfr];
    for (unsigned int f = 0; f < nfr; f++) {
        frames[f] = new CELL*[fsz.y];
        for (unsigned int h = 0; h < fsz.y; h++) {
            frames[f][h] = new CELL[fsz.x];
            for (unsigned int w = 0; w < fsz.x; w++) frames[f][h][w] = {{255, 255, 255}, ' '};
        }
    }
}

void Sprite::next_game_tick() {
    animation.current_tick++;
    if (animation.current_tick >= animation.ticks_per_frame) {
        animation.current_tick = 0;
        animation.current_frame++;
        if (animation.current_frame >= animation.nr_of_frames) animation.current_frame = 0;
    }
}