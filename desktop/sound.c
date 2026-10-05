#include "sound.h"

#include <SDL3/SDL.h>

#define RATE 44100
#define VOLUME 0.12f
#define MAX_SAMPLES (RATE / 2) /* every effect is shorter than half a second */

typedef struct {
    float start_hz;
    float end_hz;
    int ms;
} note_t;

typedef struct {
    const note_t *notes;
    int count;
} effect_t;

static const note_t EAT[] = {{660.0f, 990.0f, 70}};
static const note_t CRASH[] = {{330.0f, 82.0f, 380}};
static const note_t WIN[] = {{523.0f, 523.0f, 120}, {659.0f, 659.0f, 120}, {784.0f, 784.0f, 220}};

static const effect_t EFFECTS[] = {
    [SOUND_EAT] = {EAT, SDL_arraysize(EAT)},
    [SOUND_CRASH] = {CRASH, SDL_arraysize(CRASH)},
    [SOUND_WIN] = {WIN, SDL_arraysize(WIN)},
};

static SDL_AudioStream *stream; /* NULL when there's no audio device */
static bool muted;
static float samples[MAX_SAMPLES];

/* Writes a square wave sweeping from start_hz to end_hz at `at`; returns where it ends.
 * It fades out to silence, because a wave cut off mid-swing clicks. */
static int add_note(int at, note_t note)
{
    const int count = SDL_min(RATE * note.ms / 1000, MAX_SAMPLES - at);
    float phase = 0.0f;

    for (int i = 0; i < count; i++) {
        const float progress = (float)i / (float)count;

        phase += (note.start_hz + (note.end_hz - note.start_hz) * progress) / RATE;
        phase -= SDL_floorf(phase);
        samples[at + i] = (phase < 0.5f ? VOLUME : -VOLUME) * (1.0f - progress);
    }
    return at + count;
}

bool sound_init(void)
{
    const SDL_AudioSpec spec = {SDL_AUDIO_F32, 1, RATE};

    /* Separate from video, so a machine without audio still gets a playable, silent game. */
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        return false;
    }
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    return stream != NULL && SDL_ResumeAudioStreamDevice(stream);
}

void sound_play(sound_t effect)
{
    int length = 0;

    if (stream == NULL || muted) {
        return;
    }
    for (int i = 0; i < EFFECTS[effect].count; i++) {
        length = add_note(length, EFFECTS[effect].notes[i]);
    }
    SDL_PutAudioStreamData(stream, samples, length * (int)sizeof samples[0]);
}

void sound_toggle_mute(void)
{
    muted = !muted;
}

void sound_quit(void)
{
    SDL_DestroyAudioStream(stream);
    stream = NULL;
}
