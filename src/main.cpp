#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"

using namespace std;

int main()
{
    Window window;
    window.config();
    window.DEBUG_fill();
    cout << window.font_size_height << ' ' << window.font_size_width;
    cin.get();
    return 0;
}
