#include "libraries.h"
#include "../lib/jaft.h"

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