#include <libdragon.h>
#include <time.h>
#include <unistd.h>
#include <display.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "engine_gamestatus.h"
#include "engine_gfx.h"
#include "audioutils.h"
#include "effects.h"

effectdata_t effects;

T3DModel* expmodel = NULL;

void effects_init(){
    if(!expmodel) expmodel = t3d_model_load("rom:/models/exp/exp3d.t3dm");
    for(int i = 0; i < MAX_EFFECTS; i++){
        effects.exp3d[i].enabled = false;
        if(!effects.exp3d[i].matx) effects.exp3d[i].matx = malloc_uncached(sizeof(T3DMat4FP));

        effects.exp2d[i].enabled = false;
        
    }
    effects.screenshaketime = 0.0f;
    for(int i = 0; i < MAXPLAYERS; i++){
        effects.rumbletime[i] = 0;
        effects.rumblestate[i] = false;
    }
}
void effects_update(){
    for(int i = 0; i < MAX_EFFECTS; i++){
        if(effects.exp3d[i].enabled){
            effects.exp3d[i].time -= display_get_delta_time();
            effects.exp3d[i].color = get_rainbow_color(effects.exp3d[i].time * 2);
            effects.exp3d[i].color.a = effects.exp3d[i].time  * 127;
            if(effects.exp3d[i].time <= 0) effects.exp3d[i].enabled = false;
        }
        if(effects.exp2d[i].enabled){
            effects.exp2d[i].time -= display_get_delta_time();
            if(effects.exp2d[i].time <= 0) effects.exp2d[i].enabled = false;
        }
    }
    for(int i = 0; i < MAXPLAYERS; i++){
        if(effects.rumbletime[i] > 0){
            effects.rumbletime[i] -= display_get_delta_time();
            if(!effects.rumblestate[i]) {joypad_set_rumble_active(i, true); effects.rumblestate[i] = true;}
        } else if(effects.rumblestate[i]) {joypad_set_rumble_active(i, false); effects.rumblestate[i] = false;}
    }
    if(effects.screenshaketime > 0){
        effects.screenshaketime -= display_get_delta_time();
    } else effects.screenshaketime = 0.0f;
    for(int i = 0; i < MAXPLAYERS; i++){
        if(effects.ambientlight.r > 3) effects.ambientlight.r -=3;
        if(effects.ambientlight.g > 3) effects.ambientlight.g -=3;
        if(effects.ambientlight.b > 3) effects.ambientlight.b -=3;
        if(effects.ambientlight.a > 3) effects.ambientlight.a -=3;
    }
}

void effects_add_exp3d(T3DVec3 pos, color_t color){
    int i = 0; while(effects.exp3d[i].enabled && i < MAX_EFFECTS - 1) i++;

    effects.exp3d[i].enabled = true;
    effects.exp3d[i].position = pos;
    effects.exp3d[i].color = color;
    effects.exp3d[i].time = 2.0f;
    sound_play("explode", false);
}

void effects_rumble_stop(){
    for(int i = 0; i < MAXPLAYERS; i++){
        joypad_set_rumble_active(i, false);
        effects.rumblestate[i] = false;
    }
}

void effects_add_exp2d(T3DVec3 pos, color_t color){
    int i = 0; while(effects.exp2d[i].enabled && i < MAX_EFFECTS - 1) i++;

    effects.exp2d[i].enabled = true;
    effects.exp2d[i].position = pos;
    effects.exp2d[i].color = color;
    effects.exp2d[i].color.a = 255;
    effects.exp2d[i].time = 1.6f;
}

void effects_add_rumble(joypad_port_t port, float time){
    if(port < 0) return;
    if(!gamestatus.state.game.settings.vibration) return;
    effects.rumbletime[port] += time;
}

void effects_add_shake(float time){
    effects.screenshaketime += time;
}

void effects_add_ambientlight(color_t light){
    effects.ambientlight.r += light.r;
    effects.ambientlight.g += light.g;
    effects.ambientlight.b += light.b;
    effects.ambientlight.a += light.a;
}

void effects_draw(){
    for(int i = 0; i < MAX_EFFECTS; i++)
        if(effects.exp3d[i].enabled){
            float scale = 2.3f - effects.exp3d[i].time;
            effects.exp3d[i].color.a = effects.exp3d[i].time * (255 / 2);
            rdpq_set_prim_color(effects.exp3d[i].color);
            t3d_mat4fp_from_srt_euler(effects.exp3d[i].matx,
            (float[3]){scale,scale,scale},
            (float[3]){0,0,0},
            (float[3]){effects.exp3d[i].position.x * 64, effects.exp3d[i].position.y * 64, effects.exp3d[i].position.z * 64});
            t3d_matrix_push(effects.exp3d[i].matx);
            rdpq_sync_pipe(); // Hardware crashes otherwise
            rdpq_sync_tile(); // Hardware crashes otherwise
            t3d_model_draw(expmodel);
            rdpq_sync_pipe(); // Hardware crashes otherwise
            rdpq_sync_tile(); // Hardware crashes otherwise
            t3d_matrix_pop(1);
        }
    /*
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
    rdpq_mode_dithering(dither);
    rdpq_mode_filter(FILTER_BILINEAR);
    rdpq_mode_zbuf(false, false);
    for(int i = 0; i < MAX_EFFECTS; i++)
        if(effects.exp2d[i].enabled){
            T3DVec3 viewpos;
            t3d_viewport_calc_viewspace_pos(&viewport, &viewpos, &(effects.exp2d[i].position));
            rdpq_set_prim_color(effects.exp2d[i].color);
            float xpos = viewpos.v[0];
            float ypos = viewpos.v[1];
            int index = 16 - (effects.exp2d[i].time * 10);
            if(index >= 15) index = 15;
            if(index <= 0) index = 0;

            if(gfx_pos_within_viewport(xpos, ypos))
                rdpq_sprite_blit(sprites[spr_exp], xpos, ypos, &(rdpq_blitparms_t){.cx = 16, .cy = 16, .s0 = (index % 4)*32, (index / 4)*32, .width = 32, .height = 32});
        }*/
}

void effects_close(){
    for(int i = 0; i < MAX_EFFECTS; i++){
        effects.exp3d[i].enabled = false;
        if(effects.exp3d[i].matx) free_uncached(effects.exp3d[i].matx);
        effects.exp3d[i].matx = NULL;
        effects.exp2d[i].enabled = false;
    }
    for(int i = 0; i < MAXPLAYERS; i++)
        joypad_set_rumble_active(i, false);
    if(expmodel) t3d_model_free(expmodel);
    expmodel = NULL;
}