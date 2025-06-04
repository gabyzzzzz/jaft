//       _-------------------------------------------------------------------------_
//      | THIS IS A TOOL BUILT FOR CREATING AND EDITING SPRITE MODELS.              |
//      | ANYTHING CREATED HERE IS NOT GUARANTEED TO BE SUITED FOR FUTURE VERSIONS. |
//      -----------------------------------------------------------------------------

//       _--------------------------------------------_
//      | FEEL FREE TO USE THIS FOR TESTING PURPOSES  |
//      ----------------------------------------------


#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"
#include <vector>
#include <deque>

using namespace std;


//  ------------------------- VARIABLES ----------------------------------------

const int stack_limit = 50;
struct change {
    unsigned int x, y, r, g, b, c_frame;
    char c;
};
vector<change> changes;
deque<vector<change>> undo_stack, redo_stack; 

const double FONT_WIDTH_MULTIPLIER = 2.35;

//  P( x2, y1 ) -> Upper right corner
//  P( x1, y2 ) -> lower left corner
struct rectangle {
    unsigned int x1, y1, x2, y2;
};

Sprite brush(0); //    The literal brush you use for "painting"
Sprite* canvas = new Sprite(1);
Window window;

POINT p;

bool saved = true;
string crdir = "";
string crfile = "";

Sprite clipboard(2);

//  ----------------------------------------------------------------------------

//  Commits change to undo_stack and clears vector
void push_undo(vector<change>& mods) {
    if (!mods.empty()) {
        if (undo_stack.size() >= stack_limit) undo_stack.pop_back();
        undo_stack.push_front(mods);
        mods.clear();
    }
}

//  Commits change to redo_stack and clears vector
void push_redo(vector<change>& mods) {
    if (!mods.empty()) {
        if (redo_stack.size() >= stack_limit) redo_stack.pop_back();
        redo_stack.push_front(mods);
        mods.clear();
    }
}

void copy_chunk_from_sprite(rectangle to_copy, unsigned int x_to_paste, unsigned int y_to_paste, Sprite* target, Sprite* source) {
    unsigned int ydiff = to_copy.y2 - to_copy.y1 + 1;
    unsigned int xdiff = to_copy.x2 - to_copy.x1 + 1;
    unsigned int c_frame = target->current_frame;
    vector<change> mods;
    for (unsigned int y = 0; y < ydiff && y + y_to_paste < WINDOWHEIGHT; y++) {
        for (unsigned int x = 0; x < xdiff && x + x_to_paste < WINDOWLENGTH; x++) {
            //char clr[21] = "\x1b[38;2;<r>;<g>;<b>m<char>";
            change p1 = {
                x + x_to_paste,
                y + y_to_paste,
                target->r[c_frame][y + y_to_paste][x + x_to_paste],
                target->g[c_frame][y + y_to_paste][x + x_to_paste],
                target->b[c_frame][y + y_to_paste][x + x_to_paste],
                c_frame,
                target->sprite_frames[c_frame][y + y_to_paste][x + x_to_paste]
            };
            mods.push_back(p1);
            target->sprite_frames[c_frame][y + y_to_paste][x + x_to_paste] = source->sprite_frames[source->current_frame][y + to_copy.y1][x + to_copy.x1];
            target->r[c_frame][y + y_to_paste][x + x_to_paste] = source->r[source->current_frame][y + to_copy.y1][x + to_copy.x1];
            target->g[c_frame][y + y_to_paste][x + x_to_paste] = source->g[source->current_frame][y + to_copy.y1][x + to_copy.x1];
            target->b[c_frame][y + y_to_paste][x + x_to_paste] = source->b[source->current_frame][y + to_copy.y1][x + to_copy.x1];
        }
    }
    push_undo(mods);
}

//  Copy frames
void f_copy(unsigned int frame_to_copy, unsigned int frame_to_paste) {
    vector<change> mods;
    for (unsigned int y = 0; y < canvas->frame_height; y++) {
        for (unsigned int x = 0; x < canvas->frame_width; x++) {
            change p1 = {
                x,
                y,
                canvas->r[frame_to_paste][y][x],
                canvas->g[frame_to_paste][y][x],
                canvas->b[frame_to_paste][y][x],
                frame_to_paste,
                canvas->sprite_frames[frame_to_paste][y][x]
            };
            canvas->sprite_frames[frame_to_paste][y][x] = canvas->sprite_frames[frame_to_copy][y][x];
            canvas->r[frame_to_paste][y][x] = canvas->r[frame_to_copy][y][x];
            canvas->g[frame_to_paste][y][x] = canvas->g[frame_to_copy][y][x];
            canvas->b[frame_to_paste][y][x] = canvas->b[frame_to_copy][y][x];
            mods.push_back(p1);
        }
    }
    push_undo(mods);
}

//  Undo
void undo() {
    if (undo_stack.empty()) return;
    vector<change> und = undo_stack.front();
    vector<change> red;
    int sz = und.size();
    unsigned int x, y, c_frame;
    for (int i = 0; i < sz; i++) {
        x = und[i].x;
        y = und[i].y;
        c_frame = und[i].c_frame;

        red.push_back({x, y, 
        canvas->r[c_frame][y][x],
        canvas->g[c_frame][y][x],
        canvas->b[c_frame][y][x],
        c_frame, 
        canvas->sprite_frames[c_frame][y][x]});

        canvas->sprite_frames[c_frame][y][x] = und[i].c;
        canvas->r[c_frame][y][x] = und[i].r;
        canvas->g[c_frame][y][x] = und[i].g;
        canvas->b[c_frame][y][x] = und[i].b;
    }
    if (redo_stack.size() >= stack_limit) redo_stack.pop_back();
    redo_stack.push_front(red);
    undo_stack.pop_front();
}

//  Redo
void redo () {
    if (redo_stack.empty()) return;
    vector<change> red = redo_stack.front();
    vector<change> und;
    int sz = red.size();
    unsigned int x, y, c_frame;
    for (int i = 0; i < sz; i++) {
        x = red[i].x;
        y = red[i].y;
        c_frame = red[i].c_frame;

        und.push_back({x, y, 
        canvas->r[c_frame][y][x],
        canvas->g[c_frame][y][x],
        canvas->b[c_frame][y][x],
        c_frame, 
        canvas->sprite_frames[c_frame][y][x]});

        canvas->sprite_frames[c_frame][y][x] = red[i].c;
        canvas->r[c_frame][y][x] = red[i].r;
        canvas->g[c_frame][y][x] = red[i].g;
        canvas->b[c_frame][y][x] = red[i].b;
    }
    if (undo_stack.size() >= stack_limit) undo_stack.pop_back();
    undo_stack.push_front(und);
    redo_stack.pop_front();
}

vector<string> tokenize_input(string input) {
    vector<string> ret_val;
    string t;
    int sz = input.size();
    for (int i = 0; i < sz; i++) {
        if (input[i] == ' ') {
            if (!t.empty()) ret_val.push_back(t);
            t.clear();
        } else if (isprint(input[i])) t += input[i];
    }
    if (!t.empty()) ret_val.push_back(t);
    if (!ret_val.empty()) return ret_val;
    else return {""};
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

rectangle get_drawing(Sprite* source) {
    unsigned int y_limit = source->frame_height, x_limit = source->frame_width;
    unsigned int cf = source->current_frame;
    rectangle rt_val = {0, 0, 0, 0};
    for (unsigned int y = 0; y < y_limit; y++) 
    for (unsigned int x = 0; x < x_limit; x++) 
    if (source->sprite_frames[cf][y][x] != ' ') {
        rt_val.y1 = y;
        goto done1;
    }
    done1:
    for (unsigned int x = 0; x < x_limit; x++) 
    for (unsigned int y = 0; y < y_limit; y++) 
    if (source->sprite_frames[cf][y][x] != ' ') {
        rt_val.x1 = x;
        goto done2;
    }
    done2:
    for (int x = x_limit - 1; x >= 0; x--) 
    for (unsigned int y = 0; y < y_limit; y++) 
    if (source->sprite_frames[cf][y][x] != ' ') {
        rt_val.x2 = x;
        goto done3;
    } 
    done3:
    for (int y = y_limit - 1; y >= 0; y--) 
    for (unsigned int x = 0; x < x_limit; x++) 
    if (source->sprite_frames[cf][y][x] != ' ') {
        rt_val.y2 = y;
        goto done4;
    }
    done4:

    return rt_val;
}

void sprite_init(Sprite* spr) {
    spr->sprite_frames = new char**[spr->nr_of_frames];
    spr->r = new unsigned short int**[spr->nr_of_frames];
    spr->g = new unsigned short int**[spr->nr_of_frames];
    spr->b = new unsigned short int**[spr->nr_of_frames];
    for (unsigned int f = 0; f < spr->nr_of_frames; f++) {
        spr->sprite_frames[f] = new char*[spr->frame_height];
        spr->r[f] = new unsigned short int*[spr->frame_height];
        spr->g[f] = new unsigned short int*[spr->frame_height];
        spr->b[f] = new unsigned short int*[spr->frame_height];
        for (unsigned int h = 0; h < spr->frame_height; h++) {
            spr->sprite_frames[f][h] = new char[spr->frame_width + 1];
            spr->r[f][h] = new unsigned short int[spr->frame_width];
            spr->g[f][h] = new unsigned short int[spr->frame_width];
            spr->b[f][h] = new unsigned short int[spr->frame_width];
            for (unsigned int w = 0; w < spr->frame_width; w++) {
                spr->sprite_frames[f][h][w] = ' ';
                spr->r[f][h][w] = 255;
                spr->g[f][h][w] = 255;
                spr->b[f][h][w] = 255;
            }
            spr->sprite_frames[f][h][spr->frame_width] = '\0';
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
    cout << "\x1b[38;2;255;255;255m";

    string input;
    vector<string> tokens;

    do {
    // Handle console input
    cout << "> ";
    getline(cin, input);
    tokens = tokenize_input(input);

    if (tokens.size() == 0) continue;
    
    //  Identify and execute command
    if (tokens[0] == "open") {
        //  Opening file
        if (tokens.size() < 2) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        if (confirm_unsaved_file()) {
            //  Set current path
            string conct = crdir + tokens[1];
            char file_name[101];
            strncpy(file_name, conct.c_str(), conct.size());
            if (conct.size() < 101) file_name[conct.size()] = '\0';
            else file_name[100] = '\0';
            ifstream in(file_name);
            if (!(in.is_open())) {
                cout << "[CONSOLE] Could not find specified file \"" + conct << "\"\n";
            } else {
                redo_stack.clear();
                undo_stack.clear();
                delete canvas;
                canvas = nullptr;
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
        if (tokens.size() < 3 || (tokens.size() > 3 && tokens.size() < 5)) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        if (confirm_unsaved_file()) {
            string conct = crdir + tokens[1];
            char file_name[101];
            strncpy(file_name, conct.c_str(), conct.size());
            if (conct.size() < 101) file_name[conct.size()] = '\0';
            else file_name[100] = '\0';
            ofstream out(file_name);
            if (!(out.is_open())) { cout << "[CONSOLE] Could not create the file. Aborting...\n"; continue; }
            redo_stack.clear();
            undo_stack.clear();
            delete canvas;
            canvas = nullptr;
            canvas = new Sprite(1);
            canvas->nr_of_frames = stoi(tokens[2]);
            if (tokens.size() > 3) {
                canvas->frame_height = stoi(tokens[3]);
                canvas->frame_width = stoi(tokens[4]);
            } else {
                canvas->frame_height = WINDOWHEIGHT;
                canvas->frame_width = WINDOWLENGTH; 
            }
            sprite_init(canvas);
            cout << "[CONSOLE] Successfully created file.\n";
            crfile = tokens[1];
        }

        continue;
    }

    if (tokens[0] == "saveas") {
        if (tokens.size() < 2) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        string cnct = crdir + tokens[1];
        save_to(cnct);
        continue;
    }

    if (tokens[0] == "cd") {
        if (tokens.size() < 2) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        crdir = tokens[1];
        if ((crdir[0] == '/') && (crdir.size() == 1)) crdir = "";
        continue;
    }

    if (tokens[0] == "setchar") {
        if (tokens.size() < 2) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        if (tokens[1] == "32") brush.sprite_frames[0][0][0] = ' ';
        else brush.sprite_frames[0][0][0] = tokens[1][0];
        continue;
    }

    if (tokens[0] == "color") {
        if (tokens.size() < 4) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        brush.r[0][0][0] = stoi(tokens[1]);
        brush.g[0][0][0] = stoi(tokens[2]);
        brush.b[0][0][0] = stoi(tokens[3]);
        if (brush.r[0][0][0] > 256 || brush.g[0][0][0] > 256 || brush.b[0][0][0] > 256) 
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
        if (tokens.size() < 2) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        canvas->current_frame = stoi(tokens[1]);
        if (canvas->current_frame >= canvas->nr_of_frames) canvas->current_frame = 0;
        continue;
    }

    if (tokens[0] == "align") {
        rectangle to_copy = get_drawing(canvas);
        vector<change> mods;
        unsigned int cf = canvas->current_frame;
        for (unsigned int y = to_copy.y1; y <= to_copy.y2; y++) {
            for (unsigned int x = to_copy.x1; x <= to_copy.x2; x++) {
                change p1 = {
                    x, 
                    y, 
                    canvas->r[cf][y][x], 
                    canvas->g[cf][y][x], 
                    canvas->b[cf][y][x], 
                    cf, 
                    canvas->sprite_frames[cf][y][x]
                };
                change p2 = {
                    x - to_copy.x1, 
                    y - to_copy.y1, 
                    canvas->r[cf][y - to_copy.y1][x - to_copy.x1], 
                    canvas->g[cf][y - to_copy.y1][x - to_copy.x1], 
                    canvas->b[cf][y - to_copy.y1][x - to_copy.x1],
                    cf,
                    canvas->sprite_frames[cf][y][x]
                };
                canvas->sprite_frames[cf][y - to_copy.y1][x - to_copy.x1] = canvas->sprite_frames[cf][y][x];
                canvas->r[cf][y - to_copy.y1][x - to_copy.x1] = canvas->r[cf][y][x];
                canvas->g[cf][y - to_copy.y1][x - to_copy.x1] = canvas->g[cf][y][x];
                canvas->b[cf][y - to_copy.y1][x - to_copy.x1] = canvas->b[cf][y][x];
                canvas->sprite_frames[cf][y][x] = ' ';
                mods.push_back(p1); mods.push_back(p2);
            }
        }
        push_undo(mods);
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
        if (tokens.size() < 2) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        int value = stoi(tokens[1]);
        if (value <= 0) { cout << "[CONSOLE] Ticks per frame must be greater than 0.\n"; continue; }
        canvas->ticks_per_frame = value;
        continue;
    }

    if (tokens[0] == "copyframe") {
        if (tokens.size() < 2) { cout << "[CONSOLE] Not enough arguments!\n"; continue; }
        f_copy(stoi(tokens[1]), canvas->current_frame);
        continue;
    }

    if (tokens[0] == "esc") if (confirm_unsaved_file()) exit(0);

    if (tokens[0] != "exit") cout << "[CONSOLE] Invalid command \"" + tokens[0] + "\"\n";

    } while (tokens[0] != "exit");

    window.remove_sprites_from_renderer([](Sprite* d){
        if (d->label == 1) return true;
        else return false;
    });
    window.add_sprite_to_renderer(canvas);
    brush.x = 0; brush.y = 0;

    unsigned int font_h = round((double) window.screen_height / FONT_RATIO_HEIGHT);
    unsigned int font_w = round((double) window.screen_width / FONT_RATIO_LENGTH);
    window.set_font_settings(font_h, font_w);
}


//  -----------------------------------CHATGPT CODE DONT TOUCH IT----------------------------------------

HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
INPUT_RECORD inputRecord;
DWORD events;

void setup_input() {
    DWORD mode;
    GetConsoleMode(hInput, &mode);
    mode |= ENABLE_MOUSE_INPUT;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(hInput, mode);
}

bool left_mouse_down() {
    DWORD numEvents = 0;
    GetNumberOfConsoleInputEvents(hInput, &numEvents);
    if (numEvents == 0) return false; 
    PeekConsoleInput(hInput, &inputRecord, 1, &events);
    if (inputRecord.EventType == MOUSE_EVENT) {
        MOUSE_EVENT_RECORD& mouse = inputRecord.Event.MouseEvent;
        if (mouse.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            ReadConsoleInput(hInput, &inputRecord, 1, &events); 
            return true;
        }
    }
    ReadConsoleInput(hInput, &inputRecord, 1, &events);
    return false;
}

//  -----------------------------------------------------------------------------------------------------

//   _______
//  | LOOP |
//   

int s_mode = 0;
rectangle selection;

void loop () {
    //  TODO - GET RID OF THE MOUSE CLICKS AT COPYING. ITS SLOWING THINGS DOWN. ALSO, IMPLEMENT THE CUT
    unordered_set<char> input = window.get_keys_pressed();
    
    //  Handle brush movement to follow mouse
    GetCursorPos(&p);
    unsigned int x = (double)p.x / ((double) window.screen_width / FONT_RATIO_LENGTH) * FONT_WIDTH_MULTIPLIER;
    unsigned int y = (double)p.y / ((double) window.screen_height / FONT_RATIO_HEIGHT);
    if (canvas->frame_height > y && canvas->frame_width > x) {
        brush.x = x;
        brush.y = y;
    }
    //  If selection mode active, handle coords
    if (s_mode) {
        selection.x2 = brush.x;
        selection.y2 = brush.y;
    }
    //  Handle commands
    if (input.count('q')) get_command();
    if (input.count('z')) undo();
    if (input.count('y')) redo();

    if (input.count('c')) {
        if (s_mode) {
            brush.visible = true;
            rectangle cpy = {
                min(selection.x1, selection.x2), 
                min(selection.y1, selection.y2), 
                max(selection.x1, selection.x2), 
                max(selection.y1, selection.y2)
            };
            //  Empty clipboard
            for (int y = 0; y < WINDOWHEIGHT; y++) 
            for (int x = 0; x < WINDOWLENGTH; x++) 
            clipboard.sprite_frames[0][y][x] = ' ';
            //  Copy chunk
            copy_chunk_from_sprite(cpy, 0, 0, &clipboard, canvas);
            //  Delete chars from canvas if is in cut mode
            if (s_mode == 2) {
                vector<change> mods;
                for (unsigned y = cpy.y1; y <= cpy.y2; y++) 
                for (unsigned x = cpy.x1; x <= cpy.x2; x++) {
                    change p1 = {x, y, 
                        canvas->r[canvas->current_frame][y][x],
                        canvas->g[canvas->current_frame][y][x],
                        canvas->b[canvas->current_frame][y][x],
                        canvas->current_frame, 
                        canvas->sprite_frames[canvas->current_frame][y][x]
                    };
                    canvas->sprite_frames[canvas->current_frame][y][x] = ' ';
                    mods.push_back(p1);
                }
                Sleep(1000);
                push_undo(mods);
            }
            //  Turn off selection mode
            s_mode = 0;
        } else {
            brush.visible = false;
            s_mode = 1;
            selection = {brush.x, brush.y, brush.x, brush.y};
        }
    }

    if (input.count('x')) {
        s_mode = 2;
        selection = {brush.x, brush.y, brush.x, brush.y};
    }

    if (input.count('p')) {
        rectangle to_copy = get_drawing(&clipboard);
        copy_chunk_from_sprite(to_copy, brush.x, brush.y, canvas, &clipboard);
    }

    //  On click
    if (left_mouse_down() && brush.visible && !s_mode) {
        //  Clear stack
        redo_stack.clear();
        unsigned int c_frame = canvas->current_frame;
        //  Register changes
        change current_change = {brush.x, brush.y, 
        canvas->r[c_frame][brush.y][brush.x],
        canvas->g[c_frame][brush.y][brush.x],
        canvas->b[c_frame][brush.y][brush.x],
        c_frame, 
        canvas->sprite_frames[c_frame][brush.y][brush.x]};
        change tmp;
        if (!changes.empty()) tmp = changes[changes.size() - 1];

        //  Write and push changes
        if (changes.empty() || tmp.x != current_change.x || tmp.y != current_change.y) {
            changes.push_back(current_change);
            canvas->sprite_frames[c_frame][brush.y][brush.x] = brush.sprite_frames[0][0][0];
            canvas->r[c_frame][brush.y][brush.x] = brush.r[0][0][0];
            canvas->g[c_frame][brush.y][brush.x] = brush.g[0][0][0];
            canvas->b[c_frame][brush.y][brush.x] = brush.b[0][0][0];
            saved = false;
        }
    } 
    else if (!s_mode && brush.visible) push_undo(changes);

    //  Mark rectangle selected
    if (s_mode) {
        rectangle tmp = {
            min(selection.x1, selection.x2), 
            min(selection.y1, selection.y2), 
            max(selection.x1, selection.x2), 
            max(selection.y1, selection.y2)
        };
        for (unsigned int y = tmp.y1; y <= tmp.y2; y++) {
            for (unsigned int x = tmp.x1; x <= tmp.x2; x++) {
                char* c_str = window.buffer + (y * (WINDOWLENGTH + 1) + x) * 20;
                //char clr[21] = "\x1b[38;2;<r>;<g>;<b>m<char>";
                c_str[19] = '.';
                memcpy(c_str + 7,  "57", 3);
                memcpy(c_str + 11, "57", 3);
                memcpy(c_str + 15, "57", 3);
            }
        }
    }

    window.empty_keys_pressed();
    if (canvas->is_animation_active) canvas->next_game_tick();
}

//   ________
//  | Setup |
//

int main() { 
    //  Mouse input setup
    setup_input();

    //  Check brush file before init
    const char brush_file_name[] = "brush.spr";
    ifstream in(brush_file_name);
    if (in.is_open()) {
        in.close();
        brush.init_by_file(brush_file_name);
    } else cout << "[CONSOLE] Unable to open brush.spr\n";

    //  Setup
    brush.transparent_white_spaces = false;
    brush.stage = 1;
    clipboard.nr_of_frames = 1;
    clipboard.frame_height = WINDOWHEIGHT;
    clipboard.frame_width = WINDOWLENGTH;
    clipboard.visible = false;
    sprite_init(&clipboard);
    window.add_sprite_to_renderer(&brush);
    get_command();
    window.game_loop(loop);

    cin.get();
    return 0;
}