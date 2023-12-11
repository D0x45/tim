# Tiny Image Manipulation
just a simple c99 library for manipulating images

# Build
the default implementation uses `SDL2` and [nothings/stb](https://github.com/nothings/stb)

debug build with:
```sh
gcc resize.c src/tim_stb_sdl.c -g -std=c99 -o resize -lm -DTIM_IMPL_DISPLAY -lSDL2 -lSDL2_image -DDEBUG
```

tiny build with:
```sh
gcc resize.c src/tim_stb_sdl.c -O3 -std=c99 -o resize -lm
```

or use cmake

```sh
mkdir build
cmake build
cd build
make
```

use `-DTIM_IMPL_DISPLAY -lSDL2 -lSDL2_image` for implementing `tim_display(...)` if you have SDL installed, otherwise omit this flag.

use `-DDEBUG` to allow debug logs.

tested with `gcc 12` / `clang 14` on `Debian 12`.
