#pragma once
#include <Windows.h>

class Draw
{
public:
    void DrawFilledRect(int x, int y, int w, int h, HBRUSH BrushColor, HDC hClient);
    void DrawBorderBox(int x, int y, int w, int h, int thickness, HBRUSH Color, HDC hClient);
    void DrawLine(int targetX, int targetY, HDC hClient);
    void DrawString(int x, int y, COLORREF color, const char* text, HDC hClient, HFONT Font);
};

