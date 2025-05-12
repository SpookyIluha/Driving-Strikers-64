#include <libdragon.h>
#include "engine_gamestatus.h"
#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H

/// made by SpookyIluha
/// Audio util functions

#ifdef __cplusplus
extern "C"{
#endif

extern bool bgmusic_playing;
extern bool sound_playing;
extern char bgmusic_name[SHORTSTR_LENGTH];
extern char sound_name[SHORTSTR_LENGTH];

#define AUDIO_CHANNEL_MUSIC 0
#define AUDIO_CHANNEL_SOUND 2
#define AUDIO_SOUND_MAXSOUNDS 4

/// @brief Should be called on each game tick for audio to be played (between rdpq_attach and rdpq_detach)
void audioutils_mixer_update();

/// @brief Play music in the background
/// @param name fn of the music in the bgm folder
/// @param loop is the music looped
/// @param transition transition in seconds
void bgm_play(const char* name, bool loop, float transition);

/// @brief Stop the currently playing music
/// @param transition transition in seconds
void bgm_stop(float transition);

/// @brief Play sound effect
/// @param name fn of the sound in the sfx folder
/// @param loop is the music looped
void sound_play(const char* name, bool loop);

/// @brief Stop the currently playing music
void sound_stop();

/// @brief Set the music volume
/// @param vol 0-1 range
void music_volume(float vol);

/// @brief Set the sounds volume
/// @param vol 0-1 range
void sound_volume(float vol);

/// @brief Get the music current volume
/// @return 0-1 float
float music_volume_get();

/// @brief Get the sounds current volume
/// @return 0-1 float
float sound_volume_get();

#ifdef __cplusplus
}
#endif

#endif