// Tiny Image Manipulation - C99

#ifndef __TIM_H__
#define __TIM_H__

#include <stdint.h> // uint8_t
#include <stddef.h> // size_t

// 4ch 8bit TrueColor
// ! NOTE: implementations may discard the alpha channel
typedef struct Pixel {
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Pixel;

typedef struct Image {
    int width;
    int height;
    int channels;
    uint8_t *pixels;
} Image;

/** create a new empty 8bpc 3ch 24-bit RGB image */
Image tim_new(size_t width, size_t height);

/** read image from file */
Image tim_read(const char *file);

/** resize an image to the given dimensions */
Image tim_resize(Image *image, size_t new_width, size_t new_height);

/** get pixel at (x, y) */
Pixel tim_pixel(Image *image, size_t x, size_t y);

/** write image to a file */
int tim_write(Image *image, const char *file);

/** display the image in a gui */
void tim_display(Image *image);

/** clear image buffer and free the allocated memory */
int tim_free(Image *image);

#endif // __TIM_H__