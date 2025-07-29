#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm> // For std::fill
#include <iterator>  // For std::begin, std::end

/// \def CHIP8_MEMORY_SIZE
/// \brief Total size of CHIP-8 memory in bytes.
#define CHIP8_MEMORY_SIZE 4096

/// \def CHIP8_FONT_BUFFER_SIZE
/// \brief Size of the font sprite buffer.
#define CHIP8_FONT_BUFFER_SIZE 80

/// \def CHIP8_FONT_START_ADDRESS_IN_MEMORY
/// \brief Start address in memory for the font sprites.
#define CHIP8_FONT_START_ADDRESS_IN_MEMORY 0x50

/// \def CHIP8_REGISTERS_COUNT
/// \brief Number of general purpose registers in CHIP-8.
#define CHIP8_REGISTERS_COUNT 16

/// \def CHIP8_STACK_SIZE
/// \brief Size of the call stack.
#define CHIP8_STACK_SIZE 16

/// \def CHIP8_SCREEN_WIDTH
/// \brief Width of the CHIP-8 display in pixels.
#define CHIP8_SCREEN_WIDTH 64

/// \def CHIP8_SCREEN_HEIGHT
/// \brief Height of the CHIP-8 display in pixels.
#define CHIP8_SCREEN_HEIGHT 32

/// \def CHIP8_VIDEO_BUFFER_SIZE
/// \brief Total number of pixels in the video buffer.
#define CHIP8_VIDEO_BUFFER_SIZE (CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT)

/// \def CHIP8_START_PROGRAM_ADDRESS
/// \brief Default start address for CHIP-8 programs.
#define CHIP8_START_PROGRAM_ADDRESS 0x200

/// \def CHIP8_KEYBOARD_SIZE
/// \brief Number of keys on the CHIP-8 keyboard.
#define CHIP8_KEYBOARD_SIZE 16

/**
 * @class Chip8
 * @brief Main class implementing the CHIP-8 emulator.
 */
class Chip8
{
private:
    uint16_t loaded_rom_size;      ///< Size of the loaded ROM.
    bool draw_flag;                ///< Indicates if the screen needs to be redrawn.

    uint8_t memory[CHIP8_MEMORY_SIZE] = {};           ///< Main memory.
    uint8_t V[CHIP8_REGISTERS_COUNT] = {};            ///< General purpose registers V0-VF.
    uint16_t I = 0;                                   ///< Index register.
    uint16_t pc = CHIP8_START_PROGRAM_ADDRESS;         ///< Program counter.
    uint16_t stack[CHIP8_STACK_SIZE] = {};             ///< Call stack.
    uint8_t sp = 0;                                   ///< Stack pointer.
    uint8_t delayTimer = 0;                           ///< Delay timer.
    uint8_t soundTimer = 0;                           ///< Sound timer.
    uint8_t gfx[CHIP8_VIDEO_BUFFER_SIZE] = {};        ///< Video buffer.
    uint8_t key[CHIP8_KEYBOARD_SIZE]{};               ///< Keyboard state.

    /// Font sprite data (hexadecimal digits 0-F).
    const uint8_t sprite_font[CHIP8_FONT_BUFFER_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    /**
     * @brief Copies the font sprite data into CHIP-8 memory.
     */
    void copy_sprite_font_into_memory();

    /**
     * @brief Clears the video buffer.
     */
    void clear_video_buffer();

    /**
     * @brief Draws a sprite at the given coordinates.
     * @param x_coord X coordinate.
     * @param y_coord Y coordinate.
     * @param height Height of the sprite.
     */
    void draw_sprite(uint8_t x_coord, uint8_t y_coord, uint8_t height);

public:

    /**
     * @brief Constructs a new Chip8 emulator instance.
     */
    Chip8();

    /**
     * @brief Checks if the program counter has overflowed.
     * @return true if overflow occurred, false otherwise.
     */
    bool pc_instruction_overflow();

    /**
     * @brief Executes one emulation cycle (fetch, decode, execute).
     */
    void emulate_cycle();

    /**
     * @brief Loads a ROM file into memory.
     * @param filename Path to the ROM file.
     * @return true if loading was successful, false otherwise.
     */
    bool load_rom(const std::string &filename);

    /**
     * @brief Renders the current state of the screen.
     * 
     * If debug_draw_to_console is true, the screen will also be printed as ASCII art to the console.
     * This is useful for debugging without a graphical frontend.
     * 
     * @param debug_draw_to_console If true, draw the screen to the console as ASCII art.
     */
    void render_screen(bool debug_draw_to_console);

    /**
     * @brief Checks if the screen is ready to be redrawn.
     * @return true if redraw is needed, false otherwise.
     */
    bool is_screen_ready_to_redraw() const { return draw_flag; }

    /**
     * @brief Gets the value of a screen pixel.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @return 1 if the pixel is set, 0 otherwise.
     */
    uint8_t get_screen_pixel(int x, int y);

    /**
     * @brief Sets the state of a key.
     * @param key_index Index of the key.
     * @param value 1 if pressed, 0 if released.
     */
    void set_key(uint8_t key_index, uint8_t value);

    /**
     * @brief Checks if the sound should be played.
     * @return true if soundTimer > 0, false otherwise.
     */
    bool play_sound() const { return soundTimer > 0; }
};

#endif //