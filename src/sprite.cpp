#include "../includes/classes.h"
#include "../includes/libraries.h"
#include "../includes/defines.h"

void Sprite::init_by_file(const char file_name[])
{
    ifstream in(file_name);
    if (!in.is_open()) log(401, label);
    if (!(in >> frame_height >> frame_width >> nr_of_frames)) log(402, label);
    if (frame_height < 1 || frame_width < 1 || nr_of_frames < 1) log(403, label);

    string input;
    //Scapam de primul new line
    getline(in, input);
    sprite_frames = new char**[nr_of_frames];
    for (unsigned int f = 0; f < nr_of_frames; f++)
    {
        sprite_frames[f] = new char*[frame_height];
        //Scapam de new line
        getline(in, input);
        for (unsigned int h = 0; h < frame_height; h++)
        {
            sprite_frames[f][h] = new char[frame_width + 1];
            getline(in, input);
            //Daca am ajuns la sfarsitul liniei inlocuim caracterele cu spatii libere
            for (unsigned int w = 0; w < frame_width; w++) sprite_frames[f][h][w] = (w < input.length()) ? input[w] : ' ';
            sprite_frames[f][h][frame_width] = '\0';
        }
    }
}

void Sprite::DEBUG_sprite()
{
    //Printeaza fisierul citit
    cout << frame_height << ' ' << frame_width << ' ' << nr_of_frames << "\n\n";
    for (unsigned int f = 0; f < nr_of_frames; f++)
    {
        for (unsigned int h = 0; h < frame_height; h++)
            for (unsigned int w = 0; w < frame_width; w++) cout << sprite_frames[f][h][w];
        cout << '\n';
        if (f < nr_of_frames - 1) cout << '\n';
    }
}

Sprite::~Sprite()
{
    //Eliberam memoria folosita (De modificat daca mai folosim memorie alocata dinamic)
    for (unsigned int f = 0; f < nr_of_frames; f++)
    {
        for (unsigned int i = 0; i < frame_height; i++) delete[] sprite_frames[f][i];
        delete[] sprite_frames[f];
    }
    delete[] sprite_frames;
}
