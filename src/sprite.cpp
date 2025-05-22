#include "../includes/classes.h"
#include "../includes/libraries.h"
#include "../includes/defines.h"

void Sprite::init_by_file(const char file_name[]) {
    //Initializeaza sprite-ul din fisier
    ifstream in(file_name);
    if (!in.is_open()) log(401, label);
    if (!(in >> frame_height >> frame_width >> nr_of_frames)) log(402, label);
    if (frame_height < 1 || frame_width < 1 || nr_of_frames < 1) log(403, label);
    string input;
    //Scapam de primul new line
    getline(in, input);
    sprite_frames = new char**[nr_of_frames];
    for (unsigned int f = 0; f < nr_of_frames; f++) {
        sprite_frames[f] = new char*[frame_height];
        //Scapam de new line
        getline(in, input);
        for (unsigned int h = 0; h < frame_height; h++) {
            sprite_frames[f][h] = new char[frame_width + 1];
            getline(in, input);
            //Daca am ajuns la sfarsitul liniei inlocuim caracterele cu spatii libere
            for (unsigned int w = 0; w < frame_width; w++) sprite_frames[f][h][w] = (w < input.length()) ? input[w] : ' ';
            sprite_frames[f][h][frame_width] = '\0';
        }
    }
}

void Sprite::DEBUG_sprite() {
    //Printeaza fisierul citit
    cout << frame_height << ' ' << frame_width << ' ' << nr_of_frames << "\n\n";
    for (unsigned int f = 0; f < nr_of_frames; f++) {
        for (unsigned int h = 0; h < frame_height; h++) {
            for (unsigned int w = 0; w < frame_width; w++) cout << sprite_frames[f][h][w];
            cout << '\n';
        }
        if (f < nr_of_frames - 1) cout << '\n';
    }
}

Sprite::~Sprite() {
    //Eliberam memoria folosita (De modificat daca mai folosim memorie alocata dinamic)
    for (unsigned int f = 0; f < nr_of_frames; f++) {
        for (unsigned int i = 0; i < frame_height; i++) delete[] sprite_frames[f][i];
        delete[] sprite_frames[f];
    }
    delete[] sprite_frames;
}

Sprite::Sprite() = default;

Sprite::Sprite(const char file_name[], unsigned int lbl) : label(lbl) {
    init_by_file(file_name);
}

Sprite::Sprite(unsigned int lbl) : label(lbl) {}

bool Sprite::is_colliding(Sprite* sprite) {
    if (x + frame_width - 1 < sprite->x) return false;
    if (sprite->x + sprite->frame_width - 1 < x) return false;
    if (y + frame_height - 1 < sprite->y) return false;
    if (sprite->y + sprite->frame_height - 1 < y) return false;
    return true;
}

bool Sprite::is_colliding(Sprite* sprite, const char characters[], unsigned int sz) {
    if (!is_colliding(sprite)) return false;
    if (sz < 1) log(702, label);
    if (!characters) log(702, label);
    unsigned int x_start = max(x, sprite->x);
    unsigned int x_end = min(x + frame_width, sprite->x + sprite->frame_width);
    unsigned int y_start = max(y, sprite->y);
    unsigned int y_end = min(y + frame_height, sprite->y + sprite->frame_height);
    for (unsigned int i = y_start; i < y_end; i++) {
        for (unsigned int j = x_start; j < x_end; j++) {
            char current_pixel = sprite->sprite_frames[sprite->current_frame][i - sprite->y][j - sprite->x];
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