#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stdint.h>

#include "SDL2/SDL.h"

#include "V3d.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;

    bool quit;
    bool recording;     // Whether saving each frame.
    time_t recordingId; // Used in frame file names so the frames are grouped.
    uint32_t frameNum;  // Start at 1.

    // Right-handed coordinate system. Positive z is down. Haha.

    V3d cameraPos; // Camera position.
    // Angle in radians of horizontal look angle.
    // 0        = Looking in positive x direction
    // PI / 2.0 = Looking in positive y direction
    double horizLookRads;
    // Angle in radians of vertical look angle.
    // 0        = Looking in positive z direction
    // PI / 2.0 = Looking parallel to xy plane
    double vertLookRads;

    // Dimensions of projection plane.
    double projPlaneWidth;
    double projPlaneHeight;

    // For scaling the projection plane.
    double projPlaneFactor;

    // Dimensions of projection plane before scaling.
    double baseProjPlaneWidth;
    double baseProjPlaneHeight;
} App;

// Initialize `app`.
void App_Init(App *const app);

// Run the app until user quits.
void App_Run(App *const app);

// Deinitialize `app`. Clean up internals.
void App_Deinit(App *const app);

#ifdef __cplusplus
}
#endif

#endif
