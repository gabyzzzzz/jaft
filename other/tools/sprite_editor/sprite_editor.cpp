//       _-------------------------------------------------------------------------_
//      | THIS IS A TOOL BUILT FOR CREATING AND EDITING SPRITE MODELS.              |
//      | ANYTHING CREATED HERE IS NOT GUARANTEED TO BE SUITED FOR FUTURE VERSIONS. |
//      -----------------------------------------------------------------------------

#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"
#include <vector>

using namespace std;

//       _--------------------------------------------_
//      | FEEL FREE TO USE THIS FOR TESTING PURPOSES  |
//      ----------------------------------------------

Sprite brush("brush.spr", 0); //    The literal brush you use for "painting"
Sprite* canvas = new Sprite(1);
Window window;

bool saved = true;
unsigned int c_frame = 0;
string crdir = "";
string crfile = "";

void save_to(string cnct) {
    ofstream out(cnct);
    out << canvas->frame_height << ' ' << canvas->frame_width << ' ' << canvas->nr_of_frames << ' ';
    for (unsigned int f = 0; f < canvas->nr_of_frames; f++) 
    for (unsigned int h = 0; h < canvas->frame_height; h++) 
    for (unsigned int w = 0; w < canvas->frame_width; w++) 
    out << canvas->r[f][h][w] << ' ' << canvas->g[f][h][w] << ' ' << 
    canvas->b[f][h][w] << ' ' << (int) canvas->sprite_frames[f][h][w] << ' ';
    out.flush();
    cout << "[CONSOLE] Successfully saved file.\n";
}

void new_canvas() {
    canvas->sprite_frames = new char**[canvas->nr_of_frames];
    canvas->r = new unsigned short int**[canvas->nr_of_frames];
    canvas->g = new unsigned short int**[canvas->nr_of_frames];
    canvas->b = new unsigned short int**[canvas->nr_of_frames];
    for (unsigned int f = 0; f < canvas->nr_of_frames; f++) {
        canvas->sprite_frames[f] = new char*[canvas->frame_height];
        canvas->r[f] = new unsigned short int*[canvas->frame_height];
        canvas->g[f] = new unsigned short int*[canvas->frame_height];
        canvas->b[f] = new unsigned short int*[canvas->frame_height];
        for (unsigned int h = 0; h < canvas->frame_height; h++) {
            canvas->sprite_frames[f][h] = new char[canvas->frame_width + 1];
            canvas->r[f][h] = new unsigned short int[canvas->frame_width];
            canvas->g[f][h] = new unsigned short int[canvas->frame_width];
            canvas->b[f][h] = new unsigned short int[canvas->frame_width];
            int ch;
            for (unsigned int w = 0; w < canvas->frame_width; w++) {
                canvas->sprite_frames[f][h][w] = ' ';
                canvas->r[f][h][w] = 255;
                canvas->g[f][h][w] = 255;
                canvas->b[f][h][w] = 255;
            }
            canvas->sprite_frames[f][h][canvas->frame_width] = '\0';
        }
    }
}

bool confirm_unsaved_file() {
    if (!saved) {
        char choice;
        cout << "[CONSOLE] You didn't save your current file. Do you wish to proceed? y/n \n>_ ";
        cin >> choice;
        if (choice != 'y' && choice != 'Y') {
            cout << "[CONSOLE] Command aborted.\n";
            return false;
        } else {
            cout << "[CONSOLE] Not saving file...\n";
        }
    }
    return true;
}

void get_command() {
    window.empty_buffer();
    window.print_buffer();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = { 0, 0 };
    SetConsoleCursorPosition(hConsole, topLeft);
    window.set_font_settings(15, 15);
    string command_name, args;

    do {
    // Handle console input
    cout << "\x1b[38;2;255;255;255m";
    cout << "> ";
    cin >> command_name;

    //  Identify and execute command
    if (command_name == "open") {
        //  Opening file
        if (confirm_unsaved_file()) {
            cin >> args;
            string conct = crdir + args;
            char file_name[101];
            strncpy(file_name, conct.c_str(), conct.size());
            ifstream in(file_name);
            if (!(in.is_open())) {
                cout << "[CONSOLE] Could not find specified file \"" + conct << "\"\n";
            } else {
                if (canvas) delete canvas;
                canvas = new Sprite(file_name, 1);
                cout << "[CONSOLE] Successfully opened file.\n";
                crfile = args;
                in.close();
            }
        }
        continue;
    }

    if (command_name == "create") {
        //  Opening file
        if (confirm_unsaved_file()) {
            cin >> args;
            string conct = crdir + args;
            char file_name[101];
            strncpy(file_name, conct.c_str(), conct.size());
            if (canvas) delete canvas;
            canvas = new Sprite(1);
            ofstream out(file_name);
            if (!(out.is_open())) { cout << "[CONSOLE] Could not create the file. Aborting...\n"; continue; }
            unsigned int szinput;
            cin >> szinput;
            canvas->nr_of_frames = szinput;
            cin >> szinput;
            canvas->frame_height = szinput;
            cin >> szinput;
            canvas->frame_width = szinput;
            new_canvas();
            cout << "[CONSOLE] Successfully created file.\n";
            crfile = args;
        }

        continue;
    }

    if (command_name == "saveas") {
        string file_name, cnct;
        cin >> file_name;
        cnct = crdir + file_name;
        save_to(cnct);
        continue;
    }

    if (command_name == "cd") {
        cin >> crdir;
        if ((crdir[0] == '/') && (crdir.size() == 1)) crdir = "";
        continue;
    }

    if (command_name == "setchar") {
        string input;
        cin >> input;
        if (input.size() > 1) brush.sprite_frames[0][0][0] = ' ';
        else brush.sprite_frames[0][0][0] = input[0];
        continue;
    }

    if (command_name == "color") {
        cin >> brush.r[0][0][0] >> brush.g[0][0][0] >> brush.b[0][0][0];
        if (brush.r[0][0][0] > 255 || brush.g[0][0][0] > 255 || brush.b[0][0][0] > 255) 
        cout << "[CONSOLE] Invalid rgb code.\n";
        continue;
    }

    if (command_name == "save") {
        string cnct;
        cnct = crdir + crfile;
        save_to(cnct);
        continue;
    }

    if (command_name == "setframe") {
        cin >> canvas->current_frame;
        if (canvas->current_frame >= canvas->nr_of_frames) canvas->current_frame = 0;
        continue;
    }

    if (command_name == "esc") if (confirm_unsaved_file()) exit(0);

    } while (command_name != "exit");

    unsigned int font_h = round((double) window.screen_height / FONT_RATIO_HEIGHT);
    unsigned int font_w = round((double) window.screen_width / FONT_RATIO_LENGTH);
    window.set_font_settings(font_h, font_w);
}

void loop () {
    unordered_set<char> input = window.get_keys_pressed();
    //  Handle brush movement
    pair<int, int> coords;
    coords = make_pair(brush.x, brush.y);
    if (input.count('w')) brush.y--;
    if (input.count('a')) brush.x--;
    if (input.count('s')) brush.y++;
    if (input.count('d')) brush.x++;

    if (canvas->frame_height <= brush.y || canvas->frame_width <= brush.x) {
        brush.x = coords.first;
        brush.y = coords.second;
    }

    //  Handle commands
    if (input.count('i')) get_command();
    if (input.count('p')) {
        canvas->sprite_frames[c_frame][brush.y][brush.x] = brush.sprite_frames[0][0][0];
        canvas->r[c_frame][brush.y][brush.x] = brush.r[0][0][0];
        canvas->g[c_frame][brush.y][brush.x] = brush.g[0][0][0];
        canvas->b[c_frame][brush.y][brush.x] = brush.b[0][0][0];
        saved = false;
    }

    window.empty_keys_pressed();
}

int main() { 
    brush.stage = 1;
    window.add_sprite_to_renderer(&brush);
    window.add_sprite_to_renderer(canvas);
    get_command();
    window.game_loop(loop);
    return 0;
}