// C99
#include <stdio.h>  // stderr, fprintf, snprintf, fputs
#include <string.h> // strlen
#include <stdlib.h> // strtoul

#include "src/tim.h"

#ifdef _WIN32
    #define DIR_SEP "\\"
#else
    #define DIR_SEP "/"
#endif

static const char *steps[] = {
    "reading input file",
    "resizing",
    "writing output file",
    "displaying gui"
};

static const char *msg[] = {
    "no error",
    "memory allocation failed",
    "invalid arguments were passed",
    "underlying implementation failed"
};

int main(int argc, const char *const *argv)
{
    // windows limits MAX_PATH to 256
    // TODO: improve this
    char dst_path[256] = "resized.jpg";
    size_t new_w = 0, new_h = 0;
    tim_img original_image, edited_image;
    tim_err err;
    int s = 0;

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

    err = tim_file_read(&original_image, argv[1]);
    if (err) goto failure;
    s++;

    // calculate percentage
    if (argv[2][strlen(argv[2]) - 1] == '%')
        new_w = (((float)new_w / 100.0f) * (float)original_image.width);

    if (argc >= 4 && argv[3][strlen(argv[3]) - 1] == '%')
        new_h = (((float)new_h / 100.0f) * (float)original_image.height);

    err = tim_resize(&original_image, &edited_image, new_w, new_h);
    if (err) goto failure;
    s++;

    err = tim_file_write(&edited_image, dst_path);
    if (err) goto failure;
    s++;

    err = tim_display(&edited_image);
    if (err) goto failure;

    return tim_free(&edited_image) | tim_free(&original_image);
    failure: return fprintf(stderr, "%s failed: %s\n", steps[s], msg[err]);
}
