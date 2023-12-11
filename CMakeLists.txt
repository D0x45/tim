cmake_minimum_required(VERSION 3.25)
project(tim C)

set(CMAKE_C_STANDARD 99)

# static lib
add_library(tim STATIC "src/tim_stb_sdl.c")

# math library
target_link_libraries(tim "m")

# add and link resize with libtim
add_executable(resize "resize.c")
target_link_libraries(resize tim)
