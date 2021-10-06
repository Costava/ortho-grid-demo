#include "App.h"

#include <time.h>

#include "Clock.h"
#include "M_PI.h"
#include "Sdlu.h"
#include "V3d.h"

// Save .bmp image of given renderer to given path.
// Print to stderr if error.
static void SaveBmp(SDL_Renderer *const renderer, const char *const path) {
    int w;
    int h;
    Sdlu_GetRendererOutputSize(renderer, &w, &h);

    // https://wiki.libsdl.org/SDL_CreateRGBSurface
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    const Uint32 rmask = 0xff000000;
    const Uint32 gmask = 0x00ff0000;
    const Uint32 bmask = 0x0000ff00;
    const Uint32 amask = 0x000000ff;
#else
    const Uint32 rmask = 0x000000ff;
    const Uint32 gmask = 0x0000ff00;
    const Uint32 bmask = 0x00ff0000;
    const Uint32 amask = 0xff000000;
#endif

    SDL_Surface *const surface = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);

    if (surface == NULL) {
        fprintf(stderr, "FAILED TO SAVE SCREENSHOT. "
            "SDL_CreateRGBSurface error: %s\n", SDL_GetError());

        return;
    }

    const int rrp_code = SDL_RenderReadPixels(renderer, NULL,
        surface->format->format,
        surface->pixels,
        surface->pitch);

    if (rrp_code != 0) {
        fprintf(stderr, "FAILED TO SAVE SCREENSHOT. "
            "SDL_RenderReadPixels error: %d: %s\n",
            rrp_code, SDL_GetError());

        goto cleanup;
    }

    const int sbmp_code = SDL_SaveBMP(surface, path);

    if (sbmp_code != 0) {
        fprintf(stderr, "FAILED TO SAVE SCREENSHOT. "
            "SDL_SaveBMP error: %d: %s\n",
            sbmp_code, SDL_GetError());

        goto cleanup;
    }

    // printf("Saved screenshot: %s\n", path);

    cleanup:
    SDL_FreeSurface(surface);
}

static void UpdateProjPlaneDimensions(App *const app) {
    app->projPlaneWidth = app->baseProjPlaneWidth * app->projPlaneFactor;
    app->projPlaneHeight = app->baseProjPlaneHeight * app->projPlaneFactor;
}

void App_Init(App *const app) {
    Sdlu_Init(SDL_INIT_VIDEO);

    const int windowWidth = 800;
    const int windowHeight = 600;

    app->window = Sdlu_CreateWindow(
        "Orthographic Grid Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE);

    app->renderer = Sdlu_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED);
    app->quit = false;
    app->recording = false;

    app->cameraPos = (V3d) {500.0, 500.0, -500.0};
    app->horizLookRads = 5.0 * M_PI / 4.0;
    app->vertLookRads = M_PI / 4.0;

    app->baseProjPlaneWidth = windowWidth;
    app->baseProjPlaneHeight = windowHeight;
    app->projPlaneFactor = 1.0;

    UpdateProjPlaneDimensions(app);

    Sdlu_SetRelativeMouseMode(SDL_TRUE);
}

typedef struct RelativeMouseMotion {
    Sint32 xrel;
    Sint32 yrel;
} RelativeMouseMotion;

typedef struct MaybeRelativeMouseMotion {
    bool hasValue;
    RelativeMouseMotion value;
} MaybeRelativeMouseMotion;

static void PollEvents(App *const app, const double ddeltaNs) {
    // If multiple mousemotion events happen during one frame,
    //  we sum the relative motion and act once on the sum.
    // Otherwise we would be using the ddeltaNs multiple times (wrong).
    MaybeRelativeMouseMotion motion = { .hasValue = false };
    uint32_t numMouseMotionEvents = 0;

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
                        int screenX;
                        int screenY;

                        Sdlu_GetRendererOutputSize(app->renderer, &screenX, &screenY);

                        app->baseProjPlaneWidth = screenX;
                        app->baseProjPlaneHeight = screenY;

                        UpdateProjPlaneDimensions(app);

                        break;
                    }
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    {
                        Sdlu_SetRelativeMouseMode(SDL_TRUE);
                        break;
                    }
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    {
                        Sdlu_SetRelativeMouseMode(SDL_FALSE);
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
                if (!motion.hasValue) {
                    motion = (MaybeRelativeMouseMotion) {
                        .hasValue = true,
                        .value = {
                            .xrel = event.motion.xrel,
                            .yrel = event.motion.yrel
                        }
                    };
                }
                else {
                    motion.value.xrel += event.motion.xrel;
                    motion.value.yrel += event.motion.yrel;
                }

                // Assume it never overflows
                // (safe to assume and of low consequence)
                numMouseMotionEvents += 1;

                break;
            }
            case SDL_MOUSEWHEEL:
            {
                app->projPlaneFactor -= 0.1 * event.wheel.y;

                const double minFactor = 0.01;

                if (app->projPlaneFactor < minFactor) {
                    app->projPlaneFactor = minFactor;
                }

                UpdateProjPlaneDimensions(app);

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
                    case SDLK_f:
                    {
                        // Snap to nearest isometric view.

                        // Looking down at 45 degree angle.
                        app->vertLookRads = M_PI / 4.0;

                        // Snap horiz angle to nearest 90 deg
                        // (offset by 45 deg from 0 deg direction)

                        double rads = app->horizLookRads - (M_PI / 4.0);
                        rads /= (M_PI / 2.0);
                        rads = round(rads) * M_PI / 2.0;

                        app->horizLookRads = rads + (M_PI / 4.0);
                        break;
                    }
                    case SDLK_r:
                    {
                        if (!app->recording) {
                            // Reset frame number.
                            app->frameNum = 1;

                            // Get new recordingId.

                            struct timespec ts;
                            if (timespec_get(&ts, TIME_UTC) == 0) {
                                fprintf(stderr, "FAILED TO START RECORDING. "
                                    " timespec_get error\n");
                                break;
                            }

                            app->recordingId = ts.tv_sec;

                            fprintf(stdout, "Starting recording.\n");
                        }
                        else {
                            fprintf(stdout, "Stopping recording.\n");
                        }

                        // Toggle recording.
                        app->recording = !app->recording;
                        break;
                    }
                }

                break;
            }
        }
    }

    // // While recording, multiple may happen during one frame.
    // if (numMouseMotionEvents > 1) {
    //     fprintf(stdout, "numMouseMotionEvents: %d\n", numMouseMotionEvents);
    // }

    if (motion.hasValue) {
        const double mouseSens = 0.1e-9;

        app->horizLookRads +=
            mouseSens * motion.value.xrel * ddeltaNs;
        app->vertLookRads -=
            mouseSens * motion.value.yrel * ddeltaNs;

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
    }
}

// Orthographically project the given point onto the plane described
// by the given vectors.
// Use the app's camera position.
// Assume the look vectors are unit magnitude.
static V3d OrthoProject(App *app, V3d point, V3d lookRight, V3d lookUp) {
    V3d relativePoint = V3d_Sub(point, app->cameraPos);

    return (V3d) {
        V3d_Dot(relativePoint, lookRight),
        V3d_Dot(relativePoint, lookUp),
        0.0
    };
}

// Return the point projected onto the screen
// as a vector where [0.0, 0.0] means top-left of top-left pixel of screen
// and [1.0, 1.0] means bottom-right of bottom-right pixel of screen.
// Use the app's camera position and projection plane dimensions.
static V3d PointToScreenProportions(App *app, V3d point, V3d lookRight, V3d lookUp) {
    V3d projected = OrthoProject(app, point, lookRight, lookUp);

    return (V3d) {
        (+projected.x + (app->projPlaneWidth  / 2.0)) / app->projPlaneWidth,
        (-projected.y + (app->projPlaneHeight / 2.0)) / app->projPlaneHeight,
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

static MaybeV2i PointToPixel(App *app, V3d point, V3d lookForward, V3d lookRight, V3d lookUp) {
    V3d camToPoint = V3d_Sub(point, app->cameraPos);
    V3d unitCamToPoint = V3d_Unit(camToPoint);
    if (V3d_Dot(lookForward, unitCamToPoint) <= 0.0) {
        // Point is behind projection plane. Cannot be seen.
        return (MaybeV2i) { false };
    }

    V3d screenProps = PointToScreenProportions(app, point, lookRight, lookUp);

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

// // Currently not used.
// static void MaybeDrawPoint(App *app, V3d point, V3d look, V3d lookRight, V3d lookUp) {
//     MaybeV2i maybePixel = PointToPixel(app, point, look, lookRight, lookUp);
//
//     if (maybePixel.hasValue) {
//         fprintf(stdout, "maybePixel: (%d, %d)\n",
//             maybePixel.value.x, maybePixel.value.y);
//
//         Sdlu_RenderDrawPoint(
//             app->renderer, maybePixel.value.x, maybePixel.value.y);
//     }
// }

// Converting spherical coordinates to a vector.
// radius = 1.0 so not shown and no need to normalize the vector.
static inline V3d SphericalToCartesian(const double horizLookRads, const double vertLookRads) {
    return (V3d) {
        sin(vertLookRads) * cos(horizLookRads),
        sin(vertLookRads) * sin(horizLookRads),
        cos(vertLookRads)
    };
}

void App_Run(App *const app) {
    uint64_t oldTimeNs = Clock_GetTimeNs();
    uint64_t accumulatedNs = 0;
    const uint64_t periodNs = 1000000000 / 60;

    while (!app->quit) {
        const uint64_t newTimeNs = Clock_GetTimeNs();
        const uint64_t addNs = newTimeNs - oldTimeNs;
        accumulatedNs += addNs;

        if (accumulatedNs < periodNs) {
            goto end_of_while_loop;
        }
        else {
            accumulatedNs -= periodNs;
        }

        const uint64_t deltaNs = periodNs;
        const double ddeltaNs = (double)deltaNs;

        // const double fps = 1000000000.0 / ddeltaNs;
        // printf("fps: %lf\n", fps);
        // printf("oldTimeNs: %ld newTimeNs: %ld\n", oldTimeNs, newTimeNs);
        // printf("accumulatedNs: %ld\n", accumulatedNs);

        PollEvents(app, ddeltaNs);

        // Direction camera is looking.
        V3d look = SphericalToCartesian(app->horizLookRads, app->vertLookRads);

        // Calculate the up vector.
        const double lookUpRads = app->vertLookRads + (M_PI / 2.0);
        V3d lookUp = SphericalToCartesian(app->horizLookRads, lookUpRads);

        V3d xyForward = (V3d) {
            cos(app->horizLookRads),
            sin(app->horizLookRads),
            0.0
        };

        const double rightRads = app->horizLookRads + (M_PI / 2.0);
        V3d xyRight = (V3d) {
            cos(rightRads),
            sin(rightRads),
            0.0
        };

        // Assume camera never has roll (only pitch and yaw).
        V3d lookRight = xyRight;

        const uint8_t *const kbState = SDL_GetKeyboardState(NULL);

        // Movement speed.
        const double moveFactor = 0.0000005;

        // Movement in xy plane.

        if (kbState[SDL_SCANCODE_W] == 1) {
            app->cameraPos = V3d_Add(app->cameraPos, V3d_Mul(xyForward, moveFactor * ddeltaNs));
        }

        if (kbState[SDL_SCANCODE_S] == 1) {
            app->cameraPos = V3d_Sub(app->cameraPos, V3d_Mul(xyForward, moveFactor * ddeltaNs));
        }

        if (kbState[SDL_SCANCODE_A] == 1) {
            app->cameraPos = V3d_Sub(app->cameraPos, V3d_Mul(xyRight, moveFactor * ddeltaNs));
        }

        if (kbState[SDL_SCANCODE_D] == 1) {
            app->cameraPos = V3d_Add(app->cameraPos, V3d_Mul(xyRight, moveFactor * ddeltaNs));
        }

        // Movement up and down.

        if (kbState[SDL_SCANCODE_SPACE] == 1) {
            app->cameraPos.z -= moveFactor * ddeltaNs;
        }

        if (kbState[SDL_SCANCODE_Q] == 1) {
            app->cameraPos.z += moveFactor * ddeltaNs;
        }

        // fprintf(stdout, "Look angles: (%lf, %lf)\n",
        //     app->horizLookRads, app->vertLookRads);
        // printf("app->cameraPos: (%lf, %lf, %lf)\n",
        //     app->cameraPos.x, app->cameraPos.y, app->cameraPos.z);

        // Render

        // Fill screen with solid color.
        Sdlu_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
        Sdlu_RenderFillRect(app->renderer, NULL);

        const double cellWidth = 22.0;
        const double gridWidth = cellWidth * 22.0;
        const double gridHeight = cellWidth * 11.0;

        Sdlu_SetRenderDrawColor(app->renderer, 55, 55, 255, 255);

        // Draw lines on xy plane parallel to x-axis.
        for (double y = 0.0; y <= gridHeight; y += cellWidth) {
            // The start and end points of the line in world space.
            V3d worldStart = (V3d) {0.0, y, 0.0};
            V3d worldEnd = (V3d) {gridWidth, y, 0.0};

            MaybeV2i maybeStart = PointToPixel(app, worldStart, look, lookRight, lookUp);

            if (!maybeStart.hasValue) {
                continue;
            }

            MaybeV2i maybeEnd = PointToPixel(app, worldEnd, look, lookRight, lookUp);

            if (!maybeEnd.hasValue) {
                continue;
            }

            Sdlu_RenderDrawLine(app->renderer,
                maybeStart.value.x, maybeStart.value.y,
                maybeEnd.value.x, maybeEnd.value.y);
        }

        // Draw lines on xy plane parallel to y-axis.
        for (double x = 0.0; x <= gridWidth; x += cellWidth) {
            // The start and end points of the line in world space.
            V3d worldStart = (V3d) {x, 0.0, 0.0};
            V3d worldEnd = (V3d) {x, gridHeight, 0.0};

            MaybeV2i maybeStart = PointToPixel(app, worldStart, look, lookRight, lookUp);

            if (!maybeStart.hasValue) {
                continue;
            }

            MaybeV2i maybeEnd = PointToPixel(app, worldEnd, look, lookRight, lookUp);

            if (!maybeEnd.hasValue) {
                continue;
            }

            Sdlu_RenderDrawLine(app->renderer,
                maybeStart.value.x, maybeStart.value.y,
                maybeEnd.value.x, maybeEnd.value.y);
        }

        // // Draw 4 different-colored points near world origin.
        // Sdlu_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
        // MaybeDrawPoint(app, (V3d) {0.0, 0.0, 0.0}, look, lookRight, lookUp);
        //
        // Sdlu_SetRenderDrawColor(app->renderer, 255, 0, 0, 255);
        // MaybeDrawPoint(app, (V3d) {100.0, 0.0,  0.0}, look, lookRight, lookUp);
        //
        // Sdlu_SetRenderDrawColor(app->renderer, 0, 255, 0, 255);
        // MaybeDrawPoint(app, (V3d) {100.0, 100.0, 0.0}, look, lookRight, lookUp);
        //
        // Sdlu_SetRenderDrawColor(app->renderer, 0, 0, 255, 255);
        // MaybeDrawPoint(app, (V3d) {0.0, 100.0, 0.0}, look, lookRight, lookUp);

        SDL_RenderPresent(app->renderer);

        if (app->recording) {
            #define APP_FRAME_PATH_LEN 1024
            char path[APP_FRAME_PATH_LEN];
            snprintf(path, APP_FRAME_PATH_LEN, "screenshots/frame_%ld_%d.bmp",
                app->recordingId, app->frameNum);

            SaveBmp(app->renderer, path);

            app->frameNum += 1;
        }

        end_of_while_loop:
        oldTimeNs = newTimeNs;
    }
}

void App_Deinit(App *const app) {
    SDL_DestroyWindow(app->window);
    SDL_DestroyRenderer(app->renderer);
    SDL_Quit();
}
