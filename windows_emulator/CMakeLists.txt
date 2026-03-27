cmake_minimum_required(VERSION 3.10)
project(chip8_emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

add_executable(chip8_emulator WIN32 main.cpp ../CHIP-8/src/chip-8.cpp)
target_link_libraries(chip8_emulator winmm)

if(MSVC)
    target_compile_options(chip8_emulator PRIVATE /W4)
else()
    target_compile_options(chip8_emulator PRIVATE -Wall -Wextra -pedantic)
endif()
