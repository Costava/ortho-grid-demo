#include "App.h"

#include "Clock.h"
#include "Sdlu.h"

void App_Init(App *const app) {
    Sdlu_Init(SDL_INIT_VIDEO);

    app->window = Sdlu_CreateWindow(
        "Application",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_RESIZABLE);

    app->renderer = Sdlu_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    app->quit = false;
}

static void Render(App *const app) {
    Sdlu_RenderFillRect(app->renderer, NULL);
    SDL_RenderPresent(app->renderer);
}

static void PollEvents(App *const app) {
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
            case SDL_QUIT:
            {
                app->quit = true;
                break;
            }
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        // world.surface = sdlu_get_window_surface(world.window);
                        //
                        // sdlu_fill_surface(world.surface, 27, 60, 20);
                        // sdlu_update_window_surface(world.window);
                        break;
                    }
                }

                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                break;
            }
            case SDL_MOUSEMOTION:
            {
                break;
            }
            case SDL_KEYUP:
            {
                const SDL_Keycode keycode = event.key.keysym.sym;
                const SDL_Keymod modState = SDL_GetModState();

                switch (keycode) {
                    case SDLK_F11:
                    {
                        // Ignore F11 press if any of Alt, Shift, Ctrl, etc. are down
                        if (modState == KMOD_NONE) {
                            Sdlu_ToggleFullscreenFlag(app->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }

                        break;
                    }
                }

                break;
            }
        }
    }
}

void App_Run(App *const app) {
    while (!app->quit) {
        // const uint64_t new_time = Clock_GetTimeNs();
        // const uint64_t delta = new_time - old_time;
        // const double ddelta = (double)delta;
        // // printf("old_time: %ld new_time: %ld\n", old_time, new_time);
        // old_time = new_time;

        PollEvents(app);

        Render(app);
    }
}

void App_Deinit(App *const app) {
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    SDL_Quit();
}
