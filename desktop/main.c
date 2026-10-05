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
    Uint64 next_step_ms;
    unsigned best; /* best score this session */
    bool paused;
} app_t;

static void restart(app_t *app)
{
    game_init(&app->game, (uint32_t)SDL_GetPerformanceCounter());
    app->next_step_ms = SDL_GetTicks() + game_step_ms(&app->game);
    app->paused = false;
}

static const render_banner_t *banner_for(const app_t *app)
{
    static const render_banner_t paused = {"PAUSED", "P TO RESUME"};
    static const render_banner_t over = {"GAME OVER", "R TO RESTART"};
    static const render_banner_t won = {"BOARD FILLED - YOU WIN!", "R TO RESTART"};

    if (app->paused) {
        return &paused;
    }
    switch (app->game.status) {
    case GAME_OVER:
        return &over;
    case GAME_WON:
        return &won;
    default:
        return NULL;
    }
}

/* Scancodes are physical key positions, so WASD stays a cross on any keyboard layout. */
static bool direction_for(SDL_Scancode key, game_direction_t *direction)
{
    switch (key) {
    case SDL_SCANCODE_UP:
    case SDL_SCANCODE_W:
        *direction = GAME_UP;
        return true;
    case SDL_SCANCODE_DOWN:
    case SDL_SCANCODE_S:
        *direction = GAME_DOWN;
        return true;
    case SDL_SCANCODE_LEFT:
    case SDL_SCANCODE_A:
        *direction = GAME_LEFT;
        return true;
    case SDL_SCANCODE_RIGHT:
    case SDL_SCANCODE_D:
        *direction = GAME_RIGHT;
        return true;
    default:
        return false;
    }
}

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
    /* Without a frame cap, SDL calls SDL_AppIterate as fast as the CPU allows. */
    if (!SDL_SetRenderVSync(app->renderer, 1)) {
        SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60");
    }
    restart(app);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    app_t *app = appstate;
    Uint64 now = SDL_GetTicks();

    if (app->paused || app->game.status != GAME_PLAYING) {
        /* Nothing moves, so keep the next step a full interval away for when play resumes. */
        app->next_step_ms = now + game_step_ms(&app->game);
    } else if (now >= app->next_step_ms) {
        game_step(&app->game);
        app->best = SDL_max(app->best, app->game.score);
        app->next_step_ms += game_step_ms(&app->game);
        /* Keep a steady pace, but after a stall skip ahead instead of rushing to catch up. */
        if (app->next_step_ms < now) {
            app->next_step_ms = now + game_step_ms(&app->game);
        }
    }
    render_game(app->renderer, &app->game, app->best, banner_for(app));
    SDL_RenderPresent(app->renderer);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    app_t *app = appstate;
    game_direction_t direction;

    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    /* Clicking away from the window shouldn't cost the player their snake. */
    if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST && app->game.status == GAME_PLAYING) {
        app->paused = true;
    }
    if (event->type != SDL_EVENT_KEY_DOWN) {
        return SDL_APP_CONTINUE;
    }
    switch (event->key.scancode) {
    case SDL_SCANCODE_Q:
    case SDL_SCANCODE_ESCAPE:
        return SDL_APP_SUCCESS;
    case SDL_SCANCODE_P:
        /* Pausing only means something mid-game. */
        app->paused = !app->paused && app->game.status == GAME_PLAYING;
        break;
    case SDL_SCANCODE_R:
        restart(app);
        break;
    default:
        if (!app->paused && direction_for(event->key.scancode, &direction)) {
            game_turn(&app->game, direction);
        }
        break;
    }
    return SDL_APP_CONTINUE;
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
