#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"

using namespace std;

Sprite player("../test_sprites/sprite.spr", 0);
Window window;

const char passed_char[1] = {' '};

void game_l() {
    unordered_set<char> keys = window.get_keys_pressed();
    if (keys.count('w')) player.y--;
    if (keys.count('a')) player.x--;
    if (keys.count('s')) player.y++;
    if (keys.count('d')) player.x++;
    window.empty_keys_pressed();
}

int main() { 
    //y - 1, x - 2
    player.x = 2; player.y = 1;
    window.add_sprite_to_renderer(&player);
    window.game_loop(game_l);
    return 0;
}