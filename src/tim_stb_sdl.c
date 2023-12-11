// C99
#include <stddef.h> // NULL
#include <stdint.h> // uint8_t
#include <stdio.h>  // stderr, fprintf, snprintf
#include <stdlib.h> // calloc, free

#include "tim.h" // Tiny Image Manipulation

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

// allow implementing tim_display() with -lSDL2 -lSDL2_image -DTIM_IMPL_DISPLAY
#ifdef TIM_IMPL_DISPLAY
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#define TIM_WINDOW_W 1024
#define TIM_WINDOW_H 720
#define TIM_MIN(a, b) ((a) < (b) ? (a) : (b))
#define TIM_STRINGIFY_(x) #x
#define TIM_STRINGIFY(x) TIM_STRINGIFY_(x)
#define TIM_SDL_NullCheckERR(buffer)                                           \
  if ((buffer) == NULL) {                                                      \
    fprintf(stderr, TIM_STRINGIFY(buffer) " is NULL, %s\n", SDL_GetError());   \
    return E_INTERNAL;                                                         \
  }
#endif

// color channel offsets for RGBA
#define RGBA_C0 0 // red
#define RGBA_C1 1 // green
#define RGBA_C2 2 // blue
#define RGBA_C3 3 // alpha

// deref a color ptr either for assigning or reading its value
#define PX(im, x, y, c) *(im->pixels + ((x + im->width * y) * im->channels) + c)

// debugging enabled
#ifdef DEBUG
#include <time.h> // time
#define TRACE(...) fprintf(stderr, __VA_ARGS__);
#define PX_DBG(image, x, y)                                                    \
  TRACE("Pixel (im: %p, x: %d, y: %d) { r: %u, g: %u, b: %u, a: %u }\n",       \
        image->pixels, x, y,                                                   \
        image->channels >= 1 ? PX(image, x, y, RGBA_C0) : 0x00,                \
        image->channels >= 2 ? PX(image, x, y, RGBA_C1) : 0x00,                \
        image->channels >= 3 ? PX(image, x, y, RGBA_C2) : 0x00,                \
        image->channels >= 4 ? PX(image, x, y, RGBA_C3) : 0x00);
#else
#define TRACE(...)
#define PX_DBG(a, b, c)
#endif

TIM_Result tim_init(TIM_Image *im, size_t width, size_t height, size_t channels) {
  TRACE("tim_init(%p, %ld, %ld, %ld) => { pixels: %p }\n", im, width, height,
        channels, (im == NULL) ? NULL : im->pixels);
  if (im == NULL || channels < 1 || channels > 4)
    return E_INVALID_ARG;
  im->width = width;
  im->height = height;
  im->channels = channels;
  im->pixels = calloc(width * height * channels, sizeof(uint8_t));
  TRACE("allocated addr %p with %ld bytes\n", im->pixels,
        width * height * channels);
  return (im->pixels == NULL) ? E_ALLOC : E_OK;
}

TIM_Result tim_file_read(TIM_Image *im, const char *file) {
#ifdef DEBUG
  time_t t_start = time(NULL);
#endif
  TRACE("tim_file_read(%p, %p)\n", im, file);

  if (im == NULL || file == NULL)
    return E_INVALID_ARG;

  im->pixels =
      stbi_load(file, &im->width, &im->height, &im->channels, STBI_default);

  if (im->pixels == NULL) {
    TRACE("stbi_err: %s\n", stbi_failure_reason());
    return E_INTERNAL;
  }

  TRACE("tim_file_read(%p, %s) => { w: %d, h: %d, ch: %d, px: %p } in %lds\n",
        im, file, im->width, im->height, im->channels, im->pixels,
        time(NULL) - t_start);

  // debug display a subsequent row of pixels
  PX_DBG(im, 0, 1);
  PX_DBG(im, 0, 2);
  PX_DBG(im, 0, 3);
  PX_DBG(im, 0, 4);
  PX_DBG(im, 0, 5);

  return E_OK;
}

TIM_Result tim_file_write(TIM_Image *im, const char *file) {
  int stbi_result;
#ifdef DEBUG
  time_t t_start = time(NULL);
#endif

  TRACE("tim_file_write(%p, %p)\n", im, file);

  if (im == NULL || im->pixels == NULL || file == NULL)
    return E_INVALID_ARG;

  // ignore input file format and save as jpg 100
  stbi_result = stbi_write_jpg(file, im->width, im->height, im->channels,
                               im->pixels, 100);
  if (stbi_result <= 0) {
    TRACE("stbi_err: %s\n", stbi_failure_reason());
    return E_INTERNAL;
  }

  TRACE("tim_file_write(%p, %s) took %ld seconds, stbi_result = %d\n", im, file,
        time(NULL) - t_start, stbi_result);

  return E_OK;
}

TIM_Result tim_pixel_get(TIM_Image *im, size_t x, size_t y, TIM_Pixel *dst) {
  TRACE("tim_pixel_get(%p, %ld, %ld, %p)\n", im, x, y, dst);

  if (im == NULL || x > im->width || y > im->height || dst == NULL)
    return E_INVALID_ARG;

  dst->red = (im->channels >= 1) ? PX(im, x, y, RGBA_C0) : 0x00;
  dst->green = (im->channels >= 2) ? PX(im, x, y, RGBA_C1) : 0x00;
  dst->blue = (im->channels >= 3) ? PX(im, x, y, RGBA_C2) : 0x00;
  dst->alpha = (im->channels >= 4) ? PX(im, x, y, RGBA_C3) : 0x00;

  return E_OK;
}

TIM_Result tim_pixel_set(TIM_Image *im, size_t x, size_t y, TIM_Pixel *src) {
  TRACE("tim_pixel_set(%p, %ld, %ld, %p)\n", im, x, y, src);

  if (im == NULL || x > im->width || y > im->height || src == NULL)
    return E_INVALID_ARG;

  PX(im, x, y, RGBA_C0) = src->red;

  if (im->channels >= 2)
    PX(im, x, y, RGBA_C1) = src->green;

  if (im->channels >= 3)
    PX(im, x, y, RGBA_C2) = src->blue;

  if (im->channels >= 4)
    PX(im, x, y, RGBA_C3) = src->alpha;

  return E_OK;
}

TIM_Result tim_free(TIM_Image *im) {
  TRACE("tim_free(%p)\n", im);
  if (im == NULL || im->pixels == NULL)
    return E_INVALID_ARG;
  free(im->pixels);
  im->height = 0;
  im->width = 0;
  im->channels = 0;
  im->pixels = NULL;
  return E_OK;
}

// relative luminance calculated from linear RGB components
static TIM_Result tim_grayscale(TIM_Image *im, TIM_Image *dst) {
  TIM_Result res;
  size_t x, y;
  float r, g, b;
  uint8_t gr;
  TRACE("tim_grayscale(%p, %p)\n", im, dst);

  if (im == NULL || dst == NULL || im->channels < 3)
    return E_INVALID_ARG;
  
  // since sdl does not support displaying 1ch image (or at least i could not figure it out)
  // and also apparently stbi_write_jpg writes 3 channels (or perhaps stbi_load reads 3 channels)
  // i am gonna do the same.
  // since this is a linear grayscale image, all three channels will hold the same value
  res = tim_init(dst, im->width, im->height, 3);
  if (res != E_OK)
    return res;

  for (x = 0; x < im->width; ++x) {
    for (y = 0; y < im->height; ++y) {
      r = 0.2126f * (float)PX(im, x, y, RGBA_C0);
      g = 0.7152f * (float)PX(im, x, y, RGBA_C1);
      b = 0.0722f * (float)PX(im, x, y, RGBA_C2);
      gr = r + g + b; // multiplied by alpha?
      PX(dst, x, y, RGBA_C0) = gr;
      PX(dst, x, y, RGBA_C1) = gr;
      PX(dst, x, y, RGBA_C2) = gr;
    }
  }

  return E_OK;
}

TIM_Result tim_filter(TIM_Image *im, TIM_Image *dst, TIM_Filter f) {
  switch (f) {
    case FILTER_GRAYSCALE: return tim_grayscale(im, dst);
  }
  return E_INVALID_ARG;
}

// nearest neighbor
TIM_Result tim_resize(TIM_Image *im, TIM_Image *dst, size_t new_width, size_t new_height) {
  TIM_Result init_res;
  float r_h, r_w;
  size_t dst_y, dst_x, src_x, src_y;
#ifdef DEBUG
  time_t t_start = time(NULL);
#endif

  TRACE("tim_resize(%p, %p, %ld, %ld)\n", im, dst, new_width, new_height);

  if (im == NULL || dst == NULL)
    return E_INVALID_ARG;

  // zero means no scaling happens at that dimension
  new_height = (new_height == 0) ? im->height : new_height;
  new_width = (new_width == 0) ? im->width : new_width;

  // return since nothing changed
  if (im->height == new_height && im->width == new_width)
    return E_INVALID_ARG;

  // calculate ratio
  r_h = (float)new_height / (float)im->height;
  r_w = (float)new_width / (float)im->width;
  TRACE("r_h=%f, r_w=%f\n", r_h, r_w);

  // new empty canvas
  init_res = tim_init(dst, new_width, new_height, im->channels);
  if (init_res != E_OK)
    return init_res;

  // down-scaling skips every r_w|r_h pixels
  // up-scaling duplicates every r_w|r_h pixels
  // iterating row-first (->)
  for (dst_y = 0; dst_y < new_height; ++dst_y) {
    for (dst_x = 0; dst_x < new_width; ++dst_x) {
      // translate destination (x,y) to source (x,y)
      src_x = (size_t)((float)dst_x / r_w);
      src_y = (size_t)((float)dst_y / r_h);

      // copy color channels:
      PX(dst, dst_x, dst_y, RGBA_C0) = PX(im, src_x, src_y, RGBA_C0);

      if (im->channels >= 2)
        PX(dst, dst_x, dst_y, RGBA_C1) = PX(im, src_x, src_y, RGBA_C1);
      if (im->channels >= 3)
        PX(dst, dst_x, dst_y, RGBA_C2) = PX(im, src_x, src_y, RGBA_C2);
      if (im->channels >= 4)
        PX(dst, dst_x, dst_y, RGBA_C3) = PX(im, src_x, src_y, RGBA_C3);
    }
  }

  TRACE("resize took: %ld seconds\n", time(NULL) - t_start);

  return E_OK;
}

TIM_Result tim_display(TIM_Image *im) {
  TRACE("tim_display(%p)\n", im);
#ifdef TIM_IMPL_DISPLAY
  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Surface *surface = NULL;
  SDL_Texture *display_texture = NULL;
  SDL_Event e;
  SDL_Rect bounding_box;
  char window_title[40] = {0};
  float ratio_w, ratio_h, proportion;

  // i am too tired to wrk around sdl not displaying 1ch grayscale images
  // TODO: find a way to display 1-channel grayscale images with sdl
  if (im == NULL || im->channels < 3)
    return E_INVALID_ARG;

  // calculate proportions to fit the image into WINDOW_H*WINDOW_W without
  // distorting it
  ratio_w = (float)TIM_WINDOW_W / (float)im->width;
  ratio_h = (float)TIM_WINDOW_H / (float)im->height;
  proportion = TIM_MIN(ratio_h, ratio_w);
  bounding_box = (SDL_Rect){.x = 0,
                                 .y = 0,
                                 .w = (int)(im->width * proportion),
                                 .h = (int)(im->height * proportion)};
  snprintf(window_title, sizeof(window_title), "TIM Display | SDL2 | %dx%d",
           im->width, im->height);

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    TRACE("SDL_Init failed: %s\n", SDL_GetError());
    return E_INTERNAL;
  }

  TIM_SDL_NullCheckERR(window =
                           SDL_CreateWindow(window_title, 100, 100,
                                            bounding_box.w, bounding_box.h, 0));
  TIM_SDL_NullCheckERR(
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
  TIM_SDL_NullCheckERR(surface = SDL_CreateRGBSurfaceFrom(
                           im->pixels, im->width, im->height,
                           im->channels * 8, // 8 bits per channel for RGB(A)
                           im->width * 3, // pitch
                           // ? stbi gives RGB(A) ordered buffer
                           0x0000FF, // R
                           0x00FF00, // G
                           0xFF0000, // B
                           0x000000 // A (discard)
                           ));
  TIM_SDL_NullCheckERR(display_texture =
                           SDL_CreateTextureFromSurface(renderer, surface));

  // render once since image is static and window is not resizable
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, display_texture, NULL, &bounding_box);
  SDL_RenderPresent(renderer);

  while (1) {
    if (SDL_WaitEvent(&e) &&
        (e.type == SDL_QUIT ||
         (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)))
      break;
    // TODO: resizable window with proportion recalculation
  }

  SDL_DestroyTexture(display_texture);
  SDL_FreeSurface(surface);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
#endif
  return E_OK;
}
