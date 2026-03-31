
<img width="1023" height="801" alt="chip-8_welcome_screen" src="https://github.com/user-attachments/assets/2012100c-40e6-4a47-ac84-8ec8e1b190e0" />

YouTube link:
https://youtu.be/ehfH5C3WpYU

# CHIP-8 Emulator

A fully-featured CHIP-8 virtual machine emulator written in **C++17** with an **Allegro 5** graphical frontend. Implements the complete original CHIP-8 instruction set with sound support, hex keypad mapping, and debug console output.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![Allegro](https://img.shields.io/badge/Allegro-5-green)
![CMake](https://img.shields.io/badge/CMake-3.10+-orange)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey)
![Win32](https://img.shields.io/badge/Win32-GDI%20%2B%20WinMM-0078D6?logo=windows&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-yellow)

## Features

- **Complete CHIP-8 instruction set** -- all 35 opcodes implemented (0x0nnn through 0xFx65)
- **4 KB memory** with ROM loading starting at address 0x200
- **16 general-purpose 8-bit registers** (V0-VF) with VF as carry/borrow flag
- **64x32 monochrome display** scaled to a 1024x768 window
- **16-level call stack** with overflow/underflow detection
- **Delay and sound timers** decrementing at each cycle
- **Sound support** -- plays a WAV beep when the sound timer is active
- **Hex keypad mapping** -- 16-key input mapped to a standard QWERTY keyboard layout
- **XOR sprite drawing** with pixel collision detection (VF flag)
- **Built-in font sprites** for hexadecimal digits 0-F loaded at address 0x50
- **BCD conversion** (opcode Fx33) for decimal display of register values
- **Debug output** -- each executed opcode is logged to stdout with the program counter address
- **Console ASCII renderer** -- optional text-mode display for debugging without a graphical window
- **Boundary checking** with error messages for stack overflow, memory overflow, and invalid key indices
- **Cross-platform** -- separate build configurations for Linux and Windows

## Architecture

The emulator is split into two layers:

| Layer | Directory | Description |
|-------|-----------|-------------|
| Core | `CHIP-8/src/` | Platform-independent emulation engine (`Chip8` class) |
| Frontend | `linux_emulator/` | Allegro 5 graphical frontend, keyboard mapping, sound playback |
| Frontend | `windows_emulator/` | Native Win32 (GDI + WinMM) graphical frontend — no external dependencies |

The `Chip8` class exposes a clean API (`emulate_cycle()`, `load_rom()`, `get_screen_pixel()`, `set_key()`, `play_sound()`) that can be driven by any rendering backend.

## Dependencies

### Linux

| Library | Version | Purpose |
|---------|---------|---------|
| [Allegro 5](https://liballeg.org/) | >= 5.0 | Windowing, rendering, audio, fonts, image loading |
| CMake | >= 3.10 | Build system |
| pkg-config | any | Allegro 5 detection |

### Windows

| Library | Version | Purpose |
|---------|---------|---------|
| Win32 API (GDI) | built-in | Window creation, pixel rendering, double buffering |
| WinMM | built-in | WAV sound playback (`PlaySound`) |
| CMake | >= 3.10 | Build system |
| MSVC or MinGW | any | C++17 compiler |

The Windows version has **zero external dependencies** — it uses only the Win32 API and WinMM that ship with Windows.

### Installing dependencies (Ubuntu / Debian)

```bash
sudo apt-get install liballegro5-dev cmake pkg-config
```

## Building

### Linux (Allegro 5)

```bash
git clone https://github.com/ArturKos/CHIP-8.git
cd CHIP-8/linux_emulator
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows (Visual Studio / MSVC)

```cmd
git clone https://github.com/ArturKos/CHIP-8.git
cd CHIP-8\windows_emulator
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

The resulting `chip8_emulator.exe` will be in `build\Release\` (or `build\Debug\` for debug builds).

### Windows (MinGW)

```cmd
git clone https://github.com/ArturKos/CHIP-8.git
cd CHIP-8\windows_emulator
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

## Usage

### Linux

```bash
./chip8_emulator <path_to_rom>
```

Example:
```bash
./chip8_emulator ../../roms/Pong.ch8
```

### Windows

```cmd
chip8_emulator.exe <path_to_rom>
```

Example:
```cmd
chip8_emulator.exe ..\..\roms\Pong.ch8
```

You can also drag and drop a ROM file onto `chip8_emulator.exe` to launch it.

The emulator displays a welcome screen with key mapping information. Press **SPACE** to start execution.

## Controls

The CHIP-8 hex keypad is mapped to a QWERTY keyboard as follows:

| CHIP-8 Key | Keyboard |
|------------|----------|
| `1 2 3 C`  | `1 2 3 4` |
| `4 5 6 D`  | `Q W E R` |
| `7 8 9 E`  | `A S D F` |
| `A 0 B F`  | `Z X C V` |

| Action | Input |
|--------|-------|
| Start emulation | Space |
| Exit | Escape |

## Sound

The emulator plays a beep sound when the CHIP-8 sound timer is active. Place a `beep.wav` file in the `sounds/` directory. You can generate one with:

```bash
sox -n -r 44100 -c 1 beep.wav synth 0.1 sine 880
```

## Supported Opcodes

| Opcode | Mnemonic | Description |
|--------|----------|-------------|
| `00E0` | CLS | Clear the display |
| `00EE` | RET | Return from subroutine |
| `1NNN` | JP addr | Jump to address NNN |
| `2NNN` | CALL addr | Call subroutine at NNN |
| `3XNN` | SE Vx, byte | Skip next if Vx == NN |
| `4XNN` | SNE Vx, byte | Skip next if Vx != NN |
| `5XY0` | SE Vx, Vy | Skip next if Vx == Vy |
| `6XNN` | LD Vx, byte | Set Vx = NN |
| `7XNN` | ADD Vx, byte | Set Vx = Vx + NN |
| `8XY0` | LD Vx, Vy | Set Vx = Vy |
| `8XY1` | OR Vx, Vy | Set Vx = Vx OR Vy |
| `8XY2` | AND Vx, Vy | Set Vx = Vx AND Vy |
| `8XY3` | XOR Vx, Vy | Set Vx = Vx XOR Vy |
| `8XY4` | ADD Vx, Vy | Set Vx = Vx + Vy, VF = carry |
| `8XY5` | SUB Vx, Vy | Set Vx = Vx - Vy, VF = NOT borrow |
| `8XY6` | SHR Vx | Vx >>= 1, VF = LSB |
| `8XY7` | SUBN Vx, Vy | Set Vx = Vy - Vx, VF = NOT borrow |
| `8XYE` | SHL Vx | Vx <<= 1, VF = MSB |
| `9XY0` | SNE Vx, Vy | Skip next if Vx != Vy |
| `ANNN` | LD I, addr | Set I = NNN |
| `BNNN` | JP V0, addr | Jump to NNN + V0 |
| `CXNN` | RND Vx, byte | Vx = random AND NN |
| `DXYN` | DRW Vx, Vy, n | Draw sprite at (Vx, Vy), height N |
| `EX9E` | SKP Vx | Skip next if key Vx is pressed |
| `EXA1` | SKNP Vx | Skip next if key Vx is not pressed |
| `FX07` | LD Vx, DT | Set Vx = delay timer |
| `FX0A` | LD Vx, K | Wait for key press, store in Vx |
| `FX15` | LD DT, Vx | Set delay timer = Vx |
| `FX18` | LD ST, Vx | Set sound timer = Vx |
| `FX1E` | ADD I, Vx | Set I = I + Vx |
| `FX29` | LD F, Vx | Set I = font sprite address for Vx |
| `FX33` | LD B, Vx | Store BCD of Vx at I, I+1, I+2 |
| `FX55` | LD [I], Vx | Store V0..Vx in memory starting at I |
| `FX65` | LD Vx, [I] | Load V0..Vx from memory starting at I |

## Example ROMs

Public domain CHIP-8 ROMs can be found at:
- https://github.com/dmatlack/chip8/tree/master/roms

## Project Structure

```
CHIP-8/
├── readme.md                       # This file
├── CHIP-8/
│   └── src/
│       ├── chip-8.hpp              # Chip8 class declaration, memory layout, constants
│       └── chip-8.cpp              # Emulation core: fetch-decode-execute, ROM loading,
│                                   #   sprite drawing, timer management
├── linux_emulator/
│   ├── CMakeLists.txt              # Linux build configuration (C++17, Allegro 5)
│   └── main.cpp                    # Allegro 5 frontend: rendering, keypad mapping, sound
├── windows_emulator/
│   ├── CMakeLists.txt              # Windows build configuration (C++17, Win32 + WinMM)
│   └── main.cpp                    # Native Win32 frontend: GDI rendering, WinMM sound,
│                                   #   double-buffered display, keyboard via GetAsyncKeyState
├── images/
│   └── background.jpeg             # Welcome screen background image
└── sounds/
    └── beep.wav                    # Sound timer beep sample
```

## References

- https://tobiasvl.github.io/blog/write-a-chip -- CHIP-8 implementation guide
- https://en.wikipedia.org/wiki/CHIP-8 -- CHIP-8 specification

## License

MIT License

---

**Author:** Artur Kos
