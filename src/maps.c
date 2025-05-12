
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/tpx.h>
#include "particles_sim.h"
#include "maps.h"


const char* beach_matqueue[] = {
    "sky01",
    "sky01b",
    "sky01c",
    "sky01d",
    "sky01e",
    "A_A_water01",
    "A_B_sand01",
    "A_B_sand01b",
    "A_C_rock01",
    "A_C_rock01b",
    "A_D_bark01",
    "A_E_leaves01",
    "B_A_border01",
    "B_A_border01b",
  };
  
const char* beach_matqueue_b[] = {
    "B_A_border01c",
    "B_B_rainbow01",
    "B_C_blue01",
    "B_D_red01",
    "D_A_bark01",
    "D_B_leaves01"
  };

  const char* lava_matqueue[] = {
    "mat_lava",
    "mat_rocks_00",
    "mat_rocks",
    "mat_rocks_2",
    "mat_rocks_3",
    "mat_rocks_4",
  };
  
const char* lava_matqueue_b[] = {
    "mat_rocks_5",
  };


const char* castle_matqueue[] = {
    "f3d_trees",
    "f3d_stars",
    "f3d_moon",
    "f3d_dirt",
    "f3d_floor",
    "f3d_wall",
    "f3d_vtx",
    "f3d_metal",
    "f3d_wall_2",
    "f3d_metal_2",
  };
  
const char* castle_matqueue_b[] = {
    "f3d_wall_3",
    "f3d_wall_4",
};


const char* city_matqueue[] = {
    "f3d_asphalt",
    "f3d_stars",
    "f3d_building4",
    "f3d_building3",
    "f3d_building2",
    "f3d_building5",
    "f3d_building1",
    "f3d_building1b",
    "f3d_light1",
    "f3d_advert7",
    "f3d_advert2",
    "f3d_advert6",
    "f3d_advert5",
    "f3d_advert4",
    "f3d_advert2b",
    "f3d_advert3",
    "f3d_tree",
    "f3d_streetlamps",
    "f3d_signs",
    "f3d_line2",
    "f3d_line1",
    "f3d_fence1",
  };
  
const char* city_matqueue_b[] = {
    "f3d_fence_b",
};


const char* snow_matqueue[] = {
    "f3d_stars",
    "f3d_advert2b",
    "f3d_snow1",
    "f3d_snow2",
    "f3d_house",
    "f3d_trees",
    "f3d_advert1",
    "f3d_red1",
    "f3d_red2",
    "f3d_blue1",
    "f3d_blue2",
  };
  
const char* snow_matqueue_b[] = {
    "f3d_advert3",
    "f3d_advert4",
    "f3d_lines",
};

/*
typedef struct{
    const char* name;
    const char* previewimgfn;
    bool unlocked;

    const char* modelfn;

    const char* model_matnames;
    int         model_matnames_count;
    const char* model_matnames_b;
    int         model_matnames_b_count;

    const char* musicfn;
} mapinfo_t;
*/

mapinfo_t maps[6] = {
    {   .name = "BLUE SKY BEACH", 
        .previewimgfn = "rom:/textures/ui/stadiums/BSB.rgba32.sprite", 
        .unlocked = true, 
        .modelfn = "rom:/models/track01/track01_282.t3dm",
        .model_matnames = beach_matqueue, .model_matnames_count = 14,
        .model_matnames_b = beach_matqueue_b, .model_matnames_b_count = 6,
        .musicfn = "3.beach",
        .voicenamefn = "voice/Stadium_BlueSkyBeach"
      },
    {   .name = "COBBLESTONE CASTLE", 
        .previewimgfn = "rom:/textures/ui/stadiums/CSC.rgba32.sprite", 
        .unlocked = true, 
        .modelfn = "rom:/models/track03/track03_282.t3dm",
        .model_matnames = castle_matqueue, .model_matnames_count = 10,
        .model_matnames_b = castle_matqueue_b, .model_matnames_b_count = 2,
        .musicfn = "2.castle",
        .hdr.enabled = true, .hdr.tonemappingaverage = 0.33,
        .voicenamefn = "voice/Stadium_CobblestoneCastle",
      },
    {   .name = "MOON LIGHT CITY", 
        .previewimgfn = "rom:/textures/ui/stadiums/MLC.rgba32.sprite", 
        .unlocked = true, 
        .modelfn = "rom:/models/track04/track04_282.t3dm",
        .model_matnames = city_matqueue, .model_matnames_count = 22,
        .model_matnames_b = city_matqueue_b, .model_matnames_b_count = 1,
        .musicfn = "5.urban",
          .particles.count = 32, .particles.color = 0x2d343dff, .particles.scale = (T3DVec3){.x = 10, .y = 3, .z = 10}, .particles.size = 0.2f,
          .particles.texturefn = "rom:/textures/parts/rain.i8.sprite",
          .particles.initfunc = (void (*)(void *, int))generate_particles_random,
          .particles.updatefunc = (void (*)(void *, int))simulate_particles_rain,
        .hdr.enabled = true, .hdr.tonemappingaverage = 0.5,
        .voicenamefn = "voice/Stadium_MoonLightCity",
      },
    {   .name = "RED HOT ROCKS", 
        .previewimgfn = "rom:/textures/ui/stadiums/RHR.rgba32.sprite", 
        .unlocked = true, 
        .modelfn = "rom:/models/track02/track02_282.t3dm",
        .model_matnames = lava_matqueue, .model_matnames_count = 6,
        .model_matnames_b = lava_matqueue_b, .model_matnames_b_count = 1,
        .musicfn = "4.lava",
          .particles.count = 64, .particles.color = 0xffbd66ff, .particles.scale = (T3DVec3){.x = 10, .y = 3, .z = 10}, .particles.size = 0.2f,
          .particles.texturefn = "rom:/textures/parts/fire.i8.sprite",
          .particles.initfunc = (void (*)(void *, int))generate_particles_random,
          .particles.updatefunc = (void (*)(void *, int))simulate_particles_embers,
        .hdr.enabled = true, .hdr.tonemappingaverage = 0.2,
        .voicenamefn = "voice/Stadium_RedHotRocks",
      },
    {   .name = "LAPLAND VILLAGE", 
        .previewimgfn = "rom:/textures/ui/stadiums/LLV.rgba32.sprite", 
        .unlocked = false, 
        .modelfn = "rom:/models/track05/track05_282.t3dm",
        .model_matnames = snow_matqueue, .model_matnames_count = 11,
        .model_matnames_b = snow_matqueue_b, .model_matnames_b_count = 3,
        .musicfn = "8.holiday",
          .particles.count = 48, .particles.color = 0xb0edf7ff, .particles.scale = (T3DVec3){.x = 15, .y = 6, .z = 15}, .particles.size = 0.35f,
          .particles.texturefn = "rom:/textures/parts/fire.i8.sprite",
          .particles.initfunc = (void (*)(void *, int))generate_particles_random,
          .particles.updatefunc = (void (*)(void *, int))simulate_particles_snow,
        .hdr.enabled = true, .hdr.tonemappingaverage = 0.5,
        .voicenamefn = "voice/Stadium_LaplandVillage",
      },
    { .name = "RANDOM", 
      .previewimgfn = "rom:/textures/ui/stadiums/RND.rgba32.sprite", 
      .unlocked = true, 
      .modelfn = NULL},
};

