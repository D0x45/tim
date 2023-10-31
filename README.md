# Tiny Image Manipulation
just a simple c99 library for resizing images

# Build
the default implementation uses `SDL2` and [nothings/stb](https://github.com/nothings/stb)

build with:
```sh
gcc resize.c src/tim_stb_sdl.c -g -std=c99 -o resize -lm -DTIM_IMPL_DISPLAY -lSDL2 -lSDL2_image -DDEBUG
```

use `-DTIM_IMPL_DISPLAY -lSDL2 -lSDL2_image` for implementing `tim_display(...)` if you have SDL installed, otherwise omit this flag.

use `-DDEBUG` to allow debug logs.

compiled with `gcc 12` on `Debian 12`.
