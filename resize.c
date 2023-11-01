// C99
#include <stdio.h>  // fprintf, stderr
#include <stdint.h> // uint8_t
#include <string.h> // strlen
#include <stdlib.h> // strtoul

#include "src/tim.h" // Tiny Image Manipulation

int main(int argc, const char *const *argv)
{
    // windows limits MAX_PATH to 256
    // TODO: implement path join
    char dst_path[256] = "resized.jpg";
    Image original_image, resized_image;
    size_t new_w = 0, new_h = 0;

    if (argc < 3) {
        fprintf(
            stderr,
            "invalid arguments\n"
            "USAGE: %1$s FILENAME NEW_WIDTH [NEW_HEIGHT]\n\n"
            "new_width could be any arbitrary value that can be evaluated\n"
            "either as an absolute pixel count or a percent\n"
            "zero means no scaling happens\n\n"
            "e.g.: %1$s image.jpg 100%% 50%%\n"
            "e.g.: %1$s image.jpg 100 150\n"
            "e.g.: %1$s image.jpg 1920 120%%\n"
            "e.g.: %1$s image.jpg 150%%\n\n",
            argv[0]); // not iso c. hope it works on the target compiler
        return -1;
    }

    // parse input dimensions
    new_w = strtoul(argv[2], NULL, 10);
    new_h = argc >= 4 ? strtoul(argv[3], NULL, 10) : 0;

    original_image = tim_read(argv[1]);

    if (original_image.height == 0 || original_image.width == 0 || original_image.channels == 0)
        return -2;

    // calculate percentage
    if (argv[2][strlen(argv[2]) - 1] == '%')
        new_w = (((float)new_w / 100.0f) * (float)original_image.width);

    if (argc >= 4 && argv[3][strlen(argv[3]) - 1] == '%')
        new_h = (((float)new_h / 100.0f) * (float)original_image.height);

    tim_display(&original_image);

    resized_image = tim_resize(&original_image, new_w, new_h);

    if (tim_write(&resized_image, dst_path) <= 0)
        return -3;

    tim_display(&resized_image);

    // tim_resize returns the same image buffer, so free()ing it twice causes segfault
    return (original_image.pixels != resized_image.pixels ? tim_free(&resized_image) : 0)
        | tim_free(&original_image);
}
