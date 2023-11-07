cmake_minimum_required(VERSION 3.25)
project(tim2 C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(tim2
        src/stb_image.h
        src/stb_image_write.h
        src/tim.h
        src/tim_stb_sdl.c
        resize.c)
