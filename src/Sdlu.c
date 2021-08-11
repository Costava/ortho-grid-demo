#include "Sdlu.h"

#include <stdio.h>
#include <stdlib.h>

void Sdlu_Init(uint32_t flags) {
    const int code = SDL_Init(flags);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_Init returned %d instead of 0. "
            "[flags: %d] [Error: %s]\n",
            __func__, code,
            flags, SDL_GetError());

        exit(1);
    }
}

SDL_Window *Sdlu_CreateWindow(
    const char *title,
    int x, int y,
    int w, int h,
    uint32_t flags)
{
    SDL_Window *const window = SDL_CreateWindow(title, x, y, w, h, flags);

    if (window == NULL) {
        fprintf(stderr, "%s: SDL_CreateWindow returned NULL. "
            "[title: %s] [x: %d] [y: %d] [w: %d] [h: %d] "
            "[flags: %d] [Error: %s]\n",
            __func__,
            title, x, y, w, h,
            flags, SDL_GetError());

        exit(1);
    }

    return window;
}

void Sdlu_CreateWindowAndRenderer(
    int width,
    int height,
    Uint32 window_flags,
    SDL_Window **window,
    SDL_Renderer **renderer)
{
    const int code = SDL_CreateWindowAndRenderer(width, height, window_flags, window, renderer);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_CreateWindowAndRenderer returned %d instead of 0. "
            "[width: %d] [height: %d] [window_flags: %d] [Error: %s]\n",
            __func__, code,
            width, height, window_flags, SDL_GetError());

        exit(1);
    }
}

SDL_Renderer *Sdlu_CreateRenderer(
    SDL_Window *window,
    int index,
    Uint32 flags)
{
    SDL_Renderer *const renderer = SDL_CreateRenderer(window, index, flags);

    if (renderer == NULL) {
        fprintf(stderr, "%s: SDL_CreateRenderer returned NULL. "
            "[index: %d] [flags: %d] [Error: %s]\n",
            __func__,
            index, flags, SDL_GetError());

        exit(1);
    }

    return renderer;
}

SDL_Renderer *Sdlu_GetRenderer(SDL_Window *window) {
    SDL_Renderer *const renderer = SDL_GetRenderer(window);

    if (renderer == NULL) {
        fprintf(stderr, "%s: SDL_GetRenderer failed: %s\n", __func__, SDL_GetError());
        exit(1);
    }

    return renderer;
}

void Sdlu_GetRendererOutputSize(SDL_Renderer *renderer, int *w, int *h) {
    const int code = SDL_GetRendererOutputSize(renderer, w, h);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_GetRendererOutputSize failed: %s\n", __func__, SDL_GetError());
        exit(1);
    }
}

void Sdlu_RenderFillRect(SDL_Renderer *renderer, const SDL_Rect *rect) {
    const int code = SDL_RenderFillRect(renderer, rect);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_RenderFillRect returned %d instead of 0. [Error: %s]\n",
            __func__, code, SDL_GetError());

        exit(1);
    }
}

int Sdlu_GetWindowDisplayIndex(SDL_Window *window) {
    const int index = SDL_GetWindowDisplayIndex(window);

    if (index < 0) {
        fprintf(stderr, "%s: SDL_GetWindowDisplayIndex returned negative error code: %d. "
            "[Error: %s]\n",
            __func__, index,
            SDL_GetError());

        exit(1);
    }

    return index;
}

void Sdlu_GetDesktopDisplayMode(int displayIndex, SDL_DisplayMode *mode) {
    const int code = SDL_GetDesktopDisplayMode(displayIndex, mode);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_GetDesktopDisplayMode returned %d instead of 0. "
            "[displayIndex: %d] [Error: %s]\n",
            __func__, code,
            displayIndex, SDL_GetError());

        exit(1);
    }
}

void Sdlu_SetWindowFullscreen(SDL_Window *w, uint32_t flags) {
    const int code = SDL_SetWindowFullscreen(w, flags);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_SetWindowFullscreen returned %d instead of 0. "
            "[flags: %d] [Error: %s]\n",
            __func__, code,
            flags, SDL_GetError());

        exit(1);
    }
}

void Sdlu_ToggleFullscreenFlag(SDL_Window *w, uint32_t fullscreenFlag) {
    // Different ways of being fullscreen:
    // 1. SDL_WINDOW_FULLSCREEN flag.
    // 2. SDL_WINDOW_FULLSCREEN_DESKTOP flag.
    // 3. Window manager keyboard shortcut (or other mechanism) sets drawing size of window
    //     to size of screen e.g. Alt+F11 with XFCE which resizes the window to fullscreen
    //     without setting a fullscreen flag in the eyes of SDL2.

    const uint32_t windowFlags = SDL_GetWindowFlags(w);
    const bool hasAnyFullscreenFlag =
        windowFlags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP);

    const uint32_t flag = (hasAnyFullscreenFlag) ? 0 : fullscreenFlag;

    Sdlu_SetWindowFullscreen(w, flag);

    // This commented section is no good because the behavior I was seeing is:
    //  With XFCE and pressing Alt+F11, XFCE does not set a fullscreen flag in SDL2's eyes,
    //  XFCE just resizes the window.
    //  Thus, with XFCE, SDL_SetWindowFullscreen(w, 0) does not un-fullscreen the window
    //  that was made fullscreen using the Alt+F11 keyboard shortcut
    //  because there is no flag to remove.
    //
    // const int windowDisplayIndex = Sdlu_GetWindowDisplayIndex(w);
    //
    // SDL_DisplayMode displayMode;
    // Sdlu_GetDesktopDisplayMode(windowDisplayIndex, &displayMode);
    //
    // printf("displayMode.w: %d displayMode.h: %d\n", displayMode.w, displayMode.h);
    //
    // SDL_Renderer *const renderer = Sdlu_GetRenderer(w);
    //
    // int rendererWidth;
    // int rendererHeight;
    // Sdlu_GetRendererOutputSize(renderer, &rendererWidth, &rendererHeight);
    //
    // printf("rendererWidth: %d rendererHeight: %d\n", rendererWidth, rendererHeight);
    //
    // const bool windowFillsScreen =
    //     (displayMode.w == rendererWidth) && (displayMode.h == rendererHeight);

    // const uint32_t flag = (hasAnyFullscreenFlag || windowFillsScreen) ? 0 : fullscreenFlag;
}

void Sdlu_SetRenderDrawColor(SDL_Renderer *renderer,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    const int code = SDL_SetRenderDrawColor(renderer, r, g, b, a);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_SetRenderDrawColor returned %d instead of 0. "
            "[r: %d] [g: %d] [b: %d] [a: %d] [Error: %s]\n",
            __func__, code, r, g, b, a, SDL_GetError());

        exit(1);
    }
}

void Sdlu_RenderDrawPoint(SDL_Renderer *renderer, int x, int y) {
    const int code = SDL_RenderDrawPoint(renderer, x, y);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_RenderDrawPoint returned %d instead of 0. "
            "[x: %d] [y: %d] [Error: %s]\n",
            __func__, code, x, y, SDL_GetError());

        exit(1);
    }
}

void Sdlu_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2) {
    const int code = SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_RenderDrawLine returned %d instead of 0. "
            "[x1: %d] [y1: %d] [x2: %d] [y2: %d] [Error: %s]\n",
            __func__, code, x1, y1, x2, y2, SDL_GetError());

        exit(1);
    }
}

void Sdlu_SetRelativeMouseMode(SDL_bool enabled) {
    const int code = SDL_SetRelativeMouseMode(enabled);

    if (code != 0) {
        fprintf(stderr, "%s: SDL_SetRelativeMouseMode(%d) returned %d instead of 0. Error: %s\n",
            __func__, enabled, code, SDL_GetError());

        exit(1);
    }
}
