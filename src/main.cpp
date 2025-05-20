#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"

using namespace std;

int main()
{
    Sprite test("../test_sprites/sprite.spr", 1);
    //Great, working with constructors now! Great quality of life update. ( Also, kind of clean :)))) )
    Window window;
    window.add_sprite_to_renderer(&test);
    window.update_buffer_from_renderer();
    window.print_buffer();
    
    cin.get();
    return 0;
}
