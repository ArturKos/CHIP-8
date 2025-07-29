cmake_minimum_required(VERSION 3.10)
project(chip8_emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig REQUIRED)
add_executable(chip8_emulator main.cpp ../CHIP-8/src/chip-8.cpp)
pkg_check_modules(ALLEGRO5 REQUIRED allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5 ibus-table RapidJSON)
target_link_libraries(chip8_emulator  ${ALLEGRO5_LIBRARIES})
target_include_directories(chip8_emulator PUBLIC ${ALLEGRO5_INCLUDE_DIRS})

  if(MSVC)
	  target_compile_options(chip8_emulator PRIVATE /wall)
  else()
    target_compile_options(chip8_emulator PRIVATE -Wall -Wextra -pedantic)
  endif()