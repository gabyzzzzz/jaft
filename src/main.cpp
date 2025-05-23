#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"

using namespace std;

Sprite test("../test_sprites/sprite.spr", 1);
Sprite test2("../test_sprites/sprite2.spr", 2);
Window window;

const char passed_char[1] = {' '};

void test_function() {
    char c_key;
    if (window.get_key_pressed(c_key)) {
        pair<unsigned int, unsigned int> coords;
        coords.first = test.x;
        coords.second = test.y;
        if (c_key == 'w') test.y--;
        else if (c_key == 'a') test.x--;
        else if (c_key == 'd') test.x++;
        else if (c_key == 's') test.y++;
        if (test.is_colliding(&test2, passed_char, 1)) {
            test.x = coords.first;
            test.y = coords.second;
        }
    }
}

int main() { 
    Sprite* sprites[] = {&test, &test2};
    window.add_sprites_to_renderer(sprites, 2);
    test2.x += 20;
    test2.y += 20;
    test.stage = 1;
    test2.stage = 1;
    window.game_loop(test_function);
    return 0;
}
