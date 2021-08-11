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

    app->cameraPos = (V3f) {0.0, 200.0, 0.0};
    app->horizLookRads = 3.0 * M_PI / 2.0; // Looking in negative y direction.
    app->vertLookRads = M_PI / 2.0; // Looking parallel to xy plane.

    app->projPlaneWidth = 800.0;
    app->projPlaneHeight = 600.0;

    Sdlu_SetRelativeMouseMode(SDL_TRUE);
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
                const double mouseSens = 0.3e-7;

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

// Orthographically project the given point onto the plane described
// by the given vectors.
// Use the app's camera position.
// Assume the look vectors are unit magnitude.
static V3f OrthoProject(App *app, V3f point, V3f lookRight, V3f lookUp) {
    V3f relativePoint = V3f_Sub(point, app->cameraPos);

    return (V3f) {
        V3f_Dot(relativePoint, lookRight),
        V3f_Dot(relativePoint, lookUp),
        0.0
    };
}

// Return the point projected onto the screen
// as a vector where [0.0, 0.0] means top left pixel of screen
// and [1.0, 1.0] means bottom right pixel of screen.
// Use the app's camera position and projection plane dimensions.
static V3f PointToScreenProportions(App *app, V3f point, V3f lookRight, V3f lookUp) {
    V3f projected = OrthoProject(app, point, lookRight, lookUp);

    return (V3f) {
        (projected.x + (app->projPlaneWidth / 2.0)) / app->projPlaneWidth,
        (projected.y + (app->projPlaneHeight / 2.0)) / app->projPlaneHeight,
        0.0
    };
}

// Vector with 2 int fields.
typedef struct V2i {
    int x;
    int y;
} V2i;

typedef struct MaybeV2i {
    bool hasValue;
    V2i value;
} MaybeV2i;

static MaybeV2i PointToPixel(App *app, V3f point, V3f lookForward, V3f lookRight, V3f lookUp) {
    // Cannot see point if behind projection plane.

    V3f camToPoint = V3f_Sub(point, app->cameraPos);
    V3f unitCamToPoint = V3f_Unit(camToPoint);
    if (V3f_Dot(lookForward, unitCamToPoint) <= 0.0) {
        return (MaybeV2i) { false };
    }

    V3f screenProps = PointToScreenProportions(app, point, lookRight, lookUp);

    if (screenProps.x < 0.0 || screenProps.y < 0.0 || screenProps.x > 1.0 || screenProps.y > 1.0) {
        return (MaybeV2i) { false };
    }

    int screenX;
    int screenY;

    Sdlu_GetRendererOutputSize(app->renderer, &screenX, &screenY);

    return (MaybeV2i) {
        true,
        (V2i) {
            (int)round(screenProps.x * (screenX - 1.0)),
            (int)round(screenProps.y * (screenY - 1.0))
        }
    };
}

static void MaybeDrawPoint(App *app, V3f point, V3f look, V3f lookRight, V3f lookUp) {
    MaybeV2i maybePixel = PointToPixel(app, point, look, lookRight, lookUp);

    if (maybePixel.hasValue) {
        fprintf(stdout, "maybePixel: (%d, %d)\n",
            maybePixel.value.x, maybePixel.value.y);

        Sdlu_RenderDrawPoint(
            app->renderer, maybePixel.value.x, maybePixel.value.y);
    }
}

// Converting spherical coordinates to a vector.
// radius = 1.0 so not shown and no need to normalize the vector.
static inline V3f SphericalToCartesian(const double horizLookRads, const double vertLookRads) {
    return (V3f) {
        sin(vertLookRads) * cos(horizLookRads),
        sin(vertLookRads) * sin(horizLookRads),
        cos(vertLookRads)
    };
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

        // const double fps = 1000000000.0 / (double)deltaNs;
        // printf("fps: %lf\n", fps);
        // printf("oldTimeNs: %ld newTimeNs: %ld\n", oldTimeNs, newTimeNs);

        PollEvents(app, ddeltaNs);

        // Direction camera is looking.
        V3f look = SphericalToCartesian(app->horizLookRads, app->vertLookRads);

        // Calculate the up vector.
        const double lookUpRads = app->vertLookRads - (M_PI / 2.0);
        V3f lookUp = SphericalToCartesian(app->horizLookRads, lookUpRads);

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

        // Assume camera never has roll (only pitch and yaw).
        V3f lookRight = xyRight;

        const uint8_t *const kbState = SDL_GetKeyboardState(NULL);

        // Movement speed.
        const double moveFactor = 0.0000001;

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

        // Render

        Sdlu_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
        Sdlu_RenderFillRect(app->renderer, NULL);

        Sdlu_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
        MaybeDrawPoint(app, (V3f) {0.0, 0.0, 0.0}, look, lookRight, lookUp);

        Sdlu_SetRenderDrawColor(app->renderer, 255, 0, 0, 255);
        MaybeDrawPoint(app, (V3f) {100.0, 0.0,  0.0}, look, lookRight, lookUp);

        Sdlu_SetRenderDrawColor(app->renderer, 0, 255, 0, 255);
        MaybeDrawPoint(app, (V3f) {100.0, 100.0, 0.0}, look, lookRight, lookUp);

        Sdlu_SetRenderDrawColor(app->renderer, 0, 0, 255, 255);
        MaybeDrawPoint(app, (V3f) {0.0, 100.0, 0.0}, look, lookRight, lookUp);

        SDL_RenderPresent(app->renderer);

        oldTimeNs = newTimeNs;
    }
}

void App_Deinit(App *const app) {
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    SDL_Quit();
}
