#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>

typedef enum { SOUND_EAT, SOUND_CRASH, SOUND_WIN } sound_t;

/** Opens the default audio device; returns false, and stays silent, if there isn't one. */
bool sound_init(void);

/** Queues a short generated effect; does nothing when muted or without an audio device. */
void sound_play(sound_t effect);

/** Switches sound off or back on. */
void sound_toggle_mute(void);

/** Closes the audio device. */
void sound_quit(void);

#endif
