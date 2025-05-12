#include <libdragon.h>
#include <time.h>
#include <unistd.h>
#include <display.h>
#include "engine_gfx.h"
#include <fmath.h>

int frame = 0;
float globaltime = 0;

float frandr( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

color_t get_rainbow_color(float s) {
  float r = fm_sinf(s + 0.0f) * 127.0f + 128.0f;
  float g = fm_sinf(s + 2.0f) * 127.0f + 128.0f;
  float b = fm_sinf(s + 4.0f) * 127.0f + 128.0f;
  return RGBA32(r, g, b, 255);
}

int randm(int max){
  return (rand() % max);
}

int randr(int min, int max){
  int range = min - max;
  return (rand() % range) + min;
}

void rdpq_sprite_blit_anchor(sprite_t* sprite, rdpq_align_t horizontal, rdpq_valign_t vertical, float x, float y, rdpq_blitparms_t* parms){
    assert(sprite);
    int width = sprite->width;
    int height = sprite->height;
    switch(horizontal){
        case ALIGN_RIGHT:
            x -= width; break;
        case ALIGN_CENTER:
            x -= width / 2; break;
        default: break;
    }
    switch(vertical){
        case VALIGN_BOTTOM:
            y -= height; break;
        case VALIGN_CENTER:
            y -= height / 2; break;
        default: break;
    }
    rdpq_sprite_blit(sprite, x,y,parms);
}

void rdpq_tex_blit_anchor(const surface_t* surface, rdpq_align_t horizontal, rdpq_valign_t vertical, float x, float y, rdpq_blitparms_t* parms){
    assert(surface);
    int width = surface->width;
    int height = surface->height;
    switch(horizontal){
        case ALIGN_RIGHT:
            x -= width; break;
        case ALIGN_CENTER:
            x -= width / 2; break;
        default: break;
    }
    switch(vertical){
        case VALIGN_BOTTOM:
            y -= height; break;
        case VALIGN_CENTER:
            y -= height / 2; break;
        default: break;
    }
    rdpq_tex_blit(surface, x,y,parms);
}
