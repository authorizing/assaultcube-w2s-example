#include "Draw.h"

void Draw::DrawFilledRect(int x, int y, int w, int h, HBRUSH BrushColor, HDC hClient) {
    RECT rect = { x, y, x + w, y + h };
    FillRect(hClient, &rect, BrushColor);
}

void Draw::DrawBorderBox(int x, int y, int w, int h, int thickness, HBRUSH Color, HDC hClient) {
    DrawFilledRect(x, y, w, thickness, Color, hClient);
    DrawFilledRect(x, y, thickness, h, Color, hClient);
    DrawFilledRect((x + w), y, thickness, h, Color, hClient);
    DrawFilledRect(x, y + h, w + thickness, thickness, Color, hClient);
}

void Draw::DrawLine(int targetX, int targetY, HDC hClient) {
    MoveToEx(hClient, 960, 1080, NULL);
    LineTo(hClient, targetX, targetY);
}

void Draw::DrawString(int x, int y, COLORREF color, const char* text, HDC hClient, HFONT Font) {
    SetTextAlign(hClient, TA_CENTER | TA_NOUPDATECP);
    SetBkColor(hClient, RGB(0, 0, 0));
    SetBkMode(hClient, TRANSPARENT);
    SetTextColor(hClient, color);
    SelectObject(hClient, Font);
    TextOutA(hClient, x, y, text, strlen(text));
    DeleteObject(Font);
}