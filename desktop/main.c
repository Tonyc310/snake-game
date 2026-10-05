/* SDL calls these entry points each frame instead of the program running its own main() loop,
 * which is what lets the same code run in a browser, where a program can't block in a loop. */
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game.h"
#include "render.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    game_t game;
} app_t;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    app_t *app = SDL_calloc(1, sizeof *app);
    if (app == NULL) {
        return SDL_APP_FAILURE;
    }
    *appstate = app;

    if (!SDL_Init(SDL_INIT_VIDEO) ||
        !SDL_CreateWindowAndRenderer("Snake", RENDER_WIDTH, RENDER_HEIGHT, SDL_WINDOW_RESIZABLE,
                                     &app->window, &app->renderer)) {
        SDL_Log("Couldn't start: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    /* Draw at a fixed board size and let SDL scale it to the window, keeping its shape. */
    SDL_SetRenderLogicalPresentation(app->renderer, RENDER_WIDTH, RENDER_HEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    game_init(&app->game, (uint32_t)SDL_GetPerformanceCounter());
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    app_t *app = appstate;

    render_game(app->renderer, &app->game);
    SDL_RenderPresent(app->renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;
    return event->type == SDL_EVENT_QUIT ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    app_t *app = appstate;

    (void)result;
    if (app != NULL) {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_free(app);
    }
}
