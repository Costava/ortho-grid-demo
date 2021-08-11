#include "App.h"

#include "Clock.h"
#include "M_PI.h"
#include "Sdlu.h"
#include "V3f.h"


void App_Init(App *const app) {
    Sdlu_Init(SDL_INIT_VIDEO);

    app->window = Sdlu_CreateWindow(
        "Application",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_RESIZABLE);

    app->renderer = Sdlu_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    app->quit = false;

    app->cameraPos = (V3f) {0.0, 0.0, 0.0};
    app->horizLookRads = 3.0 * M_PI / 2.0; // Looking in negative y direction.
    app->vertLookRads = M_PI / 2.0; // Looking parallel to xy plane.
}

static void Render(App *const app) {
    Sdlu_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    Sdlu_RenderFillRect(app->renderer, NULL);
    SDL_RenderPresent(app->renderer);
}

static void PollEvents(App *const app, const double ddeltaNs) {
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
                        // app->surface = sdlu_get_window_surface(app->window);
                        //
                        // sdlu_fill_surface(app->surface, 27, 60, 20);
                        // sdlu_update_window_surface(app->window);
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
                const double mouseSens = 0.5e-6;

                app->horizLookRads +=
                    mouseSens * event.motion.xrel * ddeltaNs;
                app->vertLookRads -=
                    mouseSens * event.motion.yrel * ddeltaNs;

                // Normalize horizLookRads to range (-2pi, 2pi)
                app->horizLookRads = fmod(app->horizLookRads, 2.0 * M_PI);
                // Normalize horizLookRads to range [-pi, pi]
                if (app->horizLookRads < -M_PI)
                    { app->horizLookRads += 2.0 * M_PI; }
                else if (app->horizLookRads > M_PI)
                    { app->horizLookRads -= 2.0 * M_PI; }

                // Clamp vertLookRads
                const double minVertLookRads = 0.00001;
                const double maxVertLookRads = M_PI - minVertLookRads;
                if (app->vertLookRads < minVertLookRads) {
                    app->vertLookRads = minVertLookRads;
                }
                else if (app->vertLookRads > maxVertLookRads) {
                    app->vertLookRads = maxVertLookRads;
                }

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
    uint64_t oldTimeNs = Clock_GetTimeNs();

    while (!app->quit) {
        const uint64_t newTimeNs = Clock_GetTimeNs();
        const uint64_t deltaNs = newTimeNs - oldTimeNs;
        const double ddeltaNs = (double)deltaNs;

        if (deltaNs == 0) {
            continue;
        }

        const double fps = 1000000000.0 / (double)deltaNs;
        // printf("fps: %lf\n", fps);
        // printf("oldTimeNs: %ld newTimeNs: %ld\n", oldTimeNs, newTimeNs);

        PollEvents(app, ddeltaNs);

        // Converting spherical coordinates to a vector.
        // radius = 1.0 so not shown and no need to normalize the vector.
        // Direction camera is looking.
        V3f look = (V3f) {
            sin(app->vertLookRads) * cos(app->horizLookRads),
            sin(app->vertLookRads) * sin(app->horizLookRads),
            cos(app->vertLookRads)
        };

        // Calculate the up vector.
        const double lookUpRads = app->vertLookRads + (M_PI / 2.0);
        V3f lookUp = (V3f) {
            sin(lookUpRads) * cos(app->horizLookRads),
            sin(lookUpRads) * sin(app->horizLookRads),
            cos(lookUpRads)
        };

        V3f xyForward = (V3f) {
            cos(app->horizLookRads),
            sin(app->horizLookRads),
            0.0
        };

        const double rightRads = app->horizLookRads + (M_PI / 2.0);
        V3f xyRight = (V3f) {
            cos(rightRads),
            sin(rightRads),
            0.0
        };

        const uint8_t *const kbState = SDL_GetKeyboardState(NULL);

        // Movement speed.
        const double moveFactor = 0.00000001;

        // Movement in xy plane.

        if (kbState[SDL_SCANCODE_W] == 1) {
            app->cameraPos = V3f_Add(app->cameraPos, V3f_Mul(xyForward, moveFactor * ddeltaNs));
        }

        if (kbState[SDL_SCANCODE_S] == 1) {
            app->cameraPos = V3f_Sub(app->cameraPos, V3f_Mul(xyForward, moveFactor * ddeltaNs));
        }

        if (kbState[SDL_SCANCODE_A] == 1) {
            app->cameraPos = V3f_Sub(app->cameraPos, V3f_Mul(xyRight, moveFactor * ddeltaNs));
        }

        if (kbState[SDL_SCANCODE_D] == 1) {
            app->cameraPos = V3f_Add(app->cameraPos, V3f_Mul(xyRight, moveFactor * ddeltaNs));
        }

        // Movement up and down.

        if (kbState[SDL_SCANCODE_SPACE] == 1) {
            app->cameraPos.z -= moveFactor * ddeltaNs;
        }

        if (kbState[SDL_SCANCODE_Q] == 1) {
            app->cameraPos.z += moveFactor * ddeltaNs;
        }

        fprintf(stdout, "Look angles: (%lf, %lf)\n",
            app->horizLookRads, app->vertLookRads);
        printf("app->cameraPos: (%lf, %lf, %lf)\n",
            app->cameraPos.x, app->cameraPos.y, app->cameraPos.z);

        Render(app);

        oldTimeNs = newTimeNs;
    }
}

void App_Deinit(App *const app) {
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    SDL_Quit();
}
