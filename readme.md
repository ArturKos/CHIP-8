
<img width="1023" height="801" alt="chip-8_welcome_screen" src="https://github.com/user-attachments/assets/2012100c-40e6-4a47-ac84-8ec8e1b190e0" />

YouTube link:
https://youtu.be/ehfH5C3WpYU

# CHIP-8 Emulator

A simple CHIP-8 emulator written in C/C++ using the Allegro 5 library for graphics, sound, and input.

## Features

- Emulates the CHIP-8 virtual machine
- Graphical output (windowed)
- Keyboard input mapping
- Sound support (beep)
- Loads ROMs from file

## Requirements

- C++17 or newer
- Allegro 5 (with modules: allegro, allegro_font, allegro_image, allegro_primitives, allegro_audio, allegro_acodec)
- CMake (for building)
- A CHIP-8 ROM file (e.g. Pong.ch8, IBM_Logo.ch8)

## Building

1. Install Allegro 5 and CMake (on Ubuntu/Mint):

    ```bash
    sudo apt install liballegro5-dev cmake
    ```

2. Clone the repository and build:

    ```bash
    git clone https://github.com/ArturKos/CHIP-8.git
    cd CHIP-8
    mkdir build
    cd build
    cmake ..
    make
    ```

## Usage

```bash
./chip8_emulator ../roms/Pong.ch8
```
If no ROM is provided, the emulator will try to load a default ROM (e.g. Pong.ch8).

## Controls

| CHIP-8 Key | Keyboard Mapping |
|------------|------------------|
| 1 2 3 C    | 1 2 3 4          |
| 4 5 6 D    | Q W E R          |
| 7 8 9 E    | A S D F          |
| A 0 B F    | Z X C V          |

Press `ESC` to exit.

## Sound

The emulator plays a beep sound when the CHIP-8 sound timer is active.  
Make sure you have a `beep.wav` file in the executable directory.  
You can generate one using:

```bash
sox -n -r 44100 -c 1 beep.wav synth 0.1 sine 880
```

## Example ROMs

You can find public domain CHIP-8 ROMs here:
- https://github.com/dmatlack/chip8/tree/master/roms

## License

MIT License

---

**Author:** Artur Kos  
**Project inspired by:**  
- https://tobiasvl.github.io/blog/write-a-chip
- https://en.wikipedia.org/wiki/CHIP-8
