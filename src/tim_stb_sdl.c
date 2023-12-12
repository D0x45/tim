// C99
#include <stddef.h> // NULL
#include <stdio.h>  // stderr, fprintf, snprintf
#include <stdlib.h> // calloc, free
#include <time.h> // time

#include "tim.h" // Tiny Image Manipulation

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define TIM_MIN(a, b) ((a) < (b) ? (a) : (b))
#define TIM_STRINGIFY_(x) #x
#define TIM_STRINGIFY(x) TIM_STRINGIFY_(x)

// color channel offsets for RGBA
// TODO: do i need to check for stbi order or target host being little-endian?
#define TIM_RGBA_C0 0 // red
#define TIM_RGBA_C1 1 // green
#define TIM_RGBA_C2 2 // blue
#define TIM_RGBA_C3 3 // alpha

// deref a color ptr either for assigning or reading its value
#define TIM_PX(im, x, y, c)                                                    \
  *(im->pixels + ((x + im->width * y) * im->channels) + c)

// debugging enabled
#if defined(DEBUG) || !defined(NDEBUG)
  #define TIM_DEBUG 1
#else
  #define TIM_DEBUG 0
#endif

#if TIM_DEBUG
  #define TIM_TRACE(...) fprintf(stderr, "[TIM] " __VA_ARGS__);
#else
  #define TIM_TRACE(...)
#endif

// allow implementing tim_display() with -lSDL2 -lSDL2_image -DTIM_IMPL_DISPLAY
#ifdef TIM_IMPL_DISPLAY
  #include <SDL2/SDL.h>
  #include <SDL2/SDL_image.h>
  #include <SDL2/SDL_rect.h>
  #define TIM_WINDOW_W 1024
  #define TIM_WINDOW_H 720
  #define TIM_SDL_NullCheckERR(buffer)                                           \
    if ((buffer) == NULL) {                                                      \
      fprintf(stderr, TIM_STRINGIFY(buffer) " is NULL, %s\n", SDL_GetError());   \
      return TIM_ERR_INTERNAL;                                                   \
    }
#endif

tim_err tim_init(tim_img *im, size_t width, size_t height, size_t channels) {
  TIM_TRACE("tim_init(%p, %ld, %ld, %ld) => { pixels: %p }\n", im, width,
            height, channels, (im == NULL) ? NULL : im->pixels);
  if (im == NULL || channels < 1 || channels > 4)
    return TIM_ERR_ARG;
  im->width = width;
  im->height = height;
  im->channels = channels;
  im->pixels = calloc(width * height * channels, sizeof(uint8_t));
  TIM_TRACE("allocated addr %p with %ld bytes\n", im->pixels,
            width * height * channels);
  return (im->pixels == NULL) ? TIM_ERR_ALLOC : TIM_ERR_OK;
}

tim_err tim_file_read(tim_img *im, const char *file) {
#ifdef TIM_DEBUG
  time_t t_start = time(NULL);
#endif
  TIM_TRACE("tim_file_read(%p, %p)\n", im, file);

  if (im == NULL || file == NULL)
    return TIM_ERR_ARG;

  im->pixels =
      stbi_load(file, &im->width, &im->height, &im->channels, STBI_default);

  if (im->pixels == NULL) {
    TIM_TRACE("stbi_err: %s\n", stbi_failure_reason());
    return TIM_ERR_INTERNAL;
  }

  TIM_TRACE(
      "tim_file_read(%p, %s) => { w: %d, h: %d, ch: %d, px: %p } in %lds\n", im,
      file, im->width, im->height, im->channels, im->pixels,
      time(NULL) - t_start);

  return TIM_ERR_OK;
}

tim_err tim_file_write(tim_img *im, const char *file) {
  int stbi_result;
#ifdef TIM_DEBUG
  time_t t_start = time(NULL);
#endif

  TIM_TRACE("tim_file_write(%p, %p)\n", im, file);

  if (im == NULL || im->pixels == NULL || file == NULL)
    return TIM_ERR_ARG;

  // ignore input file format and save as jpg 100
  stbi_result = stbi_write_jpg(file, im->width, im->height, im->channels,
                               im->pixels, 100);
  if (stbi_result <= 0) {
    TIM_TRACE("stbi_err: %s\n", stbi_failure_reason());
    return TIM_ERR_INTERNAL;
  }

  TIM_TRACE("tim_file_write(%p, %s) took %ld seconds, stbi_result = %d\n", im,
            file, time(NULL) - t_start, stbi_result);

  return TIM_ERR_OK;
}

tim_err tim_pixel_get(tim_img *im, size_t x, size_t y, tim_pixel *dst) {
  TIM_TRACE("tim_pixel_get(%p, %ld, %ld, %p)\n", im, x, y, dst);

  if (im == NULL || x > im->width || y > im->height || dst == NULL)
    return TIM_ERR_ARG;

  dst->red = (im->channels >= 1) ? TIM_PX(im, x, y, TIM_RGBA_C0) : 0x00;
  dst->green = (im->channels >= 2) ? TIM_PX(im, x, y, TIM_RGBA_C1) : 0x00;
  dst->blue = (im->channels >= 3) ? TIM_PX(im, x, y, TIM_RGBA_C2) : 0x00;
  dst->alpha = (im->channels >= 4) ? TIM_PX(im, x, y, TIM_RGBA_C3) : 0x00;

  return TIM_ERR_OK;
}

tim_err tim_pixel_set(tim_img *im, size_t x, size_t y, tim_pixel *src) {
  TIM_TRACE("tim_pixel_set(%p, %ld, %ld, %p)\n", im, x, y, src);

  if (im == NULL || x > im->width || y > im->height || src == NULL)
    return TIM_ERR_ARG;

  TIM_PX(im, x, y, TIM_RGBA_C0) = src->red;

  if (im->channels >= 2)
    TIM_PX(im, x, y, TIM_RGBA_C1) = src->green;

  if (im->channels >= 3)
    TIM_PX(im, x, y, TIM_RGBA_C2) = src->blue;

  if (im->channels >= 4)
    TIM_PX(im, x, y, TIM_RGBA_C3) = src->alpha;

  return TIM_ERR_OK;
}

tim_err tim_free(tim_img *im) {
  TIM_TRACE("tim_free(%p)\n", im);
  if (im == NULL || im->pixels == NULL)
    return TIM_ERR_ARG;
  free(im->pixels);
  im->height = 0;
  im->width = 0;
  im->channels = 0;
  im->pixels = NULL;
  return TIM_ERR_OK;
}

// relative luminance calculated from linear RGB components
static tim_err tim_grayscale(tim_img *im, tim_img *dst) {
  tim_err res;
  size_t x, y;
  float r, g, b;
  uint8_t gr;
  TIM_TRACE("tim_grayscale(%p, %p)\n", im, dst);

  if (im == NULL || dst == NULL || im->channels < 3)
    return TIM_ERR_ARG;

  // since sdl does not support displaying 1ch image (or at least i could not
  // figure it out) and also apparently stbi_write_jpg writes 3 channels anyway
  // (or perhaps stbi_load reads 3 channels regardless) i am gonna do the same
  // trick here to not break the code. as this is a linear grayscale image, all
  // three channels will hold the same value
  res = tim_init(dst, im->width, im->height, 3);
  if (res != TIM_ERR_OK)
    return res;

  for (x = 0; x < im->width; ++x) {
    for (y = 0; y < im->height; ++y) {
      r = 0.2126f * (float)TIM_PX(im, x, y, TIM_RGBA_C0);
      g = 0.7152f * (float)TIM_PX(im, x, y, TIM_RGBA_C1);
      b = 0.0722f * (float)TIM_PX(im, x, y, TIM_RGBA_C2);
      gr = r + g + b; // multiply by alpha so opaque values are darker?
      TIM_PX(dst, x, y, TIM_RGBA_C0) = gr;
      TIM_PX(dst, x, y, TIM_RGBA_C1) = gr;
      TIM_PX(dst, x, y, TIM_RGBA_C2) = gr;
    }
  }

  return TIM_ERR_OK;
}

tim_err tim_apply(tim_img *im, tim_img *dst, tim_filter f) {
  switch (f) {
  case TIM_FILTER_GRAYSCALE:
    return tim_grayscale(im, dst);
  }
  return TIM_ERR_ARG;
}

// nearest neighbor
tim_err tim_resize(tim_img *im, tim_img *dst, size_t new_width,
                   size_t new_height) {
  tim_err init_res;
  float r_h, r_w;
  size_t dst_y, dst_x, src_x, src_y;
#ifdef TIM_DEBUG
  time_t t_start = time(NULL);
#endif

  TIM_TRACE("tim_resize(%p, %p, %ld, %ld)\n", im, dst, new_width, new_height);

  if (im == NULL || dst == NULL)
    return TIM_ERR_ARG;

  // zero means no scaling happens at that dimension
  new_height = (new_height == 0) ? im->height : new_height;
  new_width = (new_width == 0) ? im->width : new_width;

  // return since nothing changed
  // if (im->height == new_height && im->width == new_width)
  //   return TIM_ERR_ARG;

  // calculate ratio
  r_h = (float)new_height / (float)im->height;
  r_w = (float)new_width / (float)im->width;
  TIM_TRACE("r_h=%f, r_w=%f\n", r_h, r_w);

  // new empty canvas
  init_res = tim_init(dst, new_width, new_height, im->channels);
  if (init_res != TIM_ERR_OK)
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
      TIM_PX(dst, dst_x, dst_y, TIM_RGBA_C0) =
          TIM_PX(im, src_x, src_y, TIM_RGBA_C0);

      if (im->channels >= 2)
        TIM_PX(dst, dst_x, dst_y, TIM_RGBA_C1) =
            TIM_PX(im, src_x, src_y, TIM_RGBA_C1);
      if (im->channels >= 3)
        TIM_PX(dst, dst_x, dst_y, TIM_RGBA_C2) =
            TIM_PX(im, src_x, src_y, TIM_RGBA_C2);
      if (im->channels >= 4)
        TIM_PX(dst, dst_x, dst_y, TIM_RGBA_C3) =
            TIM_PX(im, src_x, src_y, TIM_RGBA_C3);
    }
  }

  TIM_TRACE("resize took: %ld seconds\n", time(NULL) - t_start);

  return TIM_ERR_OK;
}

tim_err tim_display(tim_img *im) {
  TIM_TRACE("tim_display(%p)\n", im);
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
    return TIM_ERR_ARG;

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
    TIM_TRACE("SDL_Init failed: %s\n", SDL_GetError());
    return TIM_ERR_INTERNAL;
  }

  TIM_SDL_NullCheckERR(window =
                           SDL_CreateWindow(window_title, 100, 100,
                                            bounding_box.w, bounding_box.h, 0));
  TIM_SDL_NullCheckERR(
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
  TIM_SDL_NullCheckERR(surface = SDL_CreateRGBSurfaceFrom(
                           im->pixels, im->width, im->height,
                           im->channels * 8, // 8 bits per channel for RGB(A)
                           im->width * 3,    // pitch
                           // ? stbi gives RGB(A) ordered buffer
                           0x0000FF, // R
                           0x00FF00, // G
                           0xFF0000, // B
                           0x000000  // A (discard)
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
  return TIM_ERR_OK;
}
