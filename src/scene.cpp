#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"

void Scene::clean() {
    //Muta toate pozitiile libere la dreapta pentru un array compact
    unsigned int next_valid = 0;
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        if (sprites[i] != nullptr) {
            if (i != next_valid) {
                sprites[next_valid] = sprites[i];
                sprites[i] = nullptr;
            }
            next_valid++;
        }
}

void Scene::hide_scene() {
    //Face fiecare sprite din scena invizibil
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        sprites[i]->visible = false;
}

void Scene::show_scene() {
    //Face fiecare sprite din scena vizibil
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        sprites[i]->visible = true;
}

void Scene::add_sprite(Sprite* sprite) {
    unsigned int i = 0;
    while (i < nr_of_sprites) {
        if (sprites[i] == nullptr) {
            sprites[i] = sprite;
            return;
        }
        i++;
    }
    log(309, sprite->label);
}

void Scene::init_by_file(const char file_name[]) {
    //Realoca si initializeaza memoria, folosind path-urile din fisier
    ifstream in(file_name);
    if(!in.is_open()) log(301);
    if (!(in >> nr_of_sprites)) log(302);
    sprites = new Sprite*[nr_of_sprites];
    for (unsigned int i = 0; i < nr_of_sprites; i++) {
        char temp[100];
        if (!(in >> temp)) log(302);
        sprites[i] = new Sprite;
        sprites[i]->init_by_file(temp);
    }
}

void Scene::set_nr_of_sprites(unsigned int nr) {
    //Realoca memorie pentru sprite-uri
    if (nr_of_sprites) {
        for (unsigned int i = 0; i < nr_of_sprites; i++) sprites[i] = nullptr;
        delete[] sprites;
        sprites = nullptr;
    }
    sprites = new Sprite*[nr];
    for (unsigned int i = 0; i < nr; i++) sprites[i] = nullptr;
    nr_of_sprites = nr;
}

void Scene::remove_sprite(Sprite* sprite) {
    //Sterge pointer-ul dat din vectorul cu sprite-uri
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        if (sprites[i] == sprite) {
            sprites[i] = nullptr;
            break;
        }
    clean();
}

void Scene::remove_sprite(unsigned int lbl) {
    //Sterge pointerii sprite-urilor cu label-ul lbl
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        if (sprites[i]->label == lbl) sprites[i] = nullptr;
    clean();
}

Scene::~Scene() {
    //Memoria alocata dinamic este eliberata (De modificat daca folosim mai multa). Destructorul nu va sterge Sprite-urile in sine, ci doar pointerele catre ele. Sa se foloseasca delete_sprite daca memoria este initializata de Scena
    if (sprites) {
        for (unsigned int i = 0; i < nr_of_sprites; i++) sprites[i] = nullptr;
        delete[] sprites;
        sprites = nullptr;
    }
}

void Scene::delete_sprites() {
    //Sa fie folosit cand memoria este initializata de clasa Scena. Daca este doar partial initializata de Scena, sa se creeze o functie separata
    if (sprites) {
        for (unsigned int i = 0; i < nr_of_sprites; i++) {
            delete sprites[i];
        }
        delete[] sprites;
        sprites = nullptr;
        nr_of_sprites = 0;
    }
}

Scene& Scene::operator=(const Scene& obj1)  {
    //Returneaza primul obiect din operatie
    if (this == &obj1) return *this;
    set_nr_of_sprites(obj1.nr_of_sprites);
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        sprites[i] = obj1.sprites[i];
    return *this;
}

void Scene::DEBUG_scene() {
    //Printeaza pe consola fiecare sprite
    for (unsigned int i = 0; i < nr_of_sprites; i++) {
        if (sprites[i] == nullptr) cout << "nullptr\n\n";
        else {
            sprites[i]->DEBUG_sprite();
            cout << '\n';
        }
    }
}
