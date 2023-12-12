// Tiny Image Manipulation - C99

#ifndef __TIM_H__
#define __TIM_H__

#include <stddef.h> // size_t

// clang kept saying stdint.h is unused included
typedef unsigned char u8;

// 4ch 8bpc
typedef struct {
  u8 alpha, red, green, blue;
} tim_pixel;

typedef struct {
  int width, height, channels;
  u8 *pixels;
} tim_img;

typedef enum {
  // no error
  TIM_ERR_OK,
  // allocation error
  TIM_ERR_ALLOC,
  // invalid arguments (possibly null) were passed to function
  TIM_ERR_ARG,
  // the underlying library failed with arbitrary error
  TIM_ERR_INTERNAL
} tim_err;

typedef enum { TIM_FILTER_GRAYSCALE } tim_filter;

/** init a new empty 8bpc image */
tim_err tim_init(tim_img *im, size_t width, size_t height, size_t channels);

/** read image from file */
tim_err tim_file_read(tim_img *im, const char *file);

/** write image to a file */
tim_err tim_file_write(tim_img *im, const char *file);

/** resize an image to the given dimensions */
tim_err tim_resize(tim_img *im, tim_img *dst, size_t new_width,
                   size_t new_height);

/** apply operation `f` on `im` and save it to `dst`. dst will be allocated. */
tim_err tim_apply(tim_img *im, tim_img *dst, tim_filter f);

/** get pixel at (x, y) */
tim_err tim_pixel_get(tim_img *im, size_t x, size_t y, tim_pixel *dst);

/** set pixel at (x, y) */
tim_err tim_pixel_set(tim_img *im, size_t x, size_t y, tim_pixel *src);

/** display the image in a gui */
tim_err tim_display(tim_img *im);

/** clear image buffer and free the allocated memory */
tim_err tim_free(tim_img *im);

#endif // __TIM_H__