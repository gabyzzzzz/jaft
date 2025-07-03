#include "libraries.h"
#include "defines.h"
#include "classes.h"

void Scene::clean() {
    unsigned int rmv = 0;
    unsigned int next_valid = 0;
    for (unsigned int i = 0; i < nr_of_sprites; i++) {
        if (sprites[i] != nullptr) {
            if (i != next_valid) {
                sprites[next_valid] = sprites[i];
                sprites[i] = nullptr;
            }
            next_valid++;
        } else rmv++;
    }
    nr_of_sprites -= rmv;
}

void Scene::hide_scene() {
    //Face fiecare sprite din scena invizibil
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        sprites[i]->hide();
}

void Scene::show_scene() {
    //Face fiecare sprite din scena vizibil
    for (unsigned int i = 0; i < nr_of_sprites; i++)
        sprites[i]->show();
}

void Scene::add_sprite(Sprite* sprite) {
    if (MAXNROFSPRITES <= nr_of_sprites + 1) 
        log(309, sprite->label);
    sprites[nr_of_sprites] = sprite;
    nr_of_sprites++;
}

void Scene::add_sprites(Sprite** s1, unsigned int sz) {
    //Adauga mai multe sprite-uri in scena
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

void Scene::remove_sprites(Sprite** s1, unsigned int sz) {
    //Sterge mai multe sprite-uri dupa pointer
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

void Scene::init_by_file(const char file_name[]) {
    //Realoca si initializeaza memoria, folosind path-urile din fisier
    std::ifstream in(file_name);
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

void Scene::remove_sprites(std::function<bool(Sprite*)> condition) {
    //Sterge pointerii sprite-urilor care respecta conditia
    unsigned int rmv = 0;
    for (unsigned int i = 0; i < nr_of_sprites; i++) 
        if (condition(sprites[i])) 
            sprites[i] = nullptr;
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

void Scene::remove_all_sprites() {
    //Sa fie folosit cand memoria este initializata de clasa Scena. Daca este doar partial initializata de Scena, sa se creeze o functie separata
    if (nr_of_sprites) {
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
        if (sprites[i] == nullptr) std::cout << "nullptr\n\n";
        else {
            sprites[i]->DEBUG_sprite();
            std::cout << '\n';
        }
    }
}

Scene::Scene(const char file_name[]) {
    init_by_file(file_name);
}

Scene::Scene() = default;