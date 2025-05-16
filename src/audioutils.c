#include <libdragon.h>
#include "audioutils.h"
#include "engine_gamestatus.h"
#include "ctype.h"

wav64_t bgmusic[12];
wav64_t sounds[32];
char bgmusicnames[64][12];
char soundsnames[64][32];

int musiccount = 0;
int soundscount = 0;

int sound_channel = 0;

bool bgmusic_playing;
bool sound_playing;
char bgmusic_name[SHORTSTR_LENGTH];
char sound_name[SHORTSTR_LENGTH];

int transitionstate;
float transitiontime;
float transitiontimemax;
bool loopingmusic;

int audio_prewarm_all_sounds_callback(const char *fn, dir_t *dir, void *data){
    char nameonly[128] = {0};
    strcpy(nameonly, strrchr(fn, '/') + 1);
    *strrchr(nameonly, '.') = '\0';
    strcpy(soundsnames[soundscount], nameonly);
    
    debugf("Found sound %i: %s  | filename %s\n", soundscount, nameonly, fn);
    wav64_open(&sounds[soundscount], fn);

    soundscount++;
    return DIR_WALK_CONTINUE;
}

int audio_prewarm_all_music_callback(const char *fn, dir_t *dir, void *data){

    char nameonly[128] = {0};
    strcpy(nameonly, strrchr(fn, '/') + 1);
    *strrchr(nameonly, '.') = '\0';
    strcpy(bgmusicnames[musiccount], nameonly);
    debugf("Found music %i: %s  | filename %s\n", musiccount, nameonly, fn);
    wav64_open(&bgmusic[musiccount], fn);

    musiccount++;
    return DIR_WALK_CONTINUE;
}


void audio_prewarm_all(){
    dir_glob("**/*.wav64", "rom:/music/", audio_prewarm_all_music_callback, NULL);
    dir_glob("**/*.wav64", "rom:/sfx/", audio_prewarm_all_sounds_callback, NULL);
}

int audio_find_sound(const char* name){
    int index = 0;
    while(strcmp(soundsnames[index], name) && index < 32) index++;
    if(index >= 32) assertf(0, "Sound not found %s", name);
    return index;
}

int audio_find_music(const char* name){
    int index = 0;
    while(strcmp(bgmusicnames[index], name) && index < 12) index++;
    if(index >= 12) assertf(0, "Music not found %s", name);
    return index;
}

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
            }
            if( bgmusic_name[0]){
                //char fn[512]; sprintf(fn, "rom:/music/%s.wav64",  bgmusic_name);
                //wav64_open(&bgmusic, fn);
                wav64_t* mus = &bgmusic[audio_find_music(bgmusic_name)];
                wav64_set_loop(mus,  loopingmusic);
                wav64_play(mus, AUDIO_CHANNEL_MUSIC);
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
    wav64_t* mus = &bgmusic[audio_find_music(name)];
    wav64_set_loop(mus,  loop);
    wav64_play(mus, AUDIO_CHANNEL_MUSIC);
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
    wav64_t* snd = &sounds[audio_find_sound(name)];
    wav64_set_loop(snd,  loop);
    wav64_play(snd, AUDIO_CHANNEL_SOUND + sound_channel*2);
    mixer_ch_set_vol(AUDIO_CHANNEL_SOUND + sound_channel*2,  gamestatus.state.audio.sound_vol,  gamestatus.state.audio.sound_vol);
    sound_channel++;
    if(sound_channel >= AUDIO_SOUND_MAXSOUNDS) sound_channel = 0;
     sound_playing = true;
    strcpy( sound_name, name);
}

void sound_stop(){
    if( sound_playing){
        mixer_ch_stop(AUDIO_CHANNEL_SOUND + sound_channel*2);
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