#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"

using namespace std;

Sprite player("../test_sprites/sprite.spr", 0);
Sprite maze("../test_sprites/sprite2.spr", 1);
Window window;

const unsigned int game_sprites_size = 2;

const char passed_char[1] = {' '};

void game_l() {
    char current_char;
    window.get_key_pressed(current_char);
    unsigned int prv_x, prv_y;
    prv_x = player.x; prv_y = player.y;
    if (current_char == 'w') player.y--;
    if (current_char == 'a') player.x--;
    if (current_char == 's') player.y++;
    if (current_char == 'd') player.x++;
    if (player.is_colliding(&maze, passed_char, 1)) { 
        exit(0);
        player.x = prv_x; 
        player.y = prv_y; 
    }
    if (player.y < 10 && player.y > 0 && player.x == 233) exit(0);
}

int main() { 
    //y - 1, x - 2
    Sprite* game_sprites[game_sprites_size] = {&player, &maze};
    player.x = 2; player.y = 1;
    window.add_sprites_to_renderer(game_sprites, game_sprites_size);
    window.game_loop(game_l);
    return 0;
}
