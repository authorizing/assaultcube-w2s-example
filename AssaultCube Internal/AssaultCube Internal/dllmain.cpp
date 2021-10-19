#include <Windows.h>

#include <iostream>

#include "offsets.h"
#include "Draw.h"

HWND AssaultCubeClient;
HDC hClient;

// Brushes

HBRUSH Brush;
HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));

COLORREF TextCOLOR;
COLORREF TextCOLORRED;

HFONT Font;

float Matrix[16];

struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };

class Entity
{
public:
    char pad_0000[4]; //0x0000
    Vec3 headPosition; //0x0004
    char pad_0010[36]; //0x0010
    Vec3 PlayerPosition; //0x0034
    Vec3 PlayerAngles; //0x0040
    char pad_004C[172]; //0x004C
    uint32_t playerHealth; //0x00F8
    char pad_00FC[296];  //0x00FC
    int8_t bAttack; //0x0224
    char pad_0255[263]; //0x0225
    int32_t team; //0x032C
    char pad_0330[68]; //0x0330
    class Weapon* currentWeapon; //0x0374
    char pad_0378[1248]; //0x0378
}; //Size: 0x0858
static_assert(sizeof(Entity) == 0x858);

class Weapon
{
public:
    char pad_0000[4]; //0x0000
    int32_t weaponid;
    class Entity* owner;
    char pad_000C[4];
    class ammoClip* ammoReserve;
    class ammoClip* ammoPointer; //0x0014
    char pad_0018[44];
}; //Size: 0x0044
static_assert(sizeof(Weapon) == 0x44);

class ammoClip
{
public:
    int32_t ammo; //0x0000
}; //Size: 0x0004
static_assert(sizeof(ammoClip) == 0x4);

// World2Screen OpenGL Function (https://guidedhacking.com/)
bool WorldToScreen(Vec3 pos, Vec2& screen, float matrix[16], int windowWidth, int windowHeight) // 3D to 2D
{
    //Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
    Vec4 clipCoords;
    clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
    clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
    clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
    clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

    if (clipCoords.w < 0.1f)
        return false;

    //perspective division, dividing by clip.W = Normalized Device Coordinates
    Vec3 NDC;
    NDC.x = clipCoords.x / clipCoords.w;
    NDC.y = clipCoords.y / clipCoords.w;
    NDC.z = clipCoords.z / clipCoords.w;

    //Transform to window coordinates
    screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
    screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
    return true;
}

bool IsAlive(int Health) {
    if (Health > 0)
        return true;
    else
        return false;
}

void ReadMemory(const void* address, void* buffer, size_t size)
{
    DWORD back = NULL;

    if (VirtualProtect((LPVOID)address, size, PAGE_READWRITE, &back))
    {
        memcpy(buffer, address, size);

        VirtualProtect((LPVOID)address, size, back, &back);
    }
}

template <typename T>
static void set_value(uintptr_t address, const T& value)
{
    *(T*)address = value;
}

typedef Entity* (__cdecl* tGetCrosshairEntity)();
tGetCrosshairEntity GetCrosshairEntity = nullptr;

Offsets* offsets = new Offsets();
Draw* draw = new Draw();
Entity* entity = new Entity();

int MainThread()
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    SetConsoleTitle(L"Output");

    TextCOLOR = RGB(255, 255, 255);
    AssaultCubeClient = FindWindow(0, (L"AssaultCube")); //Gets Window

    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");

    GetCrosshairEntity = (tGetCrosshairEntity)(moduleBase + 0x607C0);

    Entity* localPlayer = { nullptr };

    DWORD entityList = *(DWORD*)(offsets->entityList + 0x4);

    std::cout << "Module Base Address (ac_client.exe) = 0x" << std::hex << moduleBase << std::endl;
    std::cout << "Local Player Address = 0x" << std::hex << localPlayer << std::endl;
    std::cout << "Entity List Address [Start] = 0x" << std::hex << entityList << std::endl;

    RECT rect;

    int windowWidth = 0, windowHeight = 0;

    bool bTriggerbot = false;

    while (true)
    {
        if (GetAsyncKeyState(VK_XBUTTON1) && 1) {
            bTriggerbot = true;
        }

        localPlayer = *(Entity**)(moduleBase + 0x10F4F4);

        if (localPlayer) {
            if (GetWindowRect(AssaultCubeClient, &rect)) {
                windowWidth = rect.right - rect.left;
                windowHeight = rect.bottom - rect.top;
            }

            memcpy(&Matrix, (PBYTE*)(offsets->viewMatrix), sizeof(Matrix));
            hClient = GetDC(AssaultCubeClient);

            //Base of player
            Vec2 vScreen;

            //Head of player
            Vec2 vHead;

            //Sets the ammount of Players
            DWORD amountOfPlayers = *(DWORD*)(0x50F500);

            if (bTriggerbot) {
                Entity* crosshairEntity = GetCrosshairEntity();

                if (crosshairEntity) {
                    if (localPlayer->team != crosshairEntity->team) {
                        localPlayer->bAttack = 1;
                    }     
                } 
                else {
                    localPlayer->bAttack = 0;
                }  
            }

            for (short int i = 0; i < amountOfPlayers; i++)
            {
                DWORD entity = *(DWORD*)(entityList + 0x4 * i);

                if (entity != NULL)
                {
                    //Entity Position
                    float enemyX = *(float*)(entity + 0x34);
                    float enemyY = *(float*)(entity + 0x38);
                    float enemyZ = *(float*)(entity + 0x3C);

                    Vec3 enemyPos = { enemyX, enemyY, enemyZ };

                    //Enemys Head Pos
                    float enemyXHead = *(float*)(entity + 0x4);
                    float enemyYHead = *(float*)(entity + 0x8);
                    float enemyZHead = *(float*)(entity + 0xC);

                    Vec3 enemyHeadPos = { enemyXHead, enemyYHead, enemyZHead };

                    // sets each entity values
                    DWORD health = *(DWORD*)(entity + offsets->playerHealth);
                    DWORD playerTeam = *(DWORD*)(entity + offsets->playerTeam);
                    DWORD playerName = *(DWORD*)(entity + offsets->playerName);

                    //converts 3d to 2d

                    if (WorldToScreen(enemyPos, vScreen, Matrix, windowWidth, windowHeight))
                    {
                        if (WorldToScreen(enemyHeadPos, vHead, Matrix, windowWidth, windowHeight))
                        {
                            float head = vHead.y - vScreen.y;   // Creates the head height
                            float width = head / 2;             // Creates width
                            float center = width / -2;          // Creates center
                            float extra = head / -6;            // Creates extra area above head

                            wchar_t Text[32];
                            ReadMemory((const void*)(entity + offsets->playerName), Text, sizeof(Text));

                            if (IsAlive(int(health))) {
                                if (int(playerTeam) != localPlayer->team) { // Check if is an enemy
                                    draw->DrawBorderBox(vScreen.x + center, vScreen.y, width, head - extra, 1, redBrush, hClient);
                                    draw->DrawString(vScreen.x, vScreen.y, TextCOLOR, (char*)Text, hClient, Font);
                                }
                                draw->DrawFilledRect((vScreen.x + 3) + center, vScreen.y, 2, (head - extra) / 100 * (int)health, greenBrush, hClient);
                            }

                            //Turns on snaplines
                            if (GetKeyState(VK_F2) & 1)
                                draw->DrawLine(vScreen.x, vScreen.y, hClient);
                        }
                    }
                }
            }
        }

        if (GetAsyncKeyState(VK_END)) {
            break;
        }
        
        Sleep(1);
        DeleteObject(hClient);
    }

    fclose(f);
    FreeConsole();
    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MainThread, NULL, NULL, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

