#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "../CHIP-8/src/chip-8.hpp"

#define BOARD_WIDTH 1024
#define BOARD_HEIGHT 768
#define SOUND_FILE_PATH "../../sounds/beep.wav"

static Chip8 chip8;
static bool started = false;
static bool running = true;
static int pixel_width = BOARD_WIDTH / CHIP8_SCREEN_WIDTH;
static int pixel_height = BOARD_HEIGHT / CHIP8_SCREEN_HEIGHT;

// Function mapping CHIP-8 key numbers to Windows virtual key codes
int map_key(int i)
{
    switch (i)
    {
    case 0x0:
        return '1'; // CHIP-8 0x0 -> 1
    case 0x1:
        return '2'; // CHIP-8 0x1 -> 2
    case 0x2:
        return '3'; // CHIP-8 0x2 -> 3
    case 0x3:
        return '4'; // CHIP-8 0x3 -> 4
    case 0x4:
        return 'Q'; // CHIP-8 0x4 -> Q
    case 0x5:
        return 'W'; // CHIP-8 0x5 -> W
    case 0x6:
        return 'E'; // CHIP-8 0x6 -> E
    case 0x7:
        return 'R'; // CHIP-8 0x7 -> R
    case 0x8:
        return 'A'; // CHIP-8 0x8 -> A
    case 0x9:
        return 'S'; // CHIP-8 0x9 -> S
    case 0xA:
        return 'D'; // CHIP-8 0xA -> D
    case 0xB:
        return 'F'; // CHIP-8 0xB -> F
    case 0xC:
        return 'Z'; // CHIP-8 0xC -> Z
    case 0xD:
        return 'X'; // CHIP-8 0xD -> X
    case 0xE:
        return 'C'; // CHIP-8 0xE -> C
    case 0xF:
        return 'V'; // CHIP-8 0xF -> V
    default:
        return -1;
    }
}

void draw_start_screen(HDC hdc, RECT &clientRect)
{
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &clientRect, blackBrush);
    DeleteObject(blackBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));

    HFONT font = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);

    const char *line1 = "Press SPACE to start";
    const char *line2 = "Press ESC to exit";
    const char *line3 = "Key mapping: 1 2 3 4 | Q W E R | A S D F | Z X C V";
    const char *line4 = "CHIP-8 keys: 1 2 3 C | 4 5 6 D | 7 8 9 E | A 0 B F";

    int centerX = (clientRect.right - clientRect.left) / 2;
    int centerY = (clientRect.bottom - clientRect.top) / 2;

    RECT textRect;

    textRect = {clientRect.left, centerY - 10, clientRect.right, centerY + 10};
    DrawText(hdc, line1, -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    SetTextColor(hdc, RGB(200, 200, 200));

    textRect = {clientRect.left, centerY + 30, clientRect.right, centerY + 50};
    DrawText(hdc, line2, -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    textRect = {clientRect.left, centerY + 70, clientRect.right, centerY + 90};
    DrawText(hdc, line3, -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    textRect = {clientRect.left, centerY + 100, clientRect.right, centerY + 120};
    DrawText(hdc, line4, -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

void draw_emulator_screen(HDC hdc, RECT &clientRect)
{
    // Create off-screen buffer to avoid flickering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Clear to black
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(memDC, &clientRect, blackBrush);
    DeleteObject(blackBrush);

    // Draw CHIP-8 pixels
    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
    for (int y = 0; y < CHIP8_SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < CHIP8_SCREEN_WIDTH; ++x)
        {
            if (chip8.get_screen_pixel(x, y) == 1)
            {
                RECT pixelRect = {
                    x * pixel_width,
                    y * pixel_height,
                    x * pixel_width + pixel_width,
                    y * pixel_height + pixel_height};
                FillRect(memDC, &pixelRect, whiteBrush);
            }
        }
    }
    DeleteObject(whiteBrush);

    // Copy buffer to screen
    BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        running = false;
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            running = false;
            PostQuitMessage(0);
            return 0;
        }
        if (!started && wParam == VK_SPACE)
        {
            started = true;
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        if (!started)
        {
            draw_start_screen(hdc, clientRect);
        }
        else
        {
            draw_emulator_screen(hdc, clientRect);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Parse command line for ROM path
    std::string rom_path;
    if (lpCmdLine && lpCmdLine[0] != '\0')
    {
        rom_path = lpCmdLine;
        // Remove surrounding quotes if present
        if (rom_path.front() == '"' && rom_path.back() == '"')
        {
            rom_path = rom_path.substr(1, rom_path.size() - 2);
        }
    }
    else
    {
        MessageBox(NULL, "Usage: chip8_emulator.exe <rom_path>", "CHIP-8 Emulator", MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    if (!chip8.load_rom(rom_path))
    {
        MessageBox(NULL, "Failed to load ROM file.", "CHIP-8 Emulator", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Register window class
    const char *CLASS_NAME = "CHIP8WindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClass(&wc);

    // Calculate window size to get desired client area
    RECT windowRect = {0, 0, BOARD_WIDTH, BOARD_HEIGHT};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "CHIP-8 emulator by Artur Kos",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
    {
        MessageBox(NULL, "Failed to create window.", "CHIP-8 Emulator", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Main loop
    MSG msg = {};
    while (running)
    {
        // Process Windows messages (non-blocking)
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running)
            break;

        if (!started)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        if (chip8.pc_instruction_overflow())
        {
            running = false;
            break;
        }

        chip8.emulate_cycle();

        std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Speed control

        // Rendering
        if (chip8.is_screen_ready_to_redraw())
        {
            HDC hdc = GetDC(hwnd);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            draw_emulator_screen(hdc, clientRect);
            ReleaseDC(hwnd, hdc);
        }

        // Keyboard input via GetAsyncKeyState
        for (int i = 0; i < CHIP8_KEYBOARD_SIZE; i++)
        {
            int vk = map_key(i);
            if (vk != -1 && (GetAsyncKeyState(vk) & 0x8000))
            {
                chip8.set_key(i, 1); // Key pressed
            }
            else
            {
                chip8.set_key(i, 0); // Key not pressed
            }
        }

        // Sound
        if (chip8.play_sound())
        {
            PlaySound(TEXT(SOUND_FILE_PATH), NULL, SND_FILENAME | SND_ASYNC | SND_NOSTOP);
        }

        chip8.render_screen(false);
    }

    DestroyWindow(hwnd);
    return 0;
}
