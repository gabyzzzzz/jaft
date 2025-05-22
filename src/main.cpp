#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"

using namespace std;

Sprite test("../test_sprites/sprite.spr", 1);
Window window;

void test_function() {
    char c_key;
    if (window.get_key_pressed(c_key)) {
        if (c_key == 'w') test.y--;
        else if (c_key == 'a') test.x--;
        else if (c_key == 'd') test.x++;
        else if (c_key == 's') test.y++;
    }
}

int main()
{ 
    window.add_sprite_to_renderer(&test);
    window.game_loop(test_function);
    return 0;
}
