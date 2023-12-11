// C99
#include <stdio.h>  // fprintf, fputs, stderr
#include <string.h> // strlen
#include <stdlib.h> // strtoul

#include "src/tim.h" // Tiny Image Manipulation

int main(int argc, const char *const *argv)
{
    // windows limits MAX_PATH to 256
    // TODO: implement path join
    char dst_path[256] = "resized.jpg";
    size_t new_w = 0, new_h = 0;
    TIM_Image original_image, edited_image;
    TIM_Result tim_res = E_INTERNAL;

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

    tim_res = tim_file_read(&original_image, argv[1]);

    if (tim_res != E_OK) {
        fputs("there was a problem reading input file\n", stderr);
        return -2;
    }

    // calculate percentage
    if (argv[2][strlen(argv[2]) - 1] == '%')
        new_w = (((float)new_w / 100.0f) * (float)original_image.width);

    if (argc >= 4 && argv[3][strlen(argv[3]) - 1] == '%')
        new_h = (((float)new_h / 100.0f) * (float)original_image.height);

    tim_res = tim_resize(&original_image, &edited_image, new_w, new_h);
    
    if (tim_res != E_OK) {
        fputs("resize failed\n", stderr);
        return -3;
    }

    if (tim_file_write(&edited_image, dst_path) != E_OK) {
        fputs("image write failed\n", stderr);
        return -4;
    }

    tim_display(&edited_image);

    return tim_free(&edited_image) | tim_free(&original_image);
}
