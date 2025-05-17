#pragma once

#ifndef _CONSOLE_FONT_INFOEX
#define _CONSOLE_FONT_INFOEX
typedef struct _CONSOLE_FONT_INFOEX {
    ULONG cbSize;
    DWORD nFont;
    COORD dwFontSize;
    UINT  FontFamily;
    UINT  FontWeight;
    WCHAR FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;
#endif

extern "C" BOOL WINAPI GetCurrentConsoleFontEx(
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx
);

extern "C" BOOL WINAPI SetCurrentConsoleFontEx(
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx
);

