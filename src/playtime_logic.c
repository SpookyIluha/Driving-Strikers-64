#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/tpx.h>
#include "vi.h"
#include "engine_gfx.h"
#include "teams.h"
#include "entity_logic.h"
#include "audioutils.h"
#include "effects.h"
#include "playtime_logic.h"
#include "maps.h"
#include "engine_locale.h"

match_t matchinfo;
mapinfo_t* current_map = NULL;

T3DViewport viewport[6];
color_t border_time_color;
color_t border_time_score;
uint8_t colorAmbient[4] = {255, 255, 255, 0xFF};

  
  // Now allocate a fixed-point matrix, this is what t3d uses internally.
  // Note: this gets DMA'd to the RSP, so it needs to be uncached.
  // If you can't allocate uncached memory, remember to flush the cache after writing to it instead.
T3DModel *model;
T3DMat4 modelMat;
T3DMat4FP* modelMatFP = NULL;

T3DMat4FP* particleMatFP = NULL;
TPXParticle* mapparticles = NULL;
sprite_t*  mapparticles_sprite  = NULL;
  // Load a model-file, this contains the geometry and some metadata

float offset[2] = {0};
T3DModelState matstate;
TPE_World world;

/**
 * Simple example with a 3d-model file created in blender.
 * This uses the builtin model format for loading and drawing a model.
 */
// The demo will only run for a single frame and stop.

rspq_block_t *dplDraw = NULL;
rspq_block_t *dplDrawB = NULL;
rspq_block_t *dplDrawHUD = NULL;

TPE_Unit 
ramp1[6] = { TPE_TOUNITS(3),TPE_TOUNITS(-3), TPE_TOUNITS(3),TPE_TOUNITS(3), TPE_TOUNITS(-3),TPE_TOUNITS(3) },
ramp2[6] = { TPE_TOUNITS(-3),TPE_TOUNITS(-3), TPE_TOUNITS(3),TPE_TOUNITS(-3), TPE_TOUNITS(3),TPE_TOUNITS(3)  },
ramp3[6] = { TPE_TOUNITS(3),TPE_TOUNITS(3), TPE_TOUNITS(-3),TPE_TOUNITS(3), TPE_TOUNITS(-3),TPE_TOUNITS(-3)  },
ramp4[6] = { TPE_TOUNITS(-3),TPE_TOUNITS(3), TPE_TOUNITS(-3),TPE_TOUNITS(-3), TPE_TOUNITS(3),TPE_TOUNITS(-3)  };

typedef struct{
  T3DVec3 camPos;
  T3DVec3 camTarget;

  T3DVec3 camPos_off;
  T3DVec3 camTarget_off;
} camera_t;
camera_t maincamera;

typedef struct{
  int idx;
  float dist;
} entitysortgraph_t;

int entitysortgraph_compare( const void* a, const void* b)
{
    entitysortgraph_t* A = ( (entitysortgraph_t*) a );
    entitysortgraph_t* B = ( (entitysortgraph_t*) b );
     
     if ( A->dist == B->dist ) return 0;
     else if ( A->dist < B->dist ) return -1;
     else return 1;
}

float exposure = 5.0f;
float exposure_bias = 0.55;
float average_brightness = 0;

#define RAND_SAMPLE_COUNT  16
uint32_t sampleOffsets[RAND_SAMPLE_COUNT] = {0};

// autoexposure function for HDR lighting, this will control how bright the vertex colors are in range of 0-1 after T&L for the RDP HDR modulation through color combiner
void exposure_set(void* framebuffer){
  if(!framebuffer) return;
  surface_t* frame = (surface_t*) framebuffer;

  // sample points across the screen
  uint64_t *pixels = frame->buffer; // get the previous framebuffer (assumed 320x240 RGBA16 format), it shouldn't be cleared for consistency
  uint32_t maxIndex = frame->width * frame->height * 2 / 8;

  if(sampleOffsets[0] == 0) {
    for(int i = 0; i < RAND_SAMPLE_COUNT; i++){
      sampleOffsets[i] = rand() % maxIndex;
    }
  }

  uint64_t brightInt = 0;
  for (int i = 0; i < RAND_SAMPLE_COUNT; ++i) {
    uint64_t pixels4 = pixels[sampleOffsets[i]];
    for(int j = 0; j < 4; j++){
      brightInt += (pixels4 & 0b11111'00000'00000'0) >> 10;
      brightInt += (pixels4 & 0b00000'11111'00000'0) >> 5;
      brightInt += (pixels4 & 0b00000'00000'11111'0);
      pixels4 >>= 16;
    }
  }

  brightInt /= RAND_SAMPLE_COUNT * 4;
  average_brightness = brightInt / (63.0f * 3.0f);

  // exposure bracket uses an overall bias of how the bright the framebuffer is at 0-1 scale
  // eg. if the avegare brightness of the framebuffer is > 0.7, then the exposure needs to go down until
  // the average brightness is 0.7, and the other way around
  if(average_brightness > exposure_bias) {
    exposure -= 0.01f;
  }
  else if(average_brightness < exposure_bias - 0.1f) {
    exposure += 0.01f;
  }

  // min/max exposure levels
  if(exposure > 3) exposure -= 0.05f;
  if(exposure < 0.5f) exposure = 0.5f;
}


float camlocations[5][4][3] = {
  {{7,5,24},{-7,5,24},{7,5,-24},{-7,5,-24}},
  {{16,8,15},{-16,8,15},{20,4,5},{-20,4,5}},
  {{18,12,15},{18,5,15},{6,4,5},{6,4,5}},
  {{0,5,17},{0,5,17},{6,4,5},{-6,4,5}},
  {{0,5,24},{0,5,12},{0,0,0},{0,0,0}}
};

/** Function used for defining static environment, working similarly to an SDF
  (signed distance function). The parameters are: 3D point P, max distance D.
  The function should behave like this: if P is inside the solid environment
  volume, P will be returned; otherwise closest point (by Euclidean distance) to
  the solid environment volume from P will be returned, except for a case when
  this closest point would be further away than D, in which case any arbitrary
  point further away than D may be returned (this allows for optimizations). */
TPE_Vec3 environmentDistance(TPE_Vec3 point, TPE_Unit maxDistance)
{
  TPE_ENV_START( TPE_envGround(point,0),point )
  TPE_ENV_NEXT( TPE_envAABoxInside(point, TPE_vec3(0,0,0), TPE_vec3(TPE_TOUNITS(40),TPE_TOUNITS(40),TPE_TOUNITS(26))), point)
  if(point.x > 0){
    if(point.z > 0){
      TPE_ENV_NEXT( TPE_envAABox(point, TPE_vec3(TPE_TOUNITS(20),TPE_TOUNITS(0),TPE_TOUNITS(8)), TPE_vec3(TPE_TOUNITS(2),TPE_TOUNITS(40),TPE_TOUNITS(4))), point)
      TPE_ENV_NEXT(TPE_envAATriPrism(point, TPE_vec3(TPE_TOUNITS(15),(0),TPE_TOUNITS(10)), ramp1, TPE_TOUNITS(40), 1),  point)
    } else{  
      TPE_ENV_NEXT( TPE_envAABox(point, TPE_vec3(TPE_TOUNITS(20),TPE_TOUNITS(0),TPE_TOUNITS(-8)), TPE_vec3(TPE_TOUNITS(2),TPE_TOUNITS(40),TPE_TOUNITS(4))), point)
      TPE_ENV_NEXT(TPE_envAATriPrism(point, TPE_vec3(TPE_TOUNITS(15),(0),TPE_TOUNITS(-10)), ramp2, TPE_TOUNITS(40), 1),  point)
    }
  } else{
    if(point.z > 0){
      TPE_ENV_NEXT( TPE_envAABox(point, TPE_vec3(TPE_TOUNITS(-20),TPE_TOUNITS(0),TPE_TOUNITS(8)), TPE_vec3(TPE_TOUNITS(2),TPE_TOUNITS(40),TPE_TOUNITS(4))), point)
      TPE_ENV_NEXT(TPE_envAATriPrism(point, TPE_vec3(TPE_TOUNITS(-15),(0),TPE_TOUNITS(10)), ramp3, TPE_TOUNITS(40), 1),  point)
    }else  { 
      TPE_ENV_NEXT( TPE_envAABox(point, TPE_vec3(TPE_TOUNITS(-20),TPE_TOUNITS(0),TPE_TOUNITS(-8)), TPE_vec3(TPE_TOUNITS(2),TPE_TOUNITS(40),TPE_TOUNITS(4))), point)
      TPE_ENV_NEXT(TPE_envAATriPrism(point, TPE_vec3(TPE_TOUNITS(-15),(0),TPE_TOUNITS(-10)), ramp4, TPE_TOUNITS(40), 1),  point)
    }
  }

  TPE_ENV_END
}


  /** Function that can be used as a joint-joint or joint-environment collision
  callback, parameters are following: body1 index, joint1 index, body2 index,
  joint2 index, collision world position. If body1 index is the same as body1
  index, then collision type is body-environment, otherwise it is body-body
  type. The function has to return either 1 if the collision is to be allowed
  or 0 if it is to be discarded. This can besides others be used to disable
  collisions between some bodies. */


void entityCollisionCallback_ballcar(uint16_t body1idx, uint16_t body2idx){
  int caridx = playball.bodyidx == body1idx? body2idx : body1idx;
  bool shotontarget = playball.ballPos_t3d.x > 10 || playball.ballPos_t3d.x < -10;
  
  if(carplayer[caridx].team->id == matchinfo.tleft.team->id) {matchinfo.tleft.posession++; }
  else {matchinfo.tright.posession++; }

  if(caridx < 4) {
    if(!carplayer[caridx].ballcollided) {
      if(carplayer[caridx].team->id == matchinfo.tleft.team->id) {matchinfo.tleft.shots++; if (shotontarget) matchinfo.tleft.shotsontarget++; }
      else {matchinfo.tright.shots++; if(shotontarget) matchinfo.tright.shotsontarget++;}
      sound_play("ball", false);
      effects_add_rumble(carplayer[caridx].playercontroller, 0.1f);
      effects_add_shake(0.1f);
    }
    carplayer[caridx].ballcollided = true;
  }
}

void entityCollisionCallback_carcar(uint16_t body1idx, uint16_t body2idx) {
  if(carplayer[body1idx].carcollided < 0) {
    if(rand() % 4 == 0) sound_play("crash_horn", false);
    else sound_play("crash", false);
    effects_add_rumble(carplayer[body1idx].playercontroller, 0.2f);
    effects_add_shake(0.15f);
  }
  carplayer[body1idx].carcollided = body2idx;

  if(carplayer[body2idx].carcollided < 0) {
    if(rand() % 4 == 0) sound_play("crash_horn", false);
    else sound_play("crash", false);
    effects_add_rumble(carplayer[body2idx].playercontroller, 0.2f);
    effects_add_shake(0.15f);
  }
  carplayer[body2idx].carcollided = body1idx;
}
  
uint8_t entityCollisionCallback(uint16_t body1idx, uint16_t joint1idx, uint16_t body2idx, uint16_t joint2idx, TPE_Vec3 position){
  if(body1idx != body2idx){
    // ball-car collision
    if(playball.bodyidx == body1idx || playball.bodyidx == body2idx)
      entityCollisionCallback_ballcar(body1idx, body2idx);

    // car-car collision
    if(playball.bodyidx != body1idx && playball.bodyidx != body2idx)
      entityCollisionCallback_carcar(body1idx, body2idx);
  }
    return 1;
}
  
void matchinfo_init(){
  memset(&matchinfo, 0, sizeof(matchinfo));
}

void game_draw_particles(){
      rdpq_texparms_t p = {0};
      p.s.repeats = REPEAT_INFINITE;
      p.t.repeats = REPEAT_INFINITE;
      p.s.scale_log = -2;
      p.t.scale_log = -2;
      rdpq_sprite_upload(TILE0, mapparticles_sprite, &p);
      rdpq_set_mode_standard();
      rdpq_set_env_color(color_from_packed32(current_map->particles.color));
      rdpq_mode_filter(FILTER_BILINEAR);
      rdpq_mode_combiner(RDPQ_COMBINER1((ENV,0,TEX0,0), (ENV,0,TEX0,0)));
      rdpq_mode_antialias(AA_REDUCED);
      rdpq_mode_alphacompare(10);

      tpx_state_from_t3d();
      tpx_state_set_scale(current_map->particles.size, current_map->particles.size);

      tpx_state_set_tex_params(0, 0);
      tpx_particle_draw_tex(mapparticles, current_map->particles.count);
  }

void game_draw(){
  surface_t fb = display_get_current_framebuffer();
  if(current_map->hdr.enabled) exposure_set(&fb);
  else exposure = 1;
  float modelScale = 1.0f / 8;

  t3d_vec3_lerp(&maincamera.camTarget_off, &maincamera.camTarget_off, &maincamera.camTarget, 0.1f);
  t3d_vec3_lerp(&maincamera.camPos_off, &maincamera.camPos_off, &maincamera.camPos, 0.1f);
  t3d_viewport_set_projection(&viewport[(frame) % 6], T3D_DEG_TO_RAD(70.0f), 16.0f, 1000.0f);  
  T3DVec3 camtarg_shake = (T3DVec3){.x = frandr(-1,1) * effects.screenshaketime, .y = frandr(-1,1) * effects.screenshaketime, .z = frandr(-1,1) * effects.screenshaketime};
  t3d_vec3_add(&camtarg_shake, &maincamera.camTarget_off, &camtarg_shake);
  t3d_viewport_look_at(&viewport[(frame) % 6], &maincamera.camPos_off, &camtarg_shake, &(T3DVec3){{0,1,0}});

  t3d_mat4_from_srt_euler(&modelMat,
    (float[3]){modelScale, modelScale, modelScale},
    (float[3]){0,0,0},
    (float[3]){0,0,0}
  );

  if(particleMatFP)
  t3d_mat4fp_from_srt_euler(particleMatFP,
    (float[3]){current_map->particles.scale.x, current_map->particles.scale.y, current_map->particles.scale.z},
    (float[3]){0,0,0},
    (float[3]){0,current_map->particles.scale.y * 64,0}
  );

  t3d_mat4_to_fixed(modelMatFP, &modelMat);

    t3d_frame_start();
    t3d_viewport_attach(&viewport[(frame) % 6]);
    rdpq_set_scissor(0,0,display_get_width(),display_get_height(), display_get_rdpinterlace(), display_get_rdpfield());

    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_count(0);

    // you can use the regular rdpq_* functions with t3d.
    // In this example, the colored-band in the 3d-model is using the prim-color,
    // even though the model is recorded, you change it here dynamically.
    rdpq_sync_pipe();
    t3d_matrix_push(modelMatFP); 
    if(!dplDraw) {
      rspq_block_begin();
      rdpq_mode_zbuf(false,false);
      rdpq_mode_antialias(AA_REDUCED);
      // Draw the model, material settings (e.g. textures, color-combiner) are handled internally
      for(int i = 0; i < current_map->model_matnames_count; i++){
        T3DModelIter iter = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
        while(t3d_model_iter_next(&iter)){
          if(!strcmp(iter.object->material->name, current_map->model_matnames[i])){
            t3d_model_draw_material(iter.object->material, &matstate);
            t3d_model_draw_object(iter.object, NULL);
          }
        }
    }
      dplDraw = rspq_block_end();
    }


    // for the actual draw, you can use the generic rspq-api.
    rspq_block_run(dplDraw);
    t3d_matrix_push(modelMatFP); 
    t3d_light_set_exposure(exposure);
    
    if(gamestatus.state.game.settings.graphics == NICEST)
      for(int i = 0; i < 4; i++) carplayer_draw_shadow(&carplayer[i]);
    playball_draw_shadow();

    //rdpq_mode_antialias(AA_STANDARD);
    // quick-sort the entities so that they appear correct without a zbuffer
    entitysortgraph_t graph[5] = {
      {.idx = 0, .dist = t3d_vec3_distance2(&maincamera.camPos_off, &carplayer[0].Pos_t3d)},
      {.idx = 1, .dist = t3d_vec3_distance2(&maincamera.camPos_off, &carplayer[1].Pos_t3d)},
      {.idx = 2, .dist = t3d_vec3_distance2(&maincamera.camPos_off, &carplayer[2].Pos_t3d)},
      {.idx = 3, .dist = t3d_vec3_distance2(&maincamera.camPos_off, &carplayer[3].Pos_t3d)},
      {.idx = 4, .dist = t3d_vec3_distance2(&maincamera.camPos_off, &playball.ballPos_t3d)},
    };
    qsort(graph, 5, sizeof(entitysortgraph_t), entitysortgraph_compare);
    for(int i = 4; i >= 0; i--){
      if(graph[i].idx == 4) playball_draw();
      else carplayer_draw(&carplayer[graph[i].idx]);
    }
    t3d_matrix_pop(1);
    matstate = t3d_model_state_create();
    if(!dplDrawB) {
      rspq_block_begin();
      rdpq_mode_zbuf(false,false);
      rdpq_mode_antialias(gamestatus.state.game.settings.graphics == NICEST? AA_STANDARD : AA_REDUCED);
      // Draw the model, material settings (e.g. textures, color-combiner) are handled internally
      for(int i = 0; i < current_map->model_matnames_b_count; i++){
        T3DModelIter iter = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
        while(t3d_model_iter_next(&iter)){
          if(!strcmp(iter.object->material->name, current_map->model_matnames_b[i])){
            t3d_model_draw_material(iter.object->material, &matstate);
            t3d_model_draw_object(iter.object, NULL);
          }
        }
    }
      dplDrawB = rspq_block_end();
    } rspq_block_run(dplDrawB);

    effects_draw();
    if(matchinfo.countdown > 0){
      t3d_matrix_push(modelMatFP); 
      rdpq_sync_pipe();
      rdpq_set_mode_standard();
      rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,ENV), (TEX0,0,ENV,0)));
      rdpq_mode_alphacompare(50);
      rdpq_set_env_color(RGBA32(128,255,255,matchinfo.countdown * 96));
      for(int i =  0; i < 4; i++) carplayer_draw_teleport_particles(&carplayer[i]);
      t3d_matrix_pop(1);
    }

    if(current_map->particles.count) {
      t3d_matrix_push(particleMatFP); 
      game_draw_particles();
      t3d_matrix_pop(1);
    }

    if(!dplDrawHUD){
      rspq_block_begin();
      t3d_matrix_pop(1);
      rdpq_sync_pipe();
      rdpq_set_mode_fill(border_time_color);
      rdpq_fill_rectangle(190,24,240,44);
      rdpq_set_mode_fill(color_from_packed32(matchinfo.tleft.team->teamcolor));
      rdpq_fill_rectangle(240,24,282,44);
      rdpq_set_mode_fill(border_time_score);
      rdpq_fill_rectangle(282,24,353,44);
      rdpq_set_mode_fill(color_from_packed32(matchinfo.tright.team->teamcolor));
      rdpq_fill_rectangle(353,24,395,44);
  
      rdpq_textparms_t textparms = {0};
      textparms.align = ALIGN_CENTER;
      textparms.width = 240 - 190;
      rdpq_text_printf(&textparms, 1, 190, 39, "%02i:%02i", (int)matchinfo.matchtimeleft / 60, (int)matchinfo.matchtimeleft % 60);
      textparms.width = 282 - 240;
      textparms.style_id = matchinfo.tleft.team->style;
      rdpq_text_printf(&textparms, 1, 240, 39, matchinfo.tleft.team->teamshortname);
      textparms.style_id = matchinfo.tright.team->style;
      rdpq_text_printf(&textparms, 1, 353, 39, matchinfo.tright.team->teamshortname);
      textparms.width = 353 - 282;
      textparms.style_id = 1;
      rdpq_text_printf(&textparms, 1, 282, 39, "%i - %i", matchinfo.tleft.score, matchinfo.tright.score);
      dplDrawHUD = rspq_block_end();
    } rspq_block_run(dplDrawHUD);

    //rdpq_text_printf(NULL, 1, 50, 50, "^01FPS: %.2f", display_get_fps());
    //heap_stats_t stats; sys_get_heap_stats(&stats);
    //rdpq_text_printf(NULL, 1, 30, 50, "MEM: %i total, %i used", stats.total, stats.used);
}

bool game_pause(){
  sprite_t* selector_spr = sprite_load("rom:/textures/ui/selector.rgba32.sprite");
  sprite_t* button_a = sprite_load("rom:/textures/ui/button_a.rgba32.sprite");
  bool quit = false;
  int selection = 0;
  frame--; // to fix the matrix buffering
      while(matchinfo.state == MATCH_PAUSE){
        joypad_poll();
        audioutils_mixer_update();
        effects_update();
        joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        int axis_pressed = joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_Y);
        if(pressed.start) {
          matchinfo.state = MATCH_PLAY;
          break;
        } if(pressed.a) {
          matchinfo.state = MATCH_PLAY;
          if(selection == 1) {quit = true; matchinfo.exited = true;}
          break;
        } if(axis_pressed){
          sound_play("menu_navigate", false);
          selection = 1 - selection;
        }
        rdpq_attach(display_get(), NULL);
        rdpq_set_scissor(0,0,display_get_width(),display_get_height(), display_get_rdpinterlace(), display_get_rdpfield());
        game_draw();
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_antialias(AA_REDUCED);
        rdpq_set_prim_color(RGBA32(0,0,0,128));
        rdpq_fill_rectangle(190, 200, 450, 280);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
        rdpq_sprite_blit(button_a, 450, 425, NULL);
        rdpq_sprite_blit(selector_spr, 190, 205 + 40*selection, NULL);
        rdpq_textparms_t parms; parms.align = ALIGN_CENTER; parms.valign = VALIGN_CENTER; parms.width = display_get_width(); parms.height = 40; parms.style_id = 1;
        rdpq_text_printf(&parms, 4, 0, 200, dictstr("match_pause_continue"));
        rdpq_text_printf(&parms, 4, 0, 240, dictstr("match_pause_exitmatch"));
        rdpq_textparms_t parms2; parms2.style_id = 2;
        rdpq_text_printf(&parms2, 3, 490, 445, dictstr("match_s_select")); 
        rdpq_detach_show();
      }
      sprite_free(selector_spr);
      sprite_free(button_a);
      if(quit) rspq_wait();
      return !quit;
}

bool game_update(){
    frame++;
    rdpq_sync_pipe();
    audioutils_mixer_update();

    if(matchinfo.state == MATCH_PLAY){
      int inttime = matchinfo.matchtimeleft;
      globaltime += display_get_delta_time();
      matchinfo.matchtimeleft -= display_get_delta_time();
      if((int) matchinfo.matchtimeleft != inttime) {rspq_wait(); rspq_block_free(dplDrawHUD); dplDrawHUD = NULL;}
    }
    joypad_poll();

    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if(matchinfo.demomatch && (pressed.raw != 0 || matchinfo.matchtimeleft < 0.5f)) return false;
    if(pressed.start && (matchinfo.state == MATCH_PAUSE || matchinfo.state == MATCH_PLAY)) {
      matchinfo.state = MATCH_PAUSE;
      bool cont = game_pause();
      if (!cont) return false;
    }

    playball_update();
    effects_update();
    carplayer_update();
    if(current_map->particles.updatefunc) current_map->particles.updatefunc(mapparticles, current_map->particles.count);
    TPE_worldStep(&world);
    // ======== Update ======== //
    T3DVec3 camp = {0};
    if(matchinfo.state == MATCH_SCORE){
      for(int i = 0; i < 4; i++){t3d_vec3_add(&camp, &camp, &carplayer[i].Pos_t3d);} t3d_vec3_scale(&camp, &camp, 0.25f);
      float disttoball = t3d_vec3_distance(&camp, &playball.ballPos_t3d);
      T3DVec3 camp_offset = {.x = 0,.y = 10 + playball.ballPos_t3d.y,.z = 15 + disttoball};
      t3d_vec3_add(&camp, &camp, &camp_offset);
    } else{
      float maxdist = 0;
      for(int i = 0; i < 4; i++){float d = t3d_vec3_distance2(&playball.ballPos_t3d, &carplayer[i].Pos_t3d); if (d > maxdist) maxdist = d;}
      maxdist = sqrt(maxdist);
      T3DVec3 camp_offset = {.x = playball.ballPos_t3d.x / 2,.y = 10 + playball.ballPos_t3d.y, .z = 10 + playball.ballPos_t3d.z / 2 + maxdist / 2};
      t3d_vec3_add(&camp, &camp, &camp_offset);
    }
      maincamera.camPos = camp;
      t3d_vec3_scale(&maincamera.camPos, &maincamera.camPos, 8);
      maincamera.camTarget = playball.ballPos_t3d;
      t3d_vec3_scale(&maincamera.camTarget, &maincamera.camTarget, 8);
      return true;
}

void game_intro(){
  bool pressed_start = false;
  float camlocationtimer = 0;
  int camidx = 0;
  while(!pressed_start){
    camlocationtimer += display_get_delta_time() / 7;
    if(camlocationtimer >= 1) {camidx++; camlocationtimer = 0;}

    game_update();

    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if(pressed.start) pressed_start = true;

    camidx = camidx % 5;
    T3DVec3 campos; t3d_vec3_lerp(&campos, (T3DVec3*)&camlocations[camidx][0], (T3DVec3*)&camlocations[camidx][1], camlocationtimer);
    T3DVec3 camrot; t3d_vec3_lerp(&camrot, (T3DVec3*)&camlocations[camidx][2], (T3DVec3*)&camlocations[camidx][3], camlocationtimer);

    t3d_vec3_scale(&campos, &campos, 8);
    t3d_vec3_scale(&camrot, &camrot, 8);

    maincamera.camPos = campos;
    maincamera.camTarget = camrot;

    maincamera.camPos_off = campos;
    maincamera.camTarget_off = camrot;


    rdpq_attach(display_get(), NULL);
    rdpq_set_scissor(0,0,display_get_width(),display_get_height(), display_get_rdpinterlace(), display_get_rdpfield());
    game_draw();
    rdpq_textparms_t parms; parms.align = ALIGN_RIGHT; parms.width = display_get_width(); parms.style_id = 1;
    rdpq_text_printf(&parms, 2, -40, display_get_height() - 45, current_map->name);
    rdpq_detach_show();

  }
  sound_play("LetsGo", false);
}

void game_countdown(){
  playball_reset();
  for(int i = 0; i < 4; i++) carplayer_reset(&carplayer[i], i);
  sprite_t* numbers = sprite_load("rom:/textures/ui/numbers.rgba32.sprite");
  int numbersize = numbers->width / 4;

  matchinfo.countdown = 3;
  sound_play("countdown", false);
  while(matchinfo.countdown > 0){
    matchinfo.countdown -= display_get_delta_time();

    game_update();

    rdpq_attach(display_get(), NULL);
    rdpq_set_scissor(0,0,display_get_width(),display_get_height(), display_get_rdpinterlace(), display_get_rdpfield());
    game_draw();
    int timerint = matchinfo.countdown;
    rdpq_blitparms_t parms;
    parms.s0 = timerint*numbersize;
    parms.t0 = 0;
    parms.width = numbersize;
    parms.height = numbersize;
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(numbers, (display_get_width() - numbersize) / 2, (display_get_height() - numbersize) / 2, &parms);
    rdpq_detach_show();
  }
  sprite_free(numbers);
}


bool game_play(){
  matchinfo.state = MATCH_PLAY;
  bool finished = false;
  while(!finished){
    if(!game_update()) return false;

    if(playball.ballPos_t3d.y < 3){
      if(playball.ballPos_t3d.x > 18.5f){
        finished = true;
        matchinfo.tleft.score++;
      } else if(playball.ballPos_t3d.x < -18.5f){
        finished = true;
        matchinfo.tright.score++;
      }
    } if(matchinfo.matchtimeleft <= 0.5f){
      finished = true;
    }

    rdpq_attach(display_get(), NULL);
    rdpq_set_scissor(0,0,display_get_width(),display_get_height(), display_get_rdpinterlace(), display_get_rdpfield());
    game_draw();
    rdpq_detach_show();
    
  }
  {rspq_wait(); rspq_block_free(dplDrawHUD); dplDrawHUD = NULL;}
  sprite_t* message = NULL;
  if(matchinfo.matchtimeleft <= 0.5f) {
    matchinfo.state = MATCH_END;
    sound_play("airhorn", false);
  }
  else {
    matchinfo.state = MATCH_SCORE;
    effects_add_exp3d(playball.ballPos_t3d, RGBA32(255,255,255,255));
    message = sprite_load("rom:/textures/ui/goal.rgba32.sprite");
    playball.init = false;
    effects_add_shake(2.5f);
    for(int i = 0; i < 4; i++) effects_add_rumble(i, 1.0f);
    TPE_Vec3 force  = {.x = playball.ballPos_t3d.x > 0? -0.5 * TPE_F : 0.5 * TPE_F, 0.5 * TPE_F, 0};
    for(int i = 0; i < 4; i++) TPE_bodyAccelerate(&bodies[carplayer[i].bodyidx], force);
  }
  float countdowntimer = 3;
  while(countdowntimer > 0){
    countdowntimer -= display_get_delta_time();
    game_update();
    rdpq_attach(display_get(), NULL);
    rdpq_set_scissor(0,0,display_get_width(),display_get_height(), display_get_rdpinterlace(), display_get_rdpfield());
    game_draw();
    if(message) {
      float scale = ((4 - countdowntimer) / 2);
      rdpq_blitparms_t parms; parms.cx = message->width / 2; parms.cy = message->height / 2; 
      parms.scale_x = scale; parms.scale_y = scale; parms.filtering = true;
      rdpq_set_mode_standard();
      rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
      rdpq_mode_filter(FILTER_BILINEAR);
      rdpq_sprite_blit(message, display_get_width() / 2, display_get_height() / 2 , &parms);
    }
    rdpq_detach_show();
  }
  if(message) {sprite_free(message);} message = NULL;
  return true;
}

void game_score(){
  float countdowntimer = 15;
  bool pressed_a = false;
  sprite_t* button_a = sprite_load("rom:/textures/ui/button_a.rgba32.sprite");
  rspq_block_t* scoreblock = NULL;
  while(countdowntimer > 0 && !pressed_a){
    countdowntimer -= display_get_delta_time();
    game_update();
    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    if(pressed.a) pressed_a = true;
    rdpq_attach(display_get(), NULL);
    rdpq_set_scissor(0,0,display_get_width(),display_get_height(), display_get_rdpinterlace(), display_get_rdpfield());
    game_draw();
    if(!scoreblock){
      rspq_block_begin();
      rdpq_set_mode_standard();
      rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
      rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
      rdpq_mode_antialias(AA_REDUCED);
      
      for(int i = 0; i < 5; i++){
        int y = (i * 40) + 130;
        rdpq_set_prim_color(i % 2? RGBA32(0,20,20,128) : RGBA32(0,0,0,128));
        rdpq_fill_rectangle(45, y, display_get_width() - 45, y + 40);
      }

      rdpq_mode_combiner(RDPQ_COMBINER_TEX);
      rdpq_sprite_blit(button_a, 450, 425, NULL);
      rdpq_textparms_t parms; parms.align = ALIGN_CENTER; parms.valign = VALIGN_CENTER; parms.width = display_get_width(); parms.height = 40; parms.style_id = 1;
      float left_possession = (matchinfo.tleft.posession / (matchinfo.tleft.posession + matchinfo.tright.posession)) * 100;
      float right_possession = (matchinfo.tright.posession / (matchinfo.tleft.posession + matchinfo.tright.posession)) * 100;
      rdpq_text_printf(&parms, 4, 0, 130, dictstr("match_s_stats"));
      rdpq_text_printf(&parms, 4, -150, 170, "%i", matchinfo.tleft.score); rdpq_text_printf(&parms, 4, 150, 170, "%i", matchinfo.tright.score);
      rdpq_text_printf(&parms, 4, 0, 170, "%s", dictstr("match_s_goals"));
      rdpq_text_printf(&parms, 4, -150, 210, "%i", matchinfo.tleft.shots); rdpq_text_printf(&parms, 4, 150, 210, "%i", matchinfo.tright.shots);
      rdpq_text_printf(&parms, 4, 0, 210, "%s", dictstr("match_s_shots"));
      rdpq_text_printf(&parms, 4, -150, 250, "%i", matchinfo.tleft.shotsontarget); rdpq_text_printf(&parms, 4, 150, 250, "%i", matchinfo.tright.shotsontarget);
      rdpq_text_printf(&parms, 4, 0, 250, "%s", dictstr("match_s_shotsont"));
      rdpq_text_printf(&parms, 4, -150, 290, "%.0f%%", left_possession); rdpq_text_printf(&parms, 4, 150, 290, "%.0f%%", right_possession);
      rdpq_text_printf(&parms, 4, 0, 290, "%s",dictstr("match_s_posession"));
      rdpq_textparms_t parms2; parms2.style_id = 2;
      rdpq_text_printf(&parms2, 3, 490, 445, dictstr("match_s_continue")); 
      scoreblock = rspq_block_end();
    } rspq_block_run(scoreblock);
    rdpq_detach_show();
  }
  rspq_wait();
  sprite_free(button_a);
  rspq_block_free(scoreblock);
}

void game_free(){
  rspq_wait();
  playball_free();
  for(int i = 0; i < MAXPLAYERS; i++) {carplayer_free(&carplayer[i]);}

  memset(bodies, 0, sizeof(bodies));
  bodiescount = 0;
  
  current_map = NULL;
  memset(viewport, 0, sizeof(viewport));

  if(model) {t3d_model_free(model); model = NULL;}
  memset(viewport, 0, sizeof(modelMat));
  if(modelMatFP) {free_uncached(modelMatFP); modelMatFP = NULL;}

  memset(offset, 0, sizeof(offset));
  memset(&matstate, 0, sizeof(matstate));
  memset(&world, 0, sizeof(TPE_World));

  if(dplDraw) {rspq_block_free(dplDraw); dplDraw = NULL;}
  if(dplDrawB) {rspq_block_free(dplDrawB); dplDrawB = NULL;}
  if(dplDrawHUD) {rspq_block_free(dplDrawHUD); dplDrawHUD = NULL;}

  if(mapparticles) {free_uncached(mapparticles); mapparticles = NULL;}
  if(mapparticles_sprite) {sprite_free(mapparticles_sprite); mapparticles_sprite = NULL;}
  if(particleMatFP) {free_uncached(particleMatFP); particleMatFP = NULL;}

  memset(&maincamera, 0, sizeof(maincamera));
}

void game_start_demo(mapinfo_t* map){
  matchinfo.demomatch = true;
  game_start(map);
}

void game_start(mapinfo_t* map)
{
  switch(gamestatus.state.game.settings.graphics){
    case FASTEST: vi_set_aa_mode(VI_AA_MODE_NONE);                  vi_set_dedither(false); break;
    case DEFAULT: vi_set_aa_mode(VI_AA_MODE_RESAMPLE_FETCH_ALWAYS); vi_set_dedither(false); break;
    case NICEST:  vi_set_aa_mode(VI_AA_MODE_RESAMPLE_FETCH_ALWAYS); vi_set_dedither(true);break;
  }

  current_map = map;
  border_time_color = RGBA32(255,217,0,255);
  border_time_score = RGBA32(0,84,255,255);
  for(int i = 0; i < 6; i++)
    viewport[i] = t3d_viewport_create();
  exposure = 5;
  exposure_bias = current_map->hdr.tonemappingaverage;

  t3d_mat4_identity(&modelMat);
  
  // Now allocate a fixed-point matrix, this is what t3d uses internally.
  // Note: this gets DMA'd to the RSP, so it needs to be uncached.
  // If you can't allocate uncached memory, remember to flush the cache after writing to it instead.
  modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
  if(current_map->particles.count){
    particleMatFP = malloc_uncached(sizeof(T3DMat4FP));
    uint32_t allocSize = sizeof(TPXParticle) * current_map->particles.count / 2;
    mapparticles = malloc_uncached(allocSize);
    if(current_map->particles.initfunc) current_map->particles.initfunc(mapparticles, current_map->particles.count);
    mapparticles_sprite = sprite_load(current_map->particles.texturefn);
  }

  // Load a model-file, this contains the geometry and some metadata
  model = t3d_model_load(map->modelfn);
  bgm_play(map->musicfn, true, 1);

  float offset[2] = {0};
  matstate = t3d_model_state_create();

  matchinfo.matchtimeleft = (gamestatus.state.game.settings.duration) * 60;
  matchinfo.state = MATCH_INTRO;
  for(int i = 0; i < 4; i++) carplayer_init(&carplayer[i], i);
  playball_init();
  TPE_worldInit(&world,bodies,bodiescount,environmentDistance);
  world.collisionCallback = entityCollisionCallback;

  if(matchinfo.demomatch){
    matchinfo.matchtimeleft = 60;
    bool contmatch = true;
    while(matchinfo.matchtimeleft > 0.5 && contmatch){
      game_countdown();
      if(!game_play()) contmatch = false;
    }
    vi_set_dedither(true); vi_set_aa_mode(VI_AA_MODE_NONE);
    game_free();
    return;
  }

  game_intro(); 

  #if !DEBUG_RDP
  while (matchinfo.matchtimeleft > 0.5f)
  #endif
  {
    game_countdown();
    if(!game_play()) goto match_end;
    // ======== Draw ======== //
  }
  game_score();
match_end:
  vi_set_dedither(true); vi_set_aa_mode(VI_AA_MODE_NONE);
  game_free();
}
