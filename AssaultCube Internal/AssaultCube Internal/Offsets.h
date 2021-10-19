#pragma once
#include <Windows.h>

class Offsets
{
public:
    DWORD entityList = 0x0050F4F4;
    DWORD viewMatrix = 0x501AE8;

    DWORD playerHealth = 0xF8;
    DWORD playerName = 0x225;
    DWORD playerTeam = 0x32C;
    DWORD playerVelocity = 0x10;
    DWORD playerSpeed = 0x80;

    DWORD localX = 0x4;
    DWORD localY = 0x8;
    DWORD localZ = 0xC;

    DWORD yaw = 0x40;
    DWORD pitch = 0x44;
};