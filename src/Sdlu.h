#ifndef SDLU_H
#define SDLU_H

// SDL2 utility and wrapper functions.

#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize SDL using `flags`.
// If error, print to `stderr` and exit.
void Sdlu_Init(uint32_t flags);

// Create window with given args.
// If error, print to `stderr` and exit.
SDL_Window *Sdlu_CreateWindow(
    const char *title,
    int x, int y,
    int w, int h,
    uint32_t flags);

// Call corresponding SDL function.
// If error, print to `stderr` and exit.
void Sdlu_CreateWindowAndRenderer(
    int width,
    int height,
    Uint32 window_flags,
    SDL_Window **window,
    SDL_Renderer **renderer);

// Call corresponding SDL function.
// If error, print to `stderr` and exit.
SDL_Renderer *Sdlu_CreateRenderer(
    SDL_Window *window,
    int index,
    Uint32 flags);

// Get renderer for the window.
// If error, print to `stderr` and exit.
SDL_Renderer *Sdlu_GetRenderer(SDL_Window *window);

// Call corresponding SDL function.
// If error, print to `stderr` and exit.
void Sdlu_GetRendererOutputSize(SDL_Renderer *renderer, int *w, int *h);

// Call corresponding SDL function.
// If error, print to `stderr` and exit.
void Sdlu_RenderFillRect(SDL_Renderer *renderer, const SDL_Rect *rect);

// Call corresponding SDL function.
// If error, print to `stderr` and exit.
int Sdlu_GetWindowDisplayIndex(SDL_Window *window);

// Call corresponding SDL function.
// If error, print to `stderr` and exit.
void Sdlu_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode *mode);

// Set the fullscreen flags on the given window.
// If error, print to `stderr` and exit.
void Sdlu_SetWindowFullscreen(SDL_Window *w, uint32_t flags);

// Toggle fullscreen flag.
// If the window is currently fullscreen w/ either possible flag, remove fullscreen flag.
// If the window is not currently fullscreen, the given flag is applied.
// If error, print to `stderr` and exit.
void Sdlu_ToggleFullscreenFlag(SDL_Window *w, uint32_t fullscreenFlag);

// SDL_SetRenderDrawColor but, if error, print to `stderr` and exit.
void Sdlu_SetRenderDrawColor(SDL_Renderer *renderer,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// SDL_RenderDrawPoint but, if error, print to `stderr` and exit.
void Sdlu_RenderDrawPoint(SDL_Renderer *renderer, int x, int y);

// SDL_SetRelativeMouseMode but, if error, print to `stderr` and exit.
void Sdlu_SetRelativeMouseMode(SDL_bool enabled);

#ifdef __cplusplus
}
#endif

#endif
