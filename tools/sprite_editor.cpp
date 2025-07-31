//       ___________________________________________________________________________
//      | THIS IS A TOOL BUILT FOR CREATING AND EDITING SPRITE MODELS.              |
//      | ANYTHING CREATED HERE IS NOT GUARANTEED TO BE SUITED FOR FUTURE VERSIONS. |

//       ____________________________________________
//      | FEEL FREE TO USE THIS FOR TESTING PURPOSES |

#include "../lib/jaft.h"
#include <vector>
#include <deque>
#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>

using namespace jaft;
//   ____________________
//  | VARIABLES / OTHERS |

const int stack_limit = 50;
struct change {
    POINT_e coords;
    CELL cell;
    unsigned int c_frame;
};
std::vector<change> changes;
std::deque<std::vector<change>> undo_stack, redo_stack; 

const double FONT_WIDTH_MULTIPLIER = 2.295; //   FONTHEIGHT / FONTWIDTH
const double FONT_HEIGHT_MULTIPLIER = 0.98;

//  P( x2, y1 ) -> Upper right corner
//  P( x1, y2 ) -> lower left corner
struct rectangle {
    unsigned int x1, y1, x2, y2;
};

struct {
    std::string r = "0", g = "0", b = "0";
} global_background_color;

Sprite brush(0); //    The literal brush you use for "painting"
Sprite* canvas = new Sprite(1);
Window window;

POINT p;

bool saved = true;
std::string crdir = "";
std::string crfile = "";

Sprite clipboard(2);
Sprite selection_s(3);

COLOR current_palette[60];
std::string palette_dir;

//   _______________________
//  | FUNCTIONS / UTILITIES |

inline void reset_color() {
    std::cout << "\x1b[38;2;255;255;255m" << std::flush;
}

//  Commits change to undo_stack and clears vector
void push_undo(std::vector<change>& mods) {
    if (!mods.empty()) {
        if (undo_stack.size() >= stack_limit) undo_stack.pop_back();
        undo_stack.push_front(mods);
        mods.clear();
    }
}

//  Commits change to redo_stack and clears vector
void push_redo(std::vector<change>& mods) {
    if (!mods.empty()) {
        if (redo_stack.size() >= stack_limit) redo_stack.pop_back();
        redo_stack.push_front(mods);
        mods.clear();
    }
}

//  Copies rectangle from a sprite to another
void copy_chunk_from_sprite(rectangle to_copy, unsigned int x_to_paste, unsigned int y_to_paste, Sprite* target, Sprite* source) {
    //  Variables
    POINT_e diff = {to_copy.x2 - to_copy.x1 + 1, to_copy.y2 - to_copy.y1 + 1};
    POINT_e frames = {target->get_animation().current_frame, source->get_animation().current_frame};
    std::vector<change> mods;
    for (unsigned int y = 0; y < diff.y && y + y_to_paste < WINDOWHEIGHT; y++) {
        for (unsigned int x = 0; x < diff.x && x + x_to_paste < WINDOWLENGTH; x++) {
            //char clr[21] = "\x1b[38;2;<r>;<g>;<b>m<char>";
            //  Register and push changes
            POINT_e crd = { x + x_to_paste, y + y_to_paste };
            change p1 = { crd, target->frames[frames.x][crd.y][crd.x], frames.x };
            mods.push_back(p1);
            target->frames[frames.x][crd.y][crd.x] = source->frames[frames.y][y + to_copy.y1][x + to_copy.x1];
        }
    }
    push_undo(mods);
    canvas->refresh();
}

//  Copy frames
void f_copy(unsigned int frame_to_copy, unsigned int frame_to_paste) {
    std::vector<change> mods;
    for (unsigned int y = 0; y < canvas->frame_size.y; y++) {
        for (unsigned int x = 0; x < canvas->frame_size.x; x++) {
             //  Register and push changes
            POINT_e coords = {x, y};
            change p1 = { coords, canvas->frames[frame_to_paste][coords.y][coords.x], frame_to_paste };
            canvas->frames[frame_to_paste][y][x] = canvas->frames[frame_to_copy][y][x];
            mods.push_back(p1);
        }
    }
    push_undo(mods);
    canvas->refresh();
}

//  Undo
void undo() {
    if (undo_stack.empty()) return;
    std::vector<change> und = undo_stack.front();
    std::vector<change> red;
    for (int i = static_cast<int>(und.size()) - 1; i >= 0; --i) {
        unsigned int x = und[i].coords.x;
        unsigned int y = und[i].coords.y;
        unsigned int c_frame = und[i].c_frame;

        POINT_e coords = {x, y};
        change p1 = {coords, canvas->frames[c_frame][y][x], c_frame};
        red.push_back(p1);

        canvas->frames[c_frame][y][x] = und[i].cell;
    }
    if (redo_stack.size() >= stack_limit) redo_stack.pop_back();
    redo_stack.push_front(red);
    undo_stack.pop_front();
    canvas->refresh();
}

//  Redo
void redo () {
    if (redo_stack.empty()) return;
    std::vector<change> red = redo_stack.front();
    std::vector<change> und;
    int sz = red.size();
    unsigned int x, y, c_frame;
    for (int i = 0; i < sz; i++) {
        x = red[i].coords.x;
        y = red[i].coords.y;
        c_frame = red[i].c_frame;

        POINT_e coords = {x, y};
        change p1 = {coords, canvas->frames[c_frame][y][x], c_frame};
        und.push_back(p1);

        canvas->frames[c_frame][y][x] = red[i].cell;
    }
    if (undo_stack.size() >= stack_limit) undo_stack.pop_back();
    undo_stack.push_front(und);
    redo_stack.pop_front();
    canvas->refresh();
}

//  Divides input into tokens separated by spaces
std::vector<std::string> tokenize_input(std::string input) {
    std::vector<std::string> ret_val;
    std::string t;
    int sz = input.size();
    for (int i = 0; i < sz; i++) {
        if (input[i] == ' ') {
            if (!t.empty()) ret_val.push_back(t);
            t.clear();
        } else if (input[i] != '\n' && input[i] != '\t' && input[i] != '\r' && input[i] != '\v' && input[i] != '\f') t += input[i];
    }
    if (!t.empty()) ret_val.push_back(t);
    return ret_val;
}

//  Saves the contents of the file to a file specified in the arguments
void save_to(std::string cnct) {
    std::ofstream out(cnct);
    if (!out.is_open()) {
        std::cout << "[SPRITE_EDITOR] Couldn't open file " << crdir + cnct << '\n';
    }
    out << canvas->frame_size.y << ' ' << canvas->frame_size.x << ' ';
    out << canvas->get_animation().nr_of_frames << ' ' << canvas->get_srenderer().nr_of_colors << ' ';
    for (int i = 0; i < canvas->get_srenderer().nr_of_colors; i++) {
        out << canvas->get_srenderer().palette[i].r << ' ';
        out << canvas->get_srenderer().palette[i].g << ' ';
        out << canvas->get_srenderer().palette[i].b << ' ';
    }
    for (int f = 0; f < canvas->get_animation().nr_of_frames; f++) {
        for (int i = 0; i < canvas->frame_size.y; i++) {
            for (int j = 0; j < canvas->frame_size.x; j++) {
                out << canvas->frames[f][i][j].color_id << ' ';
                out << (int) canvas->frames[f][i][j].character << ' ';
            }
        }
    }
    out.flush();
    std::cout << "[SPRITE_EDITOR] Successfully saved file.\n";
    saved = true;
}

//  Returns a triangle that defines the immage without whitespace
rectangle get_drawing(Sprite* source) {
    unsigned int y_limit = source->frame_size.y;
    unsigned int x_limit = source->frame_size.x;
    unsigned int cf = source->get_animation().current_frame;
    bool found = false;
    rectangle rt_val = {x_limit, y_limit, 0, 0};
    for (unsigned int y = 0; y < y_limit; y++) {
        for (unsigned int x = 0; x < x_limit; x++) {
            if (source->frames[cf][y][x].character != ' ') {
                if (x < rt_val.x1) rt_val.x1 = x;
                if (x > rt_val.x2) rt_val.x2 = x;
                if (y < rt_val.y1) rt_val.y1 = y;
                if (y > rt_val.y2) rt_val.y2 = y;
                found = true;
            }
        }
    }
    if (!found) return {0, 0, 0, 0};
    return rt_val;
}

//  Confirms unsaved file
bool confirm_unsaved() {
    if (!saved) {
        char choice;
        std::cout << "[SPRITE_EDITOR] You didn't save your current work. Do you wish to proceed? y/n \n>_ ";
        std::cin >> choice;
        if (choice != 'y' && choice != 'Y') {
            std::cout << "[SPRITE_EDITOR] Command aborted.\n";
            return false;
        }
    }
    return true;
}

bool valid_color(int color) {
    if (color < 0 || color > 256) return false;
    else return true;
}

//  Main loop on cli
void get_command() {
    //  Clear screen, reset cursor and color
    std::cout << "\x1b[49m" << std::flush;
    std::cout << "\x1b[2J\x1b[H" << std::flush; 
    reset_color();
    window.set_font_settings(15, 15);

    std::string input;
    std::vector<std::string> tokens;

    do {
    //  Handle console input
    std::cout << "> ";
    getline(std::cin, input);
    tokens = tokenize_input(input);
    if (tokens.size() < 1) continue;

    //  Identify and execute command
    if (tokens[0] == "open") {
        //  Opening file
        if (tokens.size() < 2) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        if (confirm_unsaved()) {
            //  Set current path
            std::string conct = crdir + tokens[1];
            char file_name[101];
            strncpy(file_name, conct.c_str(), conct.size());
            if (conct.size() < 101) file_name[conct.size()] = '\0';
            else file_name[100] = '\0';
            std::ifstream in(file_name);
            if (!(in.is_open())) {
                std::cout << "[SPRITE_EDITOR] Could not find specified file \"" + conct << "\"\n";
            } else {
                redo_stack.clear();
                undo_stack.clear();
                delete canvas;
                canvas = nullptr;
                canvas = new Sprite(file_name, 1);
                std::cout << "[SPRITE_EDITOR] Successfully opened file.\n";
                crfile = tokens[1];
                canvas->refresh();
                in.close();
            }
        }
        continue;
    }

    if (tokens[0] == "create") {
        //  Opening file
        if (tokens.size() < 3 || (tokens.size() > 3 && tokens.size() < 5)) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        if (confirm_unsaved()) {
            std::string conct = crdir + tokens[1];
            char file_name[101];
            strncpy(file_name, conct.c_str(), conct.size());
            if (conct.size() < 101) file_name[conct.size()] = '\0';
            else file_name[100] = '\0';
            std::ofstream out(file_name);
            if (!(out.is_open())) { std::cout << "[SPRITE_EDITOR] Could not create the file. Aborting...\n"; continue; }
            redo_stack.clear();
            undo_stack.clear();
            delete canvas;
            canvas = nullptr;
            canvas = new Sprite(1);
            canvas->set_nr_of_frames(stoi(tokens[2]));
            if (tokens.size() > 3) {
                canvas->frame_size.y = stoi(tokens[3]);
                canvas->frame_size.x = stoi(tokens[4]);
            } else {
                canvas->frame_size.y = WINDOWHEIGHT;
                canvas->frame_size.x = WINDOWLENGTH; 
            }
            canvas->set_stage(0);
            canvas->sprite_init();
            std::cout << "[SPRITE_EDITOR] Successfully created file.\n";
            crfile = tokens[1];
        }

        continue;
    }

    if (tokens[0] == "saveas") {
        if (tokens.size() < 2) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        std::string cnct = crdir + tokens[1];
        save_to(cnct);
        continue;
    }

    if (tokens[0] == "cd") {
        if (tokens.size() < 2) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        crdir = tokens[1];
        if ((crdir[0] == '/') && (crdir.size() == 1)) crdir = "";
        continue;
    }

    if (tokens[0] == "setchar") {
        if (tokens.size() < 2) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        if (tokens[1] == "32") brush.frames[0][0][0].character = ' ';
        else brush.frames[0][0][0].character = tokens[1][0];
        brush.refresh();
        continue;
    }

    if (tokens[0] == "color") {
        if (tokens.size() < 2) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        int set_color_id = stoi(tokens[1]);
        if (set_color_id < 0 || set_color_id > MAXNROFCOLORS) {
            std::cout << "[SPRITE_EDITOR] Invalid argument! color_id must be an integer with a value between 0 and " << MAXNROFCOLORS << ".\n";
            continue;
        }
        brush.frames[0][0][0].color_id = set_color_id;
        brush.refresh();
        continue;
    }

    if (tokens[0] == "save") {
        std::string cnct;
        cnct = crdir + crfile;
        save_to(cnct);
        continue;
    }

    if (tokens[0] == "setframe") {
        if (tokens.size() < 2) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        unsigned int cf = stoi(tokens[1]);
        if (cf >= canvas->get_animation().nr_of_frames) canvas->set_current_frame(0);
        else canvas->set_current_frame(cf);
        continue;
    }

    if (tokens[0] == "align") {
        rectangle to_copy = get_drawing(canvas);
        unsigned int cf = canvas->get_animation().current_frame;
        std::vector<change> mods;
        std::vector<std::vector<CELL>> temp;
        unsigned int width = 0, height = 0;
        if (to_copy.x2 >= to_copy.x1 && to_copy.y2 >= to_copy.y1) {
            width = to_copy.x2 - to_copy.x1 + 1;
            height = to_copy.y2 - to_copy.y1 + 1;
            temp.resize(height, std::vector<CELL>(width));
            for (unsigned int y = 0; y < height; y++) {
                for (unsigned int x = 0; x < width; x++) {
                    temp[y][x] = canvas->frames[cf][to_copy.y1 + y][to_copy.x1 + x];
                }
            }
        }

        // Save all changes for undo
        for (unsigned int y = 0; y < canvas->frame_size.y; y++) {
            for (unsigned int x = 0; x < canvas->frame_size.x; x++) {
                POINT_e p = {x, y};
                change c = {p, canvas->frames[cf][y][x], cf};
                mods.push_back(c);
            }
        }

        // Clear the canvas
        for (unsigned int y = 0; y < canvas->frame_size.y; y++) {
            for (unsigned int x = 0; x < canvas->frame_size.x; x++) {
                canvas->frames[cf][y][x].character = ' ';
            }
        }

        // Copy the aligned region to the top-left
        for (unsigned int y = 0; y < height; y++) {
            for (unsigned int x = 0; x < width; x++) {
                canvas->frames[cf][y][x] = temp[y][x];
            }
        }

        push_undo(mods);
        canvas->refresh();
        continue;
    }

    if (tokens[0] == "astr") {
        brush.hide();
        canvas->start_animation();
        continue;
    }

    if (tokens[0] == "astp") {
        brush.show();
        canvas->stop_animation();
        continue;
    }

    if (tokens[0] == "tpf") {
        if (tokens.size() < 2) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        int value = stoi(tokens[1]);
        if (value <= 0) { std::cout << "[SPRITE_EDITOR] Ticks per frame must be greater than 0.\n"; continue; }
        canvas->set_ticks_per_frame(value);
        continue;
    }

    if (tokens[0] == "screensize") {
        std::cout << window.screen_size.y << ' ' << window.screen_size.x << '\n';
        continue;
    }

    if (tokens[0] == "copyframe") {
        if (tokens.size() < 2) { std::cout << "[SPRITE_EDITOR] Not enough arguments!\n"; continue; }
        f_copy(stoi(tokens[1]), canvas->get_animation().current_frame);
        continue;
    }

    //  Shows existing colors to the console. 
    if (tokens[0] == "showcolors") {
        for (int i = 0; i < MAXNROFCOLORS; i++) {
            std::cout << "\x1b[38;2;";
            std::cout << current_palette[i].r << ';';
            std::cout << current_palette[i].g << ';';
            std::cout << current_palette[i].b << 'm';
            std::cout << "[COLOR " << i << "] ";
            std::cout << current_palette[i].r << ' ';
            std::cout << current_palette[i].g << ' ';
            std::cout << current_palette[i].b << '\n';
        }
        reset_color();
        continue;
    }

    //  Sets a color from sprites palette to input
    if (tokens[0] == "setcolor") {
        if (tokens.size() < 5) {
            std::cout << "[SPRITE_EDITOR] Not enough arguments!\n";
            std::cout << "[SPRITE_EDITOR] Arguments for setcolor are: color_id, color.r, color.g, color.b\n";
            continue;
        }
        int color_id = stoi(tokens[1]);
        if (color_id > MAXNROFCOLORS || color_id < 0) {
            std::cout << "[SPRITE_EDITOR] Invalid color id!\n";
            std::cout << "[SPRITE_EDITOR] Color id must be an integer lower than " << MAXNROFCOLORS << " and greater or equal to 0.\n";
            continue;
        }
        COLOR temp = {
            stoi(tokens[2]),
            stoi(tokens[3]),
            stoi(tokens[4])
        };
        if (temp.r < 0 || temp.r > 256) {
            std::cout << "[SPRITE_EDITOR] Invalid color!\n";
            std::cout << "[SPRITE_EDITOR] Every color value must be an integer lower than 257 and greater or equal to 0.\n";
            continue;
        }
        if (temp.g < 0 || temp.b > 256) {
            std::cout << "[SPRITE_EDITOR] Invalid color!\n";
            std::cout << "[SPRITE_EDITOR] Every color value must be an integer lower than 257 and greater or equal to 0.\n";
            continue;
        }
        if (temp.b < 0 || temp.b > 256) {
            std::cout << "[SPRITE_EDITOR] Invalid color!\n";
            std::cout << "[SPRITE_EDITOR] Every color value must be an integer lower than 257 and greater or equal to 0.\n";
            continue;
        }
        current_palette[color_id] = temp;
        std::cout << "[SPRITE_EDITOR] Successfully set color no. " << color_id << " to: ";
        std::cout << "\x1b[38;2;";
        std::cout << current_palette[color_id].r << ';';
        std::cout << current_palette[color_id].g << ';';
        std::cout << current_palette[color_id].b << "m#\n";
        reset_color();
        continue;
    }

    if (tokens[0] == "createpalette") {
        if (tokens.size() < 2) {
            std::cout << "[SPRITE_EDITOR] Not enough arguments!\n";
            std::cout << "[SPRITE_EDITOR] Arguments for createpalette are: name\n";
            continue;
        } 
        std::ofstream out(crdir + tokens[1]);
        if (!out.is_open()) {
            std::cout << "[SPRITE_EDITOR] Couldn't open file " << crdir + tokens[1] << '\n';
            continue;
        }
        out.close();
        if (confirm_unsaved()) {
            COLOR default_color = { 255, 0, 0 };
            for (int i = 0; i < MAXNROFCOLORS; i++) 
                current_palette[i] = default_color;
        }
        palette_dir = crdir + tokens[1];
        std::cout << "[SPRITE_EDITOR] Successfully created new palette with path: " << crdir + tokens[1] << '\n';
        continue;
    }

    if (tokens[0] == "savepalette") {
        if (tokens.size() < 2) {
            std::cout << "[SPRITE_EDITOR] Not enough arguments!\n";
            std::cout << "[SPRITE_EDITOR] Arguments for savepalette are: nr of colors\n";
            continue;
        }
        int palette_size = stoi(tokens[1]);
        if (palette_size < 0 || palette_size > MAXNROFCOLORS) {
            std::cout << "[SPRITE_EDITOR] Invalid nr of colors.\n";
            std::cout << "[SPRITE_EDITOR] Nr of colors must be a integer lower than " << MAXNROFCOLORS << "and greater or equal than 0.";
            continue;
        }
        std::string save_path = palette_dir.empty() ? (crdir + "palette.txt") : palette_dir;
        std::ofstream out(save_path);
        if (!out.is_open()) {
            std::cout << "[SPRITE_EDITOR] Couldn't open file " << save_path << '\n';
            continue;
        }
        out << palette_size << ' ';
        for (int i = 0; i < palette_size; i++) {
            out << current_palette[i].r << ' ';
            out << current_palette[i].g << ' ';
            out << current_palette[i].b << ' ';
        }
        std::cout << "[SPRITE_EDITOR] Palette was successfully saved at " << save_path << '\n';
        continue;
    }

    if (tokens[0] == "loadpalette") {
        if (tokens.size() < 2) {
            std::cout << "[SPRITE_EDITOR] Not enough arguments!\n";
            std::cout << "[SPRITE_EDITOR] Arguments for loadpalette are: name\n";
            continue;
        }
        std::ifstream in(crdir + tokens[1]);
        if (!in.is_open()) {
            std::cout << "[SPRITE_EDITOR] Couldn't open file " << crdir + tokens[1] << '\n';
            continue;
        }
        int palette_size;
        in >> palette_size;
        if (palette_size < 0 || palette_size > MAXNROFCOLORS) {
            std::cout << "[SPRITE_EDITOR] Invalid nr of colors.\n";
            std::cout << "[SPRITE_EDITOR] Nr of colors must be a integer lower than " << MAXNROFCOLORS << "and greater or equal than 0.";
            continue;
        }
        for (int i = 0; i < palette_size; i++) {
            int r, g, b;
            in >> r >> g >> b;
            if (r < 0 || r > 256 || g < 0 || g > 256 || b < 0 || b > 256) {
                std::cout << "[SPRITE_EDITOR] Invalid color in palette file.\n"; 
                std::cout << "[SPRITE_EDITOR] Every color value must be an integer lower than 256 and greater or equal to 0.\n";
                continue;
            }
            current_palette[i] = {r, g, b};
        }  
        in.close();
        std::cout << "[SPRITE_EDITOR] Successfully loaded palette from " << crdir + tokens[1] << '\n';
    }


    if (tokens[0] == "applypalette") {
        if (tokens.size() < 2) {
            std::cout << "[SPRITE_EDITOR] Not enough arguments!\n";
            std::cout << "[SPRITE_EDITOR] Arguments for savepalette are: nr of colors\n";
            continue;
        }
        int palette_size = stoi(tokens[1]);
        if (palette_size < 0 || palette_size > MAXNROFCOLORS) {
            std::cout << "[SPRITE_EDITOR] Invalid nr of colors.\n";
            std::cout << "[SPRITE_EDITOR] Nr of colors must be a integer lower than " << MAXNROFCOLORS << "and greater or equal than 0.\n";
            continue;
        }
        for (int i = 0; i < palette_size; i++) {
            canvas->get_srenderer().palette[i] = current_palette[i];
            brush.get_srenderer().palette[i] = current_palette[i];
        }
        canvas->refresh();
        brush.refresh();
        std::cout << "[SPRITE_EDITOR] Successfully applied changes to palette.\n";
        continue;
    }

    if (tokens[0] == "clear") {
        std::cout << "\x1b[2J\x1b[H" << std::flush;
        continue;
    }

    if (tokens[0] == "setbg") {
        if (tokens.size() < 4) {
            std::cout << "[SPRITE_EDITOR] Not enough arguments! Arguments should be a color in RGB format.\n";
            continue;
        }
        COLOR background_color = {
            stoi(tokens[1]), 
            stoi(tokens[2]), 
            stoi(tokens[3])
        };
        bool valid_parameters = (
            valid_color(background_color.r) && 
            valid_color(background_color.g) && 
            valid_color(background_color.b)
        );
        if (valid_parameters) {
            global_background_color = { tokens[1], tokens[2], tokens[3] };
        } else {
            std::cout << "[SPRITE_EDITOR] Invalid color!\n";
            std::cout << "[SPRITE_EDITOR] Every color value must be an integer lower than 257 and greater or equal to 0.\n";
            continue;
        }
        continue;
    }

    if (tokens[0] == "esc") if (confirm_unsaved()) exit(0);

    if (tokens[0] != "exit") std::cout << "[SPRITE_EDITOR] Invalid command \"" + tokens[0] + "\"\n";

    } while (tokens[0] != "exit");

    window.remove_sprites_from_renderer([](Sprite* d){
        if (d->label == 1) return true;
        else return false;
    });
    window.add_sprite_to_renderer(canvas);
    brush.set_coords(0, 0);

    unsigned int font_h = (double) (window.screen_size.y / FONT_RATIO_HEIGHT);
    unsigned int font_w = (double) (window.screen_size.x / FONT_RATIO_LENGTH);
    window.set_font_settings(font_h, font_w);
    std::cout << "\x1b[48;2;" << global_background_color.r << ';' << global_background_color.g << ';' << global_background_color.b << 'm';
    std::cout << "\x1b[2J\x1b[H" << std::flush;
}

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

//   ______
//  | LOOP |  

int s_mode = 0;
rectangle selection;

void loop () {
    std::unordered_set<char> input = window.get_keys();
    
    //  Handle brush movement to follow mouse
    GetCursorPos(&p);
    unsigned int x = (double)p.x / ((double) window.screen_size.y / FONT_RATIO_HEIGHT) * FONT_WIDTH_MULTIPLIER;
    unsigned int y = round((double)p.y / ((double) window.screen_size.x / FONT_RATIO_LENGTH)) * FONT_HEIGHT_MULTIPLIER;
    if (canvas->frame_size.y > y && canvas->frame_size.x > x && (brush.get_coords().x != x || brush.get_coords().y != y)) brush.set_coords(x, y);
    //  If selection mode active, handle coords
    if (s_mode) {
        selection.x2 = brush.get_coords().x;
        selection.y2 = brush.get_coords().y;
    }
    //  Handle commands
    if (input.count('q')) get_command();
    if (input.count('z')) undo();
    if (input.count('y')) redo();

    if (input.count('c')) {
        if (s_mode) {
            selection_s.hide();
            brush.show();
            rectangle cpy = {
                (std::min)(selection.x1, selection.x2), 
                (std::min)(selection.y1, selection.y2), 
                (std::max)(selection.x1, selection.x2), 
                (std::max)(selection.y1, selection.y2)
            };
            //  Empty clipboard
            for (int y = 0; y < WINDOWHEIGHT; y++) 
            for (int x = 0; x < WINDOWLENGTH; x++) 
            clipboard.frames[0][y][x].character = ' ';
            //  Copy chunk
            copy_chunk_from_sprite(cpy, 0, 0, &clipboard, canvas);
            //  Delete chars from canvas if is in cut mode
            if (s_mode == 2) {
                std::vector<change> mods;
                for (unsigned y = cpy.y1; y <= cpy.y2; y++) 
                for (unsigned x = cpy.x1; x <= cpy.x2; x++) {
                    POINT_e p = {x, y};
                    change p1 = { 
                        p, 
                        canvas->frames[canvas->get_animation().current_frame][y][x],
                        canvas->get_animation().current_frame
                    };
                    canvas->frames[canvas->get_animation().current_frame][y][x].character = ' ';
                    mods.push_back(p1);
                }
                canvas->refresh();
                push_undo(mods);
            }
            //  Turn off selection mode
            s_mode = 0;
        } else {
            brush.hide();
            selection_s.show();
            s_mode = 1;
            selection = {
                brush.get_coords().x, 
                brush.get_coords().y, 
                brush.get_coords().x, 
                brush.get_coords().y
            };
        }
    }

    if (input.count('x')) {
        brush.hide();
        s_mode = 2;
        selection_s.show();
        selection = {
            brush.get_coords().x, 
            brush.get_coords().y, 
            brush.get_coords().x, 
            brush.get_coords().y
        };
    }

    if (input.count('p')) {
        rectangle to_copy = get_drawing(&clipboard);
        copy_chunk_from_sprite(to_copy, brush.get_coords().x, brush.get_coords().y, canvas, &clipboard);
    }

    //  On click
    if (left_mouse_down() && brush.get_view().visible && !s_mode) {
        //  Clear stack
        redo_stack.clear();
        unsigned int c_frame = canvas->get_animation().current_frame;
        //  Register changes
        POINT_e p = {
            brush.get_coords().x, 
            brush.get_coords().y
        };
        change current_change = { 
            p, 
            canvas->frames[c_frame][brush.get_coords().y][brush.get_coords().x], 
            c_frame 
        };
        change tmp;
        if (!changes.empty()) tmp = changes[changes.size() - 1];

        //  Write and push changes
        if (changes.empty() || tmp.cell.character != current_change.cell.character || tmp.cell.color_id != current_change.cell.color_id ||
        tmp.coords.x != current_change.coords.x || tmp.coords.y != current_change.coords.y) {
            changes.push_back(current_change);
            canvas->frames[c_frame][brush.get_coords().y][brush.get_coords().x] = brush.frames[0][0][0];
            saved = false;
            canvas->refresh();
        }
    } 
    else if (!s_mode && brush.get_view().visible) push_undo(changes);

    //  Mark rectangle selected
    if (s_mode) {
        rectangle tmp = {
            (std::min)(selection.x1, selection.x2), 
            (std::min)(selection.y1, selection.y2), 
            (std::max)(selection.x1, selection.x2), 
            (std::max)(selection.y1, selection.y2)
        };
        for (int y = 0; y < selection_s.frame_size.y; y++) {
            for (int x = 0; x < selection_s.frame_size.x; x++) {
                selection_s.frames[0][y][x].character = ' ';
            }
        }
        for (unsigned int y = tmp.y1; y <= tmp.y2; y++) {
            for (unsigned int x = tmp.x1; x <= tmp.x2; x++) {
                selection_s.frames[0][y][x].character = '.';
            }
        }
        selection_s.refresh();
    }

    window.empty_keys();

    if (canvas->get_animation().is_animation_active) {
        canvas->next_game_tick();
        canvas->refresh();
    }

    Sleep(1);
}

//   ________
//  | Setup |
//

int main() { 
    //  Mouse input setup
    setup_input();

    //  Setup
    brush.set_stage(1);
    brush.frame_size.y = 1;
    brush.frame_size.x = 1;
    brush.set_nr_of_frames(1);
    brush.sprite_init();
    brush.transparent_space(false);
    for (int i = 0; i < brush.get_srenderer().nr_of_colors; i++) 
        brush.get_srenderer().palette[i] = {255, 0, 0};
    brush.frames[0][0][0].character = '#';
    brush.frames[0][0][0].color_id = 0;
    brush.refresh();

    //  Clipboard
    clipboard.set_nr_of_frames(1);
    clipboard.frame_size.y = WINDOWHEIGHT;
    clipboard.frame_size.x = WINDOWLENGTH;
    clipboard.hide();
    clipboard.sprite_init();
    for (int i = 0; i < clipboard.get_srenderer().nr_of_colors; i++) 
        clipboard.get_srenderer().palette[i] = {255, 0, 0};

    //  Selection
    selection_s.set_nr_of_frames(1);
    selection_s.set_stage(3);
    selection_s.frame_size.y = WINDOWHEIGHT;
    selection_s.frame_size.x = WINDOWLENGTH;
    selection_s.hide();
    selection_s.sprite_init();
    selection_s.get_srenderer().palette[0] = {100, 100, 100};

    //  Canvas
    canvas->transparent_space(false);
    canvas->set_stage(0);
    canvas->frame_size = {WINDOWLENGTH, WINDOWHEIGHT};
    canvas->set_nr_of_frames(1);
    canvas->sprite_init();

    //  Current palette initialisation
    COLOR default_color = { 255, 0, 0 };
    for (int i = 0; i < MAXNROFCOLORS; i++) 
        current_palette[i] = default_color;

    window.add_sprite_to_renderer(&brush);
    window.add_sprite_to_renderer(&selection_s);
    get_command();
    window.game_loop(loop);

    std::cin.get();
    return 0;
}