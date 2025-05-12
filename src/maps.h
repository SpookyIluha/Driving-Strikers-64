#ifndef MAPS_H
#define MAPS_H
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

typedef struct{
    const char* name;
    const char* previewimgfn;
    bool unlocked;

    const char* modelfn;

    const char** model_matnames;
    int          model_matnames_count;
    const char** model_matnames_b;
    int          model_matnames_b_count;

    const char* musicfn;

    struct{
        int count;
        uint32_t color;
        const char* texturefn;
        void (*initfunc)(void*,int);
        void (*updatefunc)(void*,int);
        T3DVec3 scale; 
        float size;
    } particles;
    struct{
        bool enabled;
        float tonemappingaverage;
    } hdr;
    const char* voicenamefn;
} mapinfo_t;

extern mapinfo_t maps[6];

#endif