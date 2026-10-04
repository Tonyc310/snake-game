/* SDL calls these entry points each frame instead of the program running its own main() loop,
 * which is what lets the same code run in a browser, where a program can't block in a loop. */
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)appstate;
    (void)argc;
    (void)argv;
    return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    (void)appstate;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;
    (void)event;
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)appstate;
    (void)result;
}
