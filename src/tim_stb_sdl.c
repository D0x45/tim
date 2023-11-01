// C99
#include <stddef.h> // NULL
#include <stdio.h>  // fputs, stderr, printf

#include "tim.h" // Tiny Image Manipulation

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

// allow implementing tim_display() with -lSDL2 -lSDL2_image -DTIM_IMPL_DISPLAY
#ifdef TIM_IMPL_DISPLAY
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_image.h>
    #define TIM_WINDOW_W 1024
    #define TIM_WINDOW_H 720
    #define TIM_MIN(a, b) (a) < (b) ? (a) : (b)
    #define TIM_STRINGIFY_(x) #x
    #define TIM_STRINGIFY(x) TIM_STRINGIFY_(x)
    #define TIM_SDL_NullCheckERR(buffer)                                             \
        (void)0;                                                                     \
        if ((buffer) == NULL) {                                                      \
            fprintf(stderr, TIM_STRINGIFY(buffer) " is NULL, %s\n", SDL_GetError()); \
            return;                                                                  \
        }
#endif

#define RED 0
#define GREEN 1
#define BLUE 2
// deref a 8-bit color ptr either for assigning or reading its value
#define PX(im, x, y, c) *(im->pixels + ((x + im->width * y) * im->channels) + c)

// debugging enabled
#ifdef DEBUG
    #include <time.h> // time
    #define TRACE(...) printf(__VA_ARGS__);
    #define PX_DBG(image, x, y) printf(                                  \
        "Pixel (im: %p, x: %d, y: %d) { r: %u, g: %u, b: %u, a: %u }\n", \
        image->pixels, x, y,                                             \
        PX(image, x, y, RED),                                            \
        PX(image, x, y, GREEN),                                          \
        PX(image, x, y, BLUE),                                           \
        image->channels == 4 ? PX(image, x, y, 3) : 0xff);
#else
    #define TRACE(...)
    #define PX_DBG(a, b, c)
#endif

// new black canvas
Image tim_new(size_t width, size_t height)
{
    uint8_t *buffer = calloc(width * height * 3, sizeof(uint8_t));
    if (buffer == NULL)
        fprintf(stderr, "tim_new(w=%ld, h=%ld): calloc(%ld) returned NULL\n", width, height, width * height * 3);
    TRACE("tim_new(%ld, %ld) => Image { pixels: %p }\n", width, height, buffer);
    return (Image){
        .height = height,
        .width = width,
        .channels = 3,
        .pixels = buffer
    };
}

// read image file
Image tim_read(const char *const file)
{
    Image dst_img = {
        .height = 0,
        .width = 0,
        .channels = 0,
        .pixels = NULL,
    };

    if (file == NULL)
        return dst_img;

#ifdef DEBUG
    time_t t_start = time(NULL);
#endif

    dst_img.pixels = stbi_load(file, &dst_img.width, &dst_img.height, &dst_img.channels, 3);

    if (dst_img.pixels == NULL)
        fprintf(stderr, "stbi_err: %s\n", stbi_failure_reason());

    TRACE(
        "tim_read(%s) => Image { width: %d, height: %d, channels: %d, pixels: %p }; T = %lds\n",
        file, dst_img.width, dst_img.height, dst_img.channels, dst_img.pixels,
        time(NULL) - t_start
    );

    // debug display a subsequent column of pixels
    PX_DBG((&dst_img), 0, 1);
    PX_DBG((&dst_img), 0, 2);
    PX_DBG((&dst_img), 0, 3);
    PX_DBG((&dst_img), 0, 4);
    PX_DBG((&dst_img), 0, 5);

    return dst_img;
}

// this only works with 8bpc RGB / RGBA
Pixel tim_pixel(Image *image, size_t x, size_t y)
{
    TRACE("tim_pixel(%p, %ld, %ld)\n", image == NULL ? NULL : image->pixels, x, y);

    if (image == NULL || x > image->width || y > image->height)
        return (Pixel){0};
    // uint32_t packed = (alpha<<24) + (red<<16) + (green<<8) + blue;
    return (Pixel){
        .alpha = 0xff, // ignore alpha
        .red = PX(image, x, y, RED),
        .green = PX(image, x, y, GREEN),
        .blue = PX(image, x, y, BLUE)
    };
}

int tim_free(Image *image)
{
    TRACE("tim_free(%p)\n", image == NULL ? NULL : image->pixels);
    if (image == NULL || image->pixels == NULL)
        return -1;
    stbi_image_free(image->pixels);
    image->height = 0;
    image->width = 0;
    image->channels = 0;
    image->pixels = NULL;
    return 0;
}

// nearest neighbor
Image tim_resize(Image *src, size_t new_width, size_t new_height)
{
    TRACE("tim_resize(%p, %ld, %ld)\n", src == NULL ? NULL : src->pixels, new_width, new_height);

    // zero means no scaling happens at that dimension
    if (new_height == 0) new_height = src->height;
    if (new_width == 0) new_width = src->width;

    // return the original image unchanged
    if (src->height == new_height && src->width == new_width)
        return *src;

    // calculate ratio
    float r_h = (float)new_height / (float)src->height,
          r_w = (float)new_width / (float)src->width;
    TRACE("r_h=%f, r_w=%f\n", r_h, r_w);

    // new empty canvas
    Image dst = tim_new(new_width, new_height);

#ifdef DEBUG
    time_t t_start = time(NULL);
#endif

    // downscaling skips every r_w|r_h pixels
    // upscaling duplicates every r_w|r_h pixels
    // iterating row-first (->)
    for (size_t dst_y = 0; dst_y < new_height; ++dst_y) {
        for (size_t dst_x = 0; dst_x < new_width; ++dst_x) {
            // translate destination (x,y) to source (x,y)
            const size_t src_x = (size_t)((float)dst_x / r_w),
                         src_y = (size_t)((float)dst_y / r_h);
            // copy three channels:
            PX((&dst), dst_x, dst_y, RED)   = PX(src, src_x, src_y, RED);
            PX((&dst), dst_x, dst_y, GREEN) = PX(src, src_x, src_y, GREEN);
            PX((&dst), dst_x, dst_y, BLUE)  = PX(src, src_x, src_y, BLUE);
        }
    }

    TRACE("resize took: %ld seconds\n", time(NULL)-t_start);

    return dst;
}

int tim_write(Image *image, const char *file)
{
    if (image == NULL || image->pixels == NULL || file == NULL) {
        fprintf(stderr, "tim_write(%p, %s) called with NULL args\n", image == NULL ? NULL : image->pixels, file);
        return -1;
    }
#ifdef DEBUG
    time_t t_start = time(NULL);
#endif
    TRACE("tim_write(%p, %s)\n", image->pixels, file);
    // ignore input file format and save as jpg 100
    int result = stbi_write_jpg(
        file, image->width, image->height, 3,
        image->pixels, 100
    );
    TRACE("tim_write took %ld seconds\n", time(NULL)-t_start);
    return result;
}

void tim_display(Image *image)
{
    TRACE("tim_display(%p)\n", image == NULL ? NULL : image->pixels);
#ifdef TIM_IMPL_DISPLAY
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Surface *surface = NULL;
    SDL_Texture *display_texture = NULL;
    SDL_Event e;
    char window_title[40] = {0};

    if (image == NULL)
        return;

    // calculate proportions to fit the image into WINDOW_H*WINDOW_W without distorting it
    const float ratio_w = (float)TIM_WINDOW_W / (float)image->width;
    const float ratio_h = (float)TIM_WINDOW_H / (float)image->height;
    const float proportion = TIM_MIN(ratio_h, ratio_w);
    const SDL_Rect bounding_box = {
        .x = 0, .y = 0,
        .w = (int)(image->width * proportion),
        .h = (int)(image->height * proportion)
    };
    snprintf(window_title, sizeof(window_title), "TIM Display | SDL2 | %dx%d", image->width, image->height);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return;
    }

    TIM_SDL_NullCheckERR(window = SDL_CreateWindow(window_title, 100, 100, bounding_box.w, bounding_box.h, 0));
    TIM_SDL_NullCheckERR(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));
    TIM_SDL_NullCheckERR(surface = SDL_CreateRGBSurfaceFrom(
            image->pixels,
            image->width,
            image->height,
            24, // 8 bits per channel for RGB
            image->width * 3, // pitch
            // ? stbi gives RGB(A) ordered buffer
            0x0000FF, // R
            0x00FF00, // G
            0xFF0000, // B
            0x000000  // A (discard)
        )
    );
    TIM_SDL_NullCheckERR(display_texture = SDL_CreateTextureFromSurface(renderer, surface));

    // render once since image is static and window is not resizable
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, display_texture, NULL, &bounding_box);
    SDL_RenderPresent(renderer);

    while (1)
    {
        if (
            SDL_WaitEvent(&e) &&
            (e.type == SDL_QUIT || (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)))
            break;
        // TODO: resizable window with proportion recalculation
    }

    SDL_DestroyTexture(display_texture);
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
#endif
}
