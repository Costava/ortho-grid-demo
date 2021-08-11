#ifndef APP_H
#define APP_H

#include <stdbool.h>

#include "SDL2/SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;

    bool quit;
} App;

// Initialize `app`.
void App_Init(App *const app);

// Run the app until user quits.
void App_Run(App *const app);

// Deinitialize `app`.
void App_Deinit(App *const app);

#ifdef __cplusplus
}
#endif

#endif
