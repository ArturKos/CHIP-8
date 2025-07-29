#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>
#include <thread>
#include <chrono>
#include <stdint.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "../CHIP-8/src/chip-8.hpp"

#define BOARD_WIDTH 1024
#define BOARD_HEIGHT 768
#define BACKGROUND_IMAGE_PATH "../../images/background.jpeg"
#define SOUND_FILE_PATH "../../sounds/beep.wav"

// Function mapping CHIP-8 key numbers to Allegro 5 key codes
int map_key(int i)
{
    switch (i)
    {
    case 0x0:
        return ALLEGRO_KEY_1; // CHIP-8 0x0 -> ALLEGRO_KEY_1
    case 0x1:
        return ALLEGRO_KEY_2; // CHIP-8 0x1 -> ALLEGRO_KEY_2
    case 0x2:
        return ALLEGRO_KEY_3; // CHIP-8 0x2 -> ALLEGRO_KEY_3
    case 0x3:
        return ALLEGRO_KEY_4; // CHIP-8 0x3 -> ALLEGRO_KEY_4
    case 0x4:
        return ALLEGRO_KEY_Q; // CHIP-8 0x4 -> ALLEGRO_KEY_Q
    case 0x5:
        return ALLEGRO_KEY_W; // CHIP-8 0x5 -> ALLEGRO_KEY_W
    case 0x6:
        return ALLEGRO_KEY_E; // CHIP-8 0x6 -> ALLEGRO_KEY_E
    case 0x7:
        return ALLEGRO_KEY_R; // CHIP-8 0x7 -> ALLEGRO_KEY_R
    case 0x8:
        return ALLEGRO_KEY_A; // CHIP-8 0x8 -> ALLEGRO_KEY_A
    case 0x9:
        return ALLEGRO_KEY_S; // CHIP-8 0x9 -> ALLEGRO_KEY_S
    case 0xA:
        return ALLEGRO_KEY_D; // CHIP-8 0xA -> ALLEGRO_KEY_D
    case 0xB:
        return ALLEGRO_KEY_F; // CHIP-8 0xB -> ALLEGRO_KEY_F
    case 0xC:
        return ALLEGRO_KEY_Z; // CHIP-8 0xC -> ALLEGRO_KEY_Z
    case 0xD:
        return ALLEGRO_KEY_X; // CHIP-8 0xD -> ALLEGRO_KEY_X
    case 0xE:
        return ALLEGRO_KEY_C; // CHIP-8 0xE -> ALLEGRO_KEY_C
    case 0xF:
        return ALLEGRO_KEY_V; // CHIP-8 0xF -> ALLEGRO_KEY_V
    default:
        return -1; // Returns -1 for unknown keys
    }
}

int main(int argc, char *argv[])
{
    // Load ROM
    std::string rom_path = "../roms/Pong.ch8";
    if (argc > 1)
    {
        rom_path = argv[1];
    }
    else
    {
        std::cout << "Usage: " << argv[0] << " <rom_path>" << std::endl;
        return 1;
    }
    
    Chip8 chip8;
    if (!chip8.load_rom(rom_path))
    {
        return 1;
    }

    al_init();
    al_install_keyboard();
    al_init_font_addon();
    al_init_image_addon();
    al_init_primitives_addon();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(1);
    ALLEGRO_KEYBOARD_STATE allegro_keyboard;

    ALLEGRO_DISPLAY *allegro_window = al_create_display(BOARD_WIDTH, BOARD_HEIGHT);

    al_set_window_title(allegro_window, "CHIP-8 emulator by Artur Kos");

    ALLEGRO_BITMAP *background = NULL;
    ALLEGRO_FONT *font8 = al_create_builtin_font();

    background = al_load_bitmap(BACKGROUND_IMAGE_PATH);
    if (!background)
    {
        return -1;
    }

    al_draw_scaled_bitmap(background, 0, 0, al_get_bitmap_width(background),
                          al_get_bitmap_height(background), 0, 0, BOARD_WIDTH,
                          BOARD_HEIGHT, 0);
    al_draw_text(font8, al_map_rgb(255, 255, 255), BOARD_WIDTH / 2, BOARD_HEIGHT / 2,
                 ALLEGRO_ALIGN_CENTRE, "Press space to start");

    // Display info about exiting and key mapping
    al_draw_text(font8, al_map_rgb(200, 200, 200), BOARD_WIDTH / 2, BOARD_HEIGHT / 2 + 40,
                 ALLEGRO_ALIGN_CENTRE, "Press ESC to exit");
    al_draw_text(font8, al_map_rgb(200, 200, 200), BOARD_WIDTH / 2, BOARD_HEIGHT / 2 + 80,
                 ALLEGRO_ALIGN_CENTRE, "Key mapping: 1 2 3 4 | Q W E R | A S D F | Z X C V");
    al_draw_text(font8, al_map_rgb(200, 200, 200), BOARD_WIDTH / 2, BOARD_HEIGHT / 2 + 110,
                 ALLEGRO_ALIGN_CENTRE, "CHIP-8 keys: 1 2 3 C | 4 5 6 D | 7 8 9 E | A 0 B F");

    al_flip_display();

    al_clear_to_color(al_map_rgb(200, 0, 0));
    al_set_target_bitmap(al_get_backbuffer(allegro_window));

    int pixel_width = BOARD_WIDTH / 64;
    int pixel_height = BOARD_HEIGHT / 32;

    // Initial screen with information about starting the game
    bool started = false;
    while (!started)
    {
        al_get_keyboard_state(&allegro_keyboard);
        if (al_key_down(&allegro_keyboard, ALLEGRO_KEY_SPACE))
        {
            started = true;
        }
    }
    // Main loop
    while (!chip8.pc_instruction_overflow() && !al_key_down(&allegro_keyboard, ALLEGRO_KEY_ESCAPE))
    {
        chip8.emulate_cycle();

        std::this_thread::sleep_for(std::chrono::milliseconds(2)); // (optional, speed control)

        // Rendering by Allegro5 library
        if (chip8.is_screen_ready_to_redraw())
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            for (int y = 0; y < CHIP8_SCREEN_HEIGHT; ++y)
            {
                for (int x = 0; x < CHIP8_SCREEN_WIDTH; ++x)
                {
                    if (chip8.get_screen_pixel(x, y) == 1)
                    {
                        al_draw_filled_rectangle(
                            x * pixel_width, y * pixel_height,
                            x * pixel_width + pixel_width, y * pixel_height + pixel_height,
                            al_map_rgba(255, 255, 255, 255));
                    }
                }
            }

            al_flip_display();
        }

        al_get_keyboard_state(&allegro_keyboard);
        for (int i = 0; i < CHIP8_KEYBOARD_SIZE; i++)
        {
            if (al_key_down(&allegro_keyboard, map_key(i)))
            {
                chip8.set_key(i, 1); // Key pressed
            }
            else
            {
                chip8.set_key(i, 0); // Key not pressed
            }
        }
        if (chip8.play_sound())
        {
            static ALLEGRO_SAMPLE *sample = nullptr;
            if (!sample)
            {
                sample = al_load_sample(SOUND_FILE_PATH);
                if (!sample)
                {
                    std::cerr << "Cannot load " << SOUND_FILE_PATH << "!" << std::endl;
                }
            }
            if (sample)
            {
                al_play_sample(sample, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
        }
        chip8.render_screen(false); // This function modifies draw_flag, so it should be called after rendering with the Allegro
    }
    al_destroy_display(allegro_window);
    al_destroy_font(font8);
    return 0;
}