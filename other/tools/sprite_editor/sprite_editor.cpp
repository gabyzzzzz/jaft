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

struct rectangle {
    unsigned int x1, y1, x2, y2;
};

Sprite brush("brush.spr", 0); //    The literal brush you use for "painting"
Sprite* canvas = new Sprite(1);
Window window;

bool saved = true;
string crdir = "";
string crfile = "";

vector<string> tokenize_input(string input) {
    vector<string> ret_val;
    string c_token = "";
    unsigned int i = 0, sz = input.size();
    while (i < sz) {
        if (input[i] == ' ') {
            ret_val.push_back(c_token);
            c_token = "";
        } else c_token += input[i];
        i++;
    }
    return ret_val;
}

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
    saved = true;
}

rectangle get_drawing() {
    unsigned int y_limit = canvas->frame_height, x_limit = canvas->frame_width;
    unsigned int cf = canvas->current_frame;

    rectangle rt_val = {0, 0, 0, 0};

    for (unsigned int y = 0; y < y_limit; y++) 
    for (unsigned int x = 0; x < x_limit; x++) 
    if (canvas->sprite_frames[cf][y][x] != ' ') {
        rt_val.y1 = y;
        goto done1;
    }
    done1:
    
    for (unsigned int x = 0; x < x_limit; x++) 
    for (unsigned int y = 0; y < y_limit; y++) 
    if (canvas->sprite_frames[cf][y][x] != ' ') {
        rt_val.x1 = x;
        goto done2;
    }
    done2:

    for (int x = x_limit - 1; x >= 0; x--) 
    for (unsigned int y = 0; y < y_limit; y++) 
    if (canvas->sprite_frames[cf][y][x] != ' ') {
        rt_val.x2 = x;
        goto done3;
    } 
    done3:

    for (int y = y_limit - 1; y >= 0; y--) 
    for (unsigned int x = 0; x < x_limit; x++) 
    if (canvas->sprite_frames[cf][y][x] != ' ') {
        rt_val.y2 = y;
        goto done4;
    }
    done4:

    return rt_val;
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
    string input;
    vector<string> tokens;
    do {
    // Handle console input
    cout << "\x1b[38;2;255;255;255m";
    cout << "> ";
    cin >> input;
    tokens = tokenize_input(input);
    
    //  Identify and execute command
    if (tokens[0] == "open") {
        //  Opening file
        if (confirm_unsaved_file()) {
            //  Set current path
            string conct = crdir + tokens[1];
            char file_name[101];
            strncpy(file_name, conct.c_str(), conct.size());
            ifstream in(file_name);
            if (!(in.is_open())) {
                cout << "[CONSOLE] Could not find specified file \"" + conct << "\"\n";
            } else {
                if (canvas) delete canvas;
                canvas = new Sprite(file_name, 1);
                cout << "[CONSOLE] Successfully opened file.\n";
                crfile = tokens[1];
                in.close();
            }
        }
        continue;
    }

    if (tokens[0] == "create") {
        //  Opening file
        if (confirm_unsaved_file()) {
            string conct = crdir + tokens[1];
            char file_name[101];
            strncpy(file_name, conct.c_str(), conct.size());
            if (canvas) delete canvas;
            canvas = new Sprite(1);
            ofstream out(file_name);
            if (!(out.is_open())) { cout << "[CONSOLE] Could not create the file. Aborting...\n"; continue; }
            canvas->nr_of_frames = stoi(tokens[1]);
            canvas->frame_height = stoi(tokens[2]);
            canvas->frame_width = stoi(tokens[3]);
            new_canvas();
            cout << "[CONSOLE] Successfully created file.\n";
            crfile = tokens[1];
        }

        continue;
    }

    if (tokens[0] == "saveas") {
        string cnct = crdir + tokens[1];
        save_to(cnct);
        continue;
    }

    if (tokens[0] == "cd") {
        crdir = tokens[1];
        if ((crdir[0] == '/') && (crdir.size() == 1)) crdir = "";
        continue;
    }

    if (tokens[0] == "setchar") {
        if (tokens[1] == "32") brush.sprite_frames[0][0][0] = ' ';
        else brush.sprite_frames[0][0][0] = tokens[1][0];
        continue;
    }

    if (tokens[0] == "color") {
        brush.r[0][0][0] = stoi(tokens[1]);
        brush.g[0][0][0] = stoi(tokens[2]);
        brush.b[0][0][0] = stoi(tokens[3]);
        if (brush.r[0][0][0] > 255 || brush.g[0][0][0] > 255 || brush.b[0][0][0] > 255) 
        cout << "[CONSOLE] Invalid rgb code.\n";
        continue;
    }

    if (tokens[0] == "save") {
        string cnct;
        cnct = crdir + crfile;
        save_to(cnct);
        continue;
    }

    if (tokens[0] == "setframe") {
        canvas->current_frame = stoi(tokens[1]);
        if (canvas->current_frame >= canvas->nr_of_frames) canvas->current_frame = 0;
        continue;
    }

    if (tokens[0] == "align") {
        rectangle to_copy = get_drawing();
        unsigned int cf = canvas->current_frame;
        for (int y = to_copy.y1; y <= to_copy.y2; y++) {
            for (int x = to_copy.x1; x <= to_copy.x2; x++) {
                canvas->sprite_frames[cf][y - to_copy.y1][x - to_copy.x1] = canvas->sprite_frames[cf][y][x];
                canvas->r[cf][y - to_copy.y1][x - to_copy.x1] = canvas->r[cf][y][x];
                canvas->g[cf][y - to_copy.y1][x - to_copy.x1] = canvas->g[cf][y][x];
                canvas->b[cf][y - to_copy.y1][x - to_copy.x1] = canvas->b[cf][y][x];
                canvas->sprite_frames[cf][y][x] = ' ';
            }
        }
        continue;
    }

    if (tokens[0] == "astr") {
        brush.visible = false;
        canvas->is_animation_active = true;
        continue;
    }

    if (tokens[0] == "astp") {
        brush.visible = true;
        canvas->is_animation_active = false;
        continue;
    }

    if (tokens[0] == "tpf") {
        int value = stoi(tokens[1]);
        if (value <= 0) { cout << "[CONSOLE] Ticks per frame must be greater than 0.\n"; continue; }
        canvas->ticks_per_frame = value;
        continue;
    }

    if (tokens[0] == "esc") if (confirm_unsaved_file()) exit(0);

    } while (tokens[0] != "exit");

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
    if (input.count('p') && brush.visible) {
        unsigned int c_frame = canvas->current_frame;
        canvas->sprite_frames[c_frame][brush.y][brush.x] = brush.sprite_frames[0][0][0];
        canvas->r[c_frame][brush.y][brush.x] = brush.r[0][0][0];
        canvas->g[c_frame][brush.y][brush.x] = brush.g[0][0][0];
        canvas->b[c_frame][brush.y][brush.x] = brush.b[0][0][0];
        saved = false;
    }

    window.empty_keys_pressed();
    if (canvas->is_animation_active) canvas->next_game_tick();
}

int main() { 
    brush.transparent_white_spaces = false;
    brush.stage = 1;
    window.add_sprite_to_renderer(&brush);
    window.add_sprite_to_renderer(canvas);
    get_command();
    window.game_loop(loop);
    return 0;
}
