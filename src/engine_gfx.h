#ifndef ENGINE_GFX_H
#define ENGINE_GFX_H
/// made by SpookyIluha
/// Various small GFX, util and string functions

#include <libdragon.h>
#include <time.h>
#include <display.h>

#define SFX_CHANNEL_MUSIC 0
#define SFX_CHANNEL_SOUND 2
#define SFX_MAX_CHANNELS  8

extern int frame;
extern float globaltime;

#define T3D_TOUNITS(x) (64.0f*x)
#define T3D_FROMUNITS(x) (x*(1.0/64.0f))

#define TPE_TOUNITS(x) (512.0f*x)
#define TPE_FROMUNITS(x) (x*(1.0/512.0f))

float frandr( float min, float max );

/// @brief rdpq_sprite_blit but with anchor support
/// @param sprite 
/// @param horizontal horizontal anchor
/// @param vertical  vertical anchor
/// @param x 
/// @param y 
/// @param parms 
void rdpq_sprite_blit_anchor(sprite_t* sprite, rdpq_align_t horizontal, rdpq_valign_t vertical, float x, float y, rdpq_blitparms_t* parms);

/// @brief rdpq_tex_blit but with anchor support
/// @param surface 
/// @param horizontal horizontal anchor
/// @param vertical  vertical anchor
/// @param x 
/// @param y 
/// @param parms 
void rdpq_tex_blit_anchor(const surface_t* surface, rdpq_align_t horizontal, rdpq_valign_t vertical, float x, float y, rdpq_blitparms_t* parms);

/// @brief Returns whether a point is within the screen
/// @param x 
/// @param y 
/// @return true if it is
inline bool gfx_pos_within_viewport(float x, float y){
    return x > 0 && x < display_get_width() && y > 0 && y < display_get_height();
}

/// @brief Returns whether a point is within the rectangle
/// @param x 
/// @param y 
/// @return true if it is
inline bool gfx_pos_within_rect(float x, float y, float xa, float ya, float xb, float yb){
    return x > xa && x < xb && y > ya && y < yb;
}

/// @brief Generic linear interp function
/// @param a 
/// @param b 
/// @param t 
/// @return 
inline float gfx_lerp(float a, float b, float t)
{
    if(fabs(a - b) < FM_EPSILON) return b;
    return a + (b - a) * t;
}


inline float fclampr(float x, float min, float max){
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

inline float fwrap(float x, float min, float max) {
    if (min > max) {
        return fwrap(x, max, min);
    }
    return (x >= 0 ? min : max) + fmod(x, max - min);
}

inline int iwrap(int x, float min, float max) {
    if(x > max) return min;
    if(x < min) return max;
    return x;
}

void iswap(int* a, int* b);

color_t get_rainbow_color(float s);

/// @brief Random int [0-max)
/// @param max 
/// @return 
int randm(int max);

/// @brief Random int [min-max)
/// @param max 
/// @return 
int randr(int min, int max);

#endif