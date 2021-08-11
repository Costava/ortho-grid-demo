#include "App.h"

int main(void) {
    App app;
    App_Init(&app);
    App_Run(&app);
    App_Deinit(&app);

    return 0;
}
