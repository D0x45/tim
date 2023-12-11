// Tiny Image Manipulation - C99

#ifndef __TIM_H__
#define __TIM_H__

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t

// 4ch 8bpc
typedef struct {
  uint8_t alpha;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} TIM_Pixel;

typedef struct {
  int width;
  int height;
  int channels;
  uint8_t *pixels;
} TIM_Image;

typedef enum {
  // no error
  E_OK,
  // allocation error
  E_ALLOC,
  // invalid arguments (possibly null) were passed to function
  E_INVALID_ARG,
  // the underlying library failed with arbitrary error
  E_INTERNAL
} TIM_Result;

typedef enum {
  FILTER_GRAYSCALE
} TIM_Filter;

/** init a new empty 8bpc image */
TIM_Result tim_init(TIM_Image *im, size_t width, size_t height, size_t channels);

/** read image from file */
TIM_Result tim_file_read(TIM_Image *im, const char *file);

/** write image to a file */
TIM_Result tim_file_write(TIM_Image *im, const char *file);

/** resize an image to the given dimensions */
TIM_Result tim_resize(TIM_Image *im, TIM_Image *dst, size_t new_width, size_t new_height);

/** apply filter `f` on `im` and save it to `dst`. dst will be allocated. */
TIM_Result tim_filter(TIM_Image *im, TIM_Image *dst, TIM_Filter f);

/** get pixel at (x, y) */
TIM_Result tim_pixel_get(TIM_Image *im, size_t x, size_t y, TIM_Pixel *dst);

/** set pixel at (x, y) */
TIM_Result tim_pixel_set(TIM_Image *im, size_t x, size_t y, TIM_Pixel *src);

/** display the image in a gui */
TIM_Result tim_display(TIM_Image *im);

/** clear image buffer and free the allocated memory */
TIM_Result tim_free(TIM_Image *im);

#endif // __TIM_H__