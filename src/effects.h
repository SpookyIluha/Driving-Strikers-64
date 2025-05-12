#ifndef EFFECTS_H
#define EFFECTS_H

#include <libdragon.h>
#include <time.h>
#include <display.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

#define MAX_EFFECTS 4
#define MAXPLAYERS 4

typedef struct effectdata_s{
    struct{
        bool enabled;
        T3DVec3 position;
        float time;
        color_t color;
        T3DMat4FP* matx;
    } exp3d[MAX_EFFECTS];
    struct{
        bool enabled;
        T3DVec3 position;
        float time;
        color_t color;
    } exp2d[MAX_EFFECTS];

    float rumbletime[MAXPLAYERS];
    bool  rumblestate[MAXPLAYERS];
    float screenshaketime;
    color_t ambientlight;
} effectdata_t;

extern effectdata_t effects;

void effects_init();
void effects_update();
void effects_draw();
void effects_close();

void effects_add_exp3d(T3DVec3 pos, color_t color);
void effects_add_exp2d(T3DVec3 pos, color_t color);

void effects_add_rumble(joypad_port_t port, float time);
void effects_add_shake(float time);
void effects_add_ambientlight(color_t light);

void effects_rumble_stop();


#endif