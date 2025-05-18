#include "../includes/libraries.h"
#include "../includes/defines.h"
#include "../includes/classes.h"

namespace Config
{
int FPS;
};

void log(int err_code, int sprite_label)
{
    ofstream lg("log.txt");
    if (!lg.is_open())
    {
        cout << 999;
        cin.get();
        exit(err_code);
    }
    lg << "[CONSOLE] Program exited with error code: " << err_code << '\n';
    lg << "[CONSOLE] Sprite id: " << sprite_label << '\n';
    lg.flush();
    exit(err_code);
}

void Window::config()
{
    ifstream conf("settings.config");
    if (!conf.is_open()) log(101, 0);

    int dummy;
    if (!(conf >> dummy))
    {
        //Daca fisierul config este gol
        cout << "[CONFIG] The settings file is empty. Using the standard options.\n";
        conf.close();
        ofstream conf_out("settings.config");
        if (!conf_out.is_open()) log(104);

        //De continuat daca adaugam eventual mai multe setari pentru engine-ul propriu-zis
        conf_out << 1 << ' ' << 60;
        conf_out.close();
        return;
    }

    if (conf >> Config::FPS)
    {
        //Daca fisierul nu este gol
        if (Config::FPS < 1 || Config::FPS > 144) log(103);
    }
    else log(103, 0);
    conf.close();
}

void Window::empty_buffer()
{
    for (int i = 0; i < WINDOWHEIGHT; i++)
        for (int j = 0; j < WINDOWLENGTH; j++)
            buffer[i][j] = ' ';
}

void Window::DEBUG_fill()
{
    for (int i = 0; i < WINDOWHEIGHT; i++)
    {
        for (int j = 0; j < WINDOWLENGTH; j++) cout << '*';
        cout << '\n';
    }
}

void Window::init()
{
    ios_base::sync_with_stdio(false);
    empty_buffer();
    unsigned int font_h = round((double) screen_height / FONT_RATIO_HEIGHT);
    unsigned int font_w = round((double) screen_width / FONT_RATIO_LENGTH);
    set_font_settings(font_h, font_w);
}

void Window::set_font_settings(unsigned int f_height, unsigned int f_width)
{
    //Vezi documentatia windows pentru consola
    if (f_height < 1 || f_width < 1) log(200);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) log(201);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    if (!GetCurrentConsoleFontEx(hConsole, TRUE, &cfi)) log(201);
    cfi.dwFontSize.X = f_width;
    cfi.dwFontSize.Y = f_height;
    wcscpy(cfi.FaceName, L"Cascadia Mono");
    if (!SetCurrentConsoleFontEx(hConsole, TRUE, &cfi)) log(201);
    font_size_height = f_height;
    font_size_width = f_width;
}
