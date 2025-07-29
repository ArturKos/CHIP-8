#include "chip-8.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm> // For std::fill
#include <iterator>  // For std::begin, std::end
#include <cstdio>    // For printf (if you use it)

Chip8::Chip8() : loaded_rom_size(0),
                 draw_flag(false), // Initialize draw_flag
                 I(0),
                 pc(CHIP8_START_PROGRAM_ADDRESS),
                 sp(0),
                 delayTimer(0),
                 soundTimer(0)
{
    copy_sprite_font_into_memory();
}

void Chip8::copy_sprite_font_into_memory()
{
    for (int i = 0; i < CHIP8_FONT_BUFFER_SIZE; ++i)
    {
        memory[CHIP8_FONT_START_ADDRESS_IN_MEMORY + i] = sprite_font[i];
    }
}

void Chip8::clear_video_buffer()
{
    std::fill(std::begin(gfx), std::end(gfx), 0);
}

uint8_t Chip8::get_screen_pixel(int x, int y)
{
    if (x < 0 || x >= CHIP8_SCREEN_WIDTH || y < 0 || y >= CHIP8_SCREEN_HEIGHT) // Check if coordinates are in range
    {
        std::cerr << "Error: Screen coordinates out of range! x: " << x << ", y: " << y << std::endl;
        return 0; // Return 0 for pixels outside the screen
    }
    return gfx[y * 64 + x]; // Return the value of the pixel from the screen buffer
}

bool Chip8::pc_instruction_overflow()
{
    return pc >= (CHIP8_START_PROGRAM_ADDRESS + loaded_rom_size);
}

void Chip8::set_key(uint8_t key_index, uint8_t value)
{
    if (key_index < CHIP8_KEYBOARD_SIZE)
    {
        key[key_index] = value;
    }
    else
    {
        std::cerr << "Error: Key index out of range! Index: " << static_cast<int>(key_index) << std::endl;
    }
}

void Chip8::emulate_cycle()
{
    // === FETCH ===
    if (pc >= (CHIP8_MEMORY_SIZE - 1))
    {
        std::cerr << "Error: pc out of memory range! pc = 0x" << std::hex << pc << std::endl;
        return; // End cycle if pc is out of range
    }

    uint16_t opcode = (static_cast<uint16_t>(memory[pc]) << 8) | memory[pc + 1];

    std::cout << "Opcode: 0x" << std::hex << opcode << " (PC: 0x" << std::hex << pc << ")" << std::endl;

    pc += 2; // Increase PC before executing the instruction

    // === DECODE + EXECUTE ===
    switch (opcode & 0xF000)
    {
    case 0x0000:
        switch (opcode)
        {
        case 0x00E0:              // CLS: clear the screen
            clear_video_buffer(); // Use the existing function
            break;

        case 0x00EE: // RET: return from subroutine
            if (sp == 0)
            {
                std::cerr << "Error: Stack Underflow! Attempt to return from an empty stack." << std::endl;
                return;
            }
            sp--;
            pc = stack[sp];
            break;
        default:
            std::cerr << "Unknown system instruction: 0x" << std::hex << opcode << std::endl;
            break;
        }
        break;

    case 0x1000:
    { // JP addr: jump to address NNN
        uint16_t addr = opcode & 0x0FFF;
        pc = addr;
        break;
    }
    case 0x2000: // CALL addr: call subroutine
        if (sp >= CHIP8_STACK_SIZE)
        {
            std::cerr << "Error: Stack Overflow! Attempt to exceed stack size." << std::endl;
            return;
        }
        stack[sp++] = pc;     // Save current PC on the stack
        pc = opcode & 0x0FFF; // Jump to address NNN
        break;
    case 0x3000:
    { // SE Vx, byte: if Vx == NN
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t nn = opcode & 0x00FF;
        if (V[x] == nn)
        {
            pc += 2; // Skip next instruction
        }
        break;
    }
    case 0x4000:
    { // SNE Vx, byte: if Vx != NN
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t nn = opcode & 0x00FF;
        if (V[x] != nn)
        {
            pc += 2; // Skip next instruction
        }
        break;
    }
    case 0x5000:
    { // SE Vx, Vy: if Vx == Vy
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t y = (opcode & 0x00F0) >> 4;
        if (V[x] == V[y])
        {
            pc += 2; // Skip next instruction
        }
        break;
    }
    case 0x6000:
    { // LD Vx, byte: Vx = NN
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t nn = opcode & 0x00FF;
        V[x] = nn;
        break;
    }
    case 0x7000:
    { // ADD Vx, byte: Vx += NN
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t nn = opcode & 0x00FF;
        V[x] += nn;
        break;
    }
    case 0x8000:
        switch (opcode & 0x000F)
        {
        case 0x0000: // LD Vx, Vy: Vx = Vy
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            V[x] = V[y];
            break;
        }
        case 0x0001: // OR Vx, Vy: Vx |= Vy
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            V[x] |= V[y];
            break;
        }
        case 0x0002: // AND Vx, Vy: Vx &= Vy
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            V[x] &= V[y];
            break;
        }
        case 0x0003: // XOR Vx, Vy: Vx ^= Vy
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            V[x] ^= V[y];
            break;
        }
        case 0x0004: // ADD Vx, Vy: Vx += Vy
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            uint16_t sum = static_cast<uint16_t>(V[x]) + static_cast<uint16_t>(V[y]);
            V[0xF] = (sum > 255) ? 1 : 0; // Carry flag
            V[x] = sum & 0xFF;            // Store result in Vx
            break;
        }
        case 0x0005: // SUB Vx, Vy: Vx -= Vy
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            V[0xF] = (V[x] > V[y]) ? 1 : 0; // Borrow flag
            V[x] -= V[y];
            break;
        }
        case 0x0006: // SHR Vx {, Vy}: Vx >>= 1
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            V[0xF] = V[x] & 0x01; // Carry flag
            V[x] >>= 1;           // Shift right
            break;
        }
        case 0x0007: // SUBN Vx, Vy: Vx = Vy - Vx
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;
            V[0xF] = (V[y] > V[x]) ? 1 : 0; // Borrow flag
            V[x] = V[y] - V[x];
            break;
        }
        case 0x000E: // SHL Vx {, Vy}: Vx <<= 1
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            V[0xF] = (V[x] & 0x80) >> 7; // Carry flag
            V[x] <<= 1;                  // Shift left
            break;
        }
        default:
            std::cerr << "Unknown 0x8000 instruction: 0x" << std::hex << opcode << std::endl;
            break;
        }
        break;
    case 0xA000: // LD I, addr: I = NNN
        I = opcode & 0x0FFF;
        break;
    case 0xB000: // JP V0, addr: jump to address NNN + V0
    {
        uint16_t addr = opcode & 0x0FFF;
        pc = V[0] + addr; // Jump to address with V0 offset
        break;
    }
    case 0xC000: // RND Vx, byte: Vx = (random number & NN)
    {
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t nn = opcode & 0x00FF;
        V[x] = (rand() % 256) & nn; // Random number in range 0-255
        break;
    }
    case 0xD000:
    { // DRW Vx, Vy, nibble: Draw sprite
        uint8_t x = (opcode & 0x0F00) >> 8;
        uint8_t y = (opcode & 0x00F0) >> 4;
        uint8_t height = opcode & 0x000F;

        draw_sprite(V[x], V[y], height);

        break;
    }
    case 0xE000:
        switch (opcode & 0x00FF)
        {
        case 0x009E: // SKP Vx: if key Vx is pressed
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            if (key[V[x]] != 0)
            {
                pc += 2; // Skip next instruction
            }
            break;
        }
        case 0x00A1: // SKNP Vx: if key Vx is not pressed
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            if (key[V[x]] == 0)
            {
                pc += 2; // Skip next instruction
            }
            break;
        }
        default:
            std::cerr << "Unknown E000 instruction: 0x" << std::hex << opcode << std::endl;
            break;
        }
        break;
    case 0xF000:
        switch (opcode & 0x00FF)
        {
        case 0x0007: // LD Vx, DT: Vx = delayTimer
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            V[x] = delayTimer;
            break;
        }
        case 0x000A: // LD Vx, K: wait for key press
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            bool key_pressed = false;
            for (int i = 0; i < CHIP8_KEYBOARD_SIZE; ++i)
            {
                if (key[i] != 0)
                {
                    V[x] = i; // Store key index in Vx
                    key_pressed = true;
                    break;
                }
            }
            if (!key_pressed)
            {
                pc -= 2; // Revert PC if no key was pressed
            }
            break;
        }
        case 0x0015: // LD DT, Vx: delayTimer = Vx
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            delayTimer = V[x];
            break;
        }
        case 0x0018: // LD ST, Vx: soundTimer = Vx
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            soundTimer = V[x];
            break;
        }
        case 0x001E: // ADD I, Vx: I += Vx
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            I += V[x];
            if (I >= CHIP8_MEMORY_SIZE)
            {
                std::cerr << "Error: I exceeded memory range! I = 0x" << std::hex << I << std::endl;
                I = 0; // Reset I if out of range
            }
            break;
        }
        case 0x0029: // LD F, Vx: I = font address for Vx
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            if (V[x] >= 16)
            {
                std::cerr << "Error: Vx out of font range! Vx = " << static_cast<int>(V[x]) << std::endl;
                I = 0; // Reset I if Vx out of range
            }
            else
            {
                I = CHIP8_FONT_START_ADDRESS_IN_MEMORY + (V[x] * 5);
            }
            break;
        }
        case 0x0033: // LD B, Vx: memory[I] = BCD representation of Vx
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t value = V[x];
            memory[I] = value / 100;           // Set hundreds
            memory[I + 1] = (value / 10) % 10; // Set tens
            memory[I + 2] = value % 10;        // Set units
            break;
        }
        case 0x0055: // LD [I], Vx: memory[I] = V0, V1, ..., Vx
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            if (I + x >= CHIP8_MEMORY_SIZE)
            {
                std::cerr << "Error: I + x exceeded memory range! I = 0x"
                          << std::hex << I << ", x = " << static_cast<int>(x) << std::endl;
                return; // End cycle if out of range
            }
            for (uint8_t i = 0; i <= x; ++i)
            {
                memory[I + i] = V[i];
            }
            break;
        }
        case 0x0065: // LD Vx, [I]: V0, V1, ..., Vx = memory[I]
        {
            uint8_t x = (opcode & 0x0F00) >> 8;
            if (I + x >= CHIP8_MEMORY_SIZE)
            {
                std::cerr << "Error: I + x exceeded memory range! I = 0x"
                          << std::hex << I << ", x = " << static_cast<int>(x) << std::endl;
                return; // End cycle if out of range
            }
            for (uint8_t i = 0; i <= x; ++i)
            {
                V[i] = memory[I + i];
            }
            break;
        }
        default:
            std::cerr << "Unknown F000 instruction: 0x" << std::hex << opcode << std::endl;
            break;
        }
        break;
    default:
        std::cerr << "Unknown instruction (main switch): 0x" << std::hex << opcode << std::endl;
        break;
    }

    // === Timers ===
    if (delayTimer > 0)
    {
        delayTimer--;
    }

    if (soundTimer > 0)
    {
        soundTimer--;
    }
}

bool Chip8::load_rom(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "Cannot open ROM file: " << filename << std::endl;
        return false;
    }

    loaded_rom_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (loaded_rom_size > (CHIP8_MEMORY_SIZE - CHIP8_START_PROGRAM_ADDRESS))
    {
        std::cerr << "ROM is too large! Maximum size: "
                  << (CHIP8_MEMORY_SIZE - CHIP8_START_PROGRAM_ADDRESS)
                  << " bytes." << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char *>(&memory[CHIP8_START_PROGRAM_ADDRESS]), loaded_rom_size);
    file.close();
    std::cout << "ROM loaded: " << loaded_rom_size << " bytes." << std::endl;
    return true;
}

void Chip8::draw_sprite(uint8_t x_coord, uint8_t y_coord, uint8_t height)
{
    V[0xF] = 0; // Set collision flag to 0 at the beginning

    x_coord %= 64;
    y_coord %= 32;

    for (int row = 0; row < height; ++row)
    {
        // Check if the drawn sprite does not go beyond the vertical screen boundaries
        if (y_coord + row >= 32)
        {
            break; // Stop if the sprite goes beyond the bottom edge
        }

        uint8_t sprite_byte = memory[I + row];

        for (int col = 0; col < 8; ++col)
        {
            uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 0x01;
            int screen_x = (x_coord + col) % 64; // Wrapping for X
            int screen_y = (y_coord + row) % 32; // Wrapping for Y

            int pixel_index = screen_y * 64 + screen_x;

            if (gfx[pixel_index] == 1 && sprite_pixel == 1)
            {
                V[0xF] = 1;
            }
            gfx[pixel_index] ^= sprite_pixel;
        }
    }
    draw_flag = true;
}

void Chip8::render_screen(bool debug_draw_to_console)
{
    if (!draw_flag)
    {
        return;
    }

    if (debug_draw_to_console)
    {
        std::cout << "\n--- CHIP-8 SCREEN ---\n";
        for (int y = 0; y < CHIP8_SCREEN_HEIGHT; ++y)
        {
            for (int x = 0; x < CHIP8_SCREEN_HEIGHT; ++x)
            {
                std::cout << (gfx[y * 64 + x] ? '#' : ' ');
            }
            std::cout << '\n'; // Use '\n' instead of std::endl for performance in a loop
        }
        std::cout << "---------------------\n\n";
    }
    draw_flag = false; // Reset the flag after the graphics system
}