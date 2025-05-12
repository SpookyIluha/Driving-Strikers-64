#include <libdragon.h>
#include "audioutils.h"
#include "engine_gamestatus.h"

wav64_t bgmusic;
wav64_t sounds[AUDIO_SOUND_MAXSOUNDS];
int sound_channel = 0;

bool bgmusic_playing;
bool sound_playing;
char bgmusic_name[SHORTSTR_LENGTH];
char sound_name[SHORTSTR_LENGTH];

int transitionstate;
float transitiontime;
float transitiontimemax;
bool loopingmusic;

void audioutils_mixer_update(){
    float volume =  gamestatus.state.audio.bgmusic_vol;
    mixer_try_play();
    if( transitionstate == 1 &&  transitiontime > 0){
         transitiontime -= display_get_delta_time();

        if( transitiontime <= 0) {
             transitionstate = 2;
            if( bgmusic_playing){
                 bgmusic_playing = false;
                mixer_ch_stop(AUDIO_CHANNEL_MUSIC);
                rspq_wait();
                if(bgmusic.st) wav64_close(&bgmusic);
            }
            if( bgmusic_name[0]){
                char fn[512]; sprintf(fn, "rom:/music/%s.wav64",  bgmusic_name);
                wav64_open(&bgmusic, fn);
                wav64_set_loop(&bgmusic,  loopingmusic);
                wav64_play(&bgmusic, AUDIO_CHANNEL_MUSIC);
                 bgmusic_playing = true;
            }
        }
        volume *= ( transitiontime /  transitiontimemax);
    }
    if( transitionstate == 2 &&  transitiontime <  transitiontimemax){
         transitiontime += display_get_delta_time();
        volume *= ( transitiontime /  transitiontimemax);
    }
    mixer_ch_set_vol(AUDIO_CHANNEL_MUSIC, volume, volume);
}

void bgm_hardplay(const char* name, bool loop, float transition){
     loopingmusic = loop;
     transitionstate = 0;
     transitiontime = 0;
     transitiontimemax = 1;
    char fn[512]; sprintf(fn, "rom:/music/%s.wav64", name);
    if(bgmusic.st) wav64_close(&bgmusic);
    wav64_open(&bgmusic, fn);
    wav64_set_loop(&bgmusic,  loopingmusic);
    wav64_play(&bgmusic, AUDIO_CHANNEL_MUSIC);
     bgmusic_playing = true;
    strcpy( bgmusic_name, name);
}

void bgm_play(const char* name, bool loop, float transition){
    if(transition == 0) { bgm_hardplay(name, loop, transition); return;}
    bgm_stop(1);
     loopingmusic = loop;
     transitionstate = 1;
     transitiontime = transition;
     transitiontimemax = transition;
    strcpy( bgmusic_name, name);
}


void bgm_hardstop(){
     bgmusic_playing = false;
     transitionstate = 0;
     transitiontime = 0.1;
     transitiontimemax = 0.1;
     bgmusic_name[0] = 0;
    mixer_ch_stop(AUDIO_CHANNEL_MUSIC);
    rspq_wait();
    if(bgmusic.st) wav64_close(&bgmusic);
}

void bgm_stop(float transition){
    if(transition == 0) { bgm_hardstop(); return;}
     transitionstate = 1;
     transitiontime = 1;
     transitiontimemax = 1;
     bgmusic_name[0] = 0;
}

void sound_play(const char* name, bool loop){
    sound_stop();
    char fn[512]; sprintf(fn, "rom:/sfx/%s.wav64", name);
    wav64_open(&sounds[sound_channel], fn);
    wav64_set_loop(&sounds[sound_channel], loop);
    wav64_play(&sounds[sound_channel], AUDIO_CHANNEL_SOUND + sound_channel*2);
    mixer_ch_set_vol(AUDIO_CHANNEL_SOUND + sound_channel*2,  gamestatus.state.audio.sound_vol,  gamestatus.state.audio.sound_vol);
    sound_channel++;
    if(sound_channel >= AUDIO_SOUND_MAXSOUNDS) sound_channel = 0;
     sound_playing = true;
    strcpy( sound_name, name);
}

void sound_stop(){
    if( sound_playing){
        mixer_ch_stop(AUDIO_CHANNEL_SOUND + sound_channel*2);
        if(sounds[sound_channel].st) wav64_close(&sounds[sound_channel]);
    }
     sound_playing = false;
     sound_name[0] = 0;
}

void music_volume(float vol){
    gamestatus.state.audio.bgmusic_vol = vol;
}

void sound_volume(float vol){
    mixer_ch_set_vol(AUDIO_CHANNEL_SOUND, vol, vol);
    gamestatus.state.audio.sound_vol = vol;
}

float music_volume_get() {return  gamestatus.state.audio.bgmusic_vol;};

float sound_volume_get() {return  gamestatus.state.audio.sound_vol;};