#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/tpx.h>
#include "engine_gfx.h"
#include "entity_logic.h"
#include "teams.h"
#include "playtime_logic.h"
#include "particles_sim.h"

TPE_Body bodies[64] = {0};
int bodiescount = 0;

playball_t playball;
carplayer_t carplayer[4];

uint32_t playercolors[4] = {
  0xff1e1eFF,
  0x1eff1eFF,
  0x1e1effFF,
  0xffff1eFF
};

  void playball_init(){
    if(!playball.modelball) playball.modelball = t3d_model_load("rom:/models/ball.t3dm");
    if(!playball.modelshadow) playball.modelshadow = t3d_model_load("rom:/models/ball_shadow.t3dm");
    t3d_mat4_identity(&playball.modelMatball);
    t3d_mat4_identity(&playball.modelshadowMat);
    if(!playball.modelMatFPball) playball.modelMatFPball = malloc_uncached(sizeof(T3DMat4FP)*6);
    if(!playball.modelMatFPshadow) playball.modelMatFPshadow = malloc_uncached(sizeof(T3DMat4FP)*6);
    playball.ballPoslast = (T3DVec3){.x = 0,.y = TPE_F * 8,.z = 0};
    fm_quat_identity(&playball.ballquat);
    playball.joint = TPE_joint((TPE_Vec3){0,TPE_F * 8,0},TPE_F / 1.3);
    playball.bodyidx = bodiescount; bodiescount++;
    TPE_bodyInit(&bodies[playball.bodyidx],&playball.joint,1,0,0, TPE_F / 2);  
    bodies[playball.bodyidx].friction = TPE_F / 25; // decrease friction for fun
    bodies[playball.bodyidx].elasticity =  TPE_F / 3;
  }

  void playball_free(){
    if(playball.modelball) t3d_model_free(playball.modelball);
    if(playball.modelshadow) t3d_model_free(playball.modelshadow);
    if(playball.modelMatFPball) free_uncached(playball.modelMatFPball);
    if(playball.modelMatFPshadow) free_uncached(playball.modelMatFPshadow);
    if(playball.blockmodel) rspq_block_free(playball.blockmodel);
    if(playball.blockshadow) rspq_block_free(playball.blockshadow);
    memset(&playball, 0, sizeof(playball));
  }

  void playball_reset(){
    playball.init = true;
    t3d_mat4_identity(&playball.modelMatball);
    t3d_mat4_identity(&playball.modelshadowMat);
    playball.ballPoslast = (T3DVec3){.x = 0,.y = TPE_F * 8,.z = 0};
    fm_quat_identity(&playball.ballquat);
    playball.joint = TPE_joint((TPE_Vec3){0,TPE_F * 8,0},TPE_F / 1.5);
    TPE_bodyInit(&bodies[playball.bodyidx],&playball.joint,1,0,0, TPE_F / 2);  
    bodies[playball.bodyidx].friction = TPE_F / 25; // decrease friction for fun
    bodies[playball.bodyidx].elasticity =  TPE_F / 3;
  }
  
  void playball_update(){
    if(!playball.init) return;
  
    if(TPE_bodyIsActive(&bodies[playball.bodyidx]))
      TPE_bodyApplyGravity(&bodies[playball.bodyidx],TPE_F / 100);
    
    TPE_Vec3 ballPos = bodies[playball.bodyidx].joints[0].position;
    playball.ballPos_t3d = (T3DVec3){.x = ballPos.x * (1.0/TPE_FRACTIONS_PER_UNIT), .y = ballPos.y * (1.0/TPE_FRACTIONS_PER_UNIT), .z = ballPos.z * (1.0/TPE_FRACTIONS_PER_UNIT)};
  
    // ======== Update ======== // 
    {
      T3DVec3 diff; t3d_vec3_diff(&diff, &playball.ballPoslast, &playball.ballPos_t3d);
      diff.y = 0;
      float distance = t3d_vec3_len(&diff);
      if(distance > 0){
        t3d_vec3_norm(&diff);
        T3DVec3 diff_cross; t3d_vec3_cross(&diff_cross, &diff, &(T3DVec3){.x = 0,.y = 1,.z = 0});
        diff_cross.y = 0;
        t3d_vec3_norm(&diff_cross);
        t3d_quat_rotate_euler(&playball.ballquat, (float*)&diff_cross, distance);
        t3d_quat_normalize(&playball.ballquat);
      }
    }    playball.ballPoslast = playball.ballPos_t3d;
    //t3d_quat_rotate_euler(&quat, (float[3]){0,0,1}, -ballPost3d.x);
    //t3d_quat_rotate_euler(&quat, (float[3]){1,0,0}, ballPost3d.z);
    //t3d_quat_rotate_euler(&quat, (float[3]){0,1,0}, 0);
  
    t3d_mat4_from_srt(&playball.modelMatball,
      (float[3]){1, 1, 1},
      (float*)&playball.ballquat,
      (float[3]){playball.ballPos_t3d.x* 64, (playball.ballPos_t3d.y + 0.3)* 64, playball.ballPos_t3d.z* 64}
    );
    t3d_mat4_from_srt_euler(&playball.modelshadowMat,
      (float[3]){1, 1, 1},
      (float[3]){0, 0, 0},
      (float[3]){playball.ballPos_t3d.x* 64, 0, playball.ballPos_t3d.z* 64}
    );
    t3d_mat4_to_fixed(&playball.modelMatFPball[(frame) % 6], &playball.modelMatball);
    t3d_mat4_to_fixed(&playball.modelMatFPshadow[(frame) % 6], &playball.modelshadowMat);
  }
  
  void playball_draw(){
    if(!playball.init) return;
    t3d_matrix_set(&playball.modelMatFPshadow[(frame) % 6], true); 
    if(!playball.blockshadow){
      rspq_block_begin();
      t3d_model_draw(playball.modelshadow);
      playball.blockshadow = rspq_block_end();
    } rspq_block_run(playball.blockshadow);
    t3d_matrix_set(&playball.modelMatFPball[(frame) % 6], true); 
    if(!playball.blockmodel){
      rspq_block_begin();
      t3d_model_draw(playball.modelball);
      rdpq_sync_pipe();
      playball.blockmodel = rspq_block_end();
    } rspq_block_run(playball.blockmodel);
    //t3d_matrix_pop(1);
  }
  
  void playball_draw_shadow(){
    if(!playball.init) return;
    t3d_matrix_set(&playball.modelMatFPshadow[(frame) % 6], true); 
    if(!playball.blockshadow){
      rspq_block_begin();
      t3d_model_draw(playball.modelshadow);
      playball.blockshadow = rspq_block_end();
    } rspq_block_run(playball.blockshadow);
    //t3d_matrix_pop(1);
  }

  /*void carplayer_particles(carplayer_t* car){
    switch(car->particle_type)
    {
      case 2: // Flame
        time += deltaTime * 1.0f;
        float posX = car->Pos_t3d.x;
        float posZ = car->Pos_t3d.z;

        simulate_particles_fire(car->particles, PARTICLES_MAX, posX, posZ);
        particleMatScale = (T3DVec3){{0.9f, partMatScaleVal, 0.9f}};
        particlePos.y = partMatScaleVal * 130.0f;
        isSpriteRot = true;
      break;
      case 1: // Random
        generate_particles_random(car->particles, PARTICLES_MAX);
      break;
      default: break;
    }
  }*/
  
  void carplayer_init(carplayer_t* car, int index){
    car->team = index < 2? matchinfo.tleft.team : matchinfo.tright.team;
    car->is_team_left = index < 2;
    car->playercontroller = matchinfo.playercontrollers[index];
    car->ai_maxturnspeed = car->team->maxturnspeed + frandr(-0.25, 0.1);
    car->ai_difficulty = car->team->difficulty + frandr(-0.025, 0.03);
    if(!car->model) car->model = t3d_model_load(car->team->modelfilename);
    if(!car->debugjointmodel) car->debugjointmodel = t3d_model_load("rom:/models/celica/debugball.t3dm");
    if(!car->shadowmodel) car->shadowmodel = t3d_model_load("rom:/models/celica/celica_shadow.t3dm");
    if(!car->nitromodel) car->nitromodel = t3d_model_load("rom:/models/celica/celica_nitro.t3dm");
    t3d_mat4_identity(&car->modelMat);
    t3d_mat4_identity(&car->shadowmodelMat);
    if(!car->modelMatFP) car->modelMatFP = malloc_uncached(sizeof(T3DMat4FP)*6);
    if(!car->shadowmodelMatFP) car->shadowmodelMatFP = malloc_uncached(sizeof(T3DMat4FP)*6);
    if(!car->particles) {
      uint32_t allocSize = sizeof(TPXParticle) * PARTICLES_MAX / 2;
      car->particles = malloc_uncached(allocSize);
    }
    if(!car->part_firespr) car->part_firespr = sprite_load("rom:/textures/parts/fire.i8.sprite");
    if(!car->part_glowspr) car->part_glowspr = sprite_load("rom:/textures/parts/flare.i8.sprite");
    //carplayer.debugjointmodelMatFP = malloc_uncached(sizeof(T3DMat4FP)*4);
    T3DVec3 startpos;
    startpos.x = index < 2? -12 : 12;
    startpos.z = index % 2? 7 : -7;
  
    car->Poslast = (T3DVec3){.x = TPE_F * (startpos.x),.y = TPE_F * 0.5,.z = TPE_F * (startpos.z)};
    fm_quat_identity(&car->quat);
    car->joints[0] = TPE_joint((TPE_Vec3){TPE_F * (startpos.x),TPE_F * 0.5, TPE_F * (startpos.z)}, TPE_F / 1.5);
    car->joints[1] = TPE_joint((TPE_Vec3){TPE_F * (index < 2? startpos.x + 2 : startpos.x - 2),TPE_F * 0.5, TPE_F * (startpos.z) * 0.85f}, TPE_F / 1.5);
    car->connections[0].joint1 = 0;
    car->connections[0].joint2 = 1;
    car->connections[0].length = TPE_F * 1;
    car->bodyidx = bodiescount; bodiescount++;
    TPE_bodyInit(&bodies[car->bodyidx], car->joints,2,car->connections,1,1 * TPE_F); 
    bodies[car->bodyidx].friction = TPE_F / 10;
    bodies[car->bodyidx].elasticity =  TPE_F / 10;
  }
  
  void carplayer_free(carplayer_t* car){
    if(car->model) t3d_model_free(car->model);
    if(car->debugjointmodel) t3d_model_free(car->debugjointmodel);
    if(car->shadowmodel) t3d_model_free(car->shadowmodel);
    if(car->nitromodel) t3d_model_free(car->nitromodel);
    if(car->modelMatFP) free_uncached(car->modelMatFP);
    if(car->shadowmodelMatFP) free_uncached(car->shadowmodelMatFP);
    if(car->debugjointmodelMatFP) free_uncached(car->debugjointmodelMatFP);
    if(car->particles) free_uncached(car->particles);
    if(car->part_firespr)  sprite_free(car->part_firespr);
    if(car->part_glowspr) sprite_free(car->part_glowspr);
    if(car->blockmodel) rspq_block_free(car->blockmodel);
    if(car->blockshadow) rspq_block_free(car->blockshadow);
    memset(car, 0, sizeof(carplayer_t));
  }

  void carplayer_reset(carplayer_t* car, int index){
    t3d_mat4_identity(&car->modelMat);
    t3d_mat4_identity(&car->shadowmodelMat);
    T3DVec3 startpos;
    startpos.x = index < 2? -12 : 12;
    startpos.z = index % 2? 7 : -7;

    car->ballcollided = false;
    car->carcollided = -1;
    
    car->ai_maxturnspeed = car->team->maxturnspeed + frandr(-0.25, 0.1);
    car->ai_difficulty = car->team->difficulty + frandr(-0.02, 0.03);
  
    car->Poslast = (T3DVec3){.x = TPE_F * (startpos.x),.y = TPE_F * 0.5,.z = TPE_F * (startpos.z)};
    fm_quat_identity(&car->quat);
    car->joints[0] = TPE_joint((TPE_Vec3){TPE_F * (startpos.x),TPE_F * 0.5, TPE_F * (startpos.z)}, TPE_F / 1.5);
    car->joints[1] = TPE_joint((TPE_Vec3){TPE_F * (index < 2? startpos.x + 2 : startpos.x - 2),TPE_F * 0.5, TPE_F * (startpos.z) * 0.85f}, TPE_F / 1.5);
    TPE_bodyInit(&bodies[car->bodyidx], car->joints,2,car->connections,1,1 * TPE_F); 
    bodies[car->bodyidx].friction = TPE_F / 10;
    bodies[car->bodyidx].elasticity =  TPE_F / 10;
    generate_particles_random(car->particles, PARTICLES_MAX);
  }

  void carplayer_ai(int index, joypad_inputs_t* outinput, joypad_buttons_t* outpressed){
    T3DVec3 carpos = carplayer[index].Pos_t3d;
    t3d_vec3_lerp(&carplayer[index].ai_targetpos, &carplayer[index].ai_targetpos, &playball.ballPos_t3d, carplayer[index].ai_difficulty);
    T3DVec3 target = carplayer[index].ai_targetpos;
    T3DVec3 goal =  (T3DVec3){.x = -19, .y = 0, .z = 0};
    float dist = t3d_vec3_distance(&carpos, &target);
    bool can_strike = false;
    // set target vectors
    if(carplayer[index].is_team_left){
      goal.x = -goal.x;
      can_strike = carpos.x < target.x;
    } else can_strike = carpos.x >= target.x;
    if(can_strike){ // target the ball
      T3DVec3 balloffset; t3d_vec3_diff(&balloffset, &target, &goal); t3d_vec3_norm(&balloffset);
      t3d_vec3_add(&target, &target, &balloffset);
    } else { // drive around the ball
      T3DVec3 balloffset = (T3DVec3){.x = 0, .y = 0, .z = 4};
      if(carpos.z < target.z) balloffset.z = -balloffset.z;
      t3d_vec3_add(&target, &target, &balloffset);
    } // target to inputs
    if(dist > 2) {
      if(target.y - 1 > carpos.y) {outpressed->l = 1;} // jump if the target is high
      outinput->btn.z = 1; // nitro if the target is far enough
    }
    T3DVec3 forw; t3d_vec3_diff(&forw, &target, &carpos); t3d_vec3_norm(&forw);
    outinput->stick_x = forw.x * 68;
    outinput->stick_y = -forw.z * 68;
  }

  void carplayer_update(){
    for(int i = 0; i < 4; i++){
    if(TPE_bodyIsActive(&bodies[carplayer[i].bodyidx]))
      TPE_bodyApplyGravity(&bodies[carplayer[i].bodyidx],TPE_F / 100);
    }

    for(int i = 0; i < 4; i++){
      TPE_Vec3 Pos = TPE_bodyGetCenterOfMass(&bodies[carplayer[i].bodyidx]);
      carplayer[i].Pos_t3d = (T3DVec3){.x = Pos.x * (1.0/TPE_F), .y = Pos.y * (1.0/TPE_F), .z = Pos.z * (1.0/TPE_F)};
    
      carplayer[i].Rot = TPE_bodyGetRotation(&bodies[carplayer[i].bodyidx], 0, 1, 3);
      TPE_Vec3 diff = TPE_vec3Subtract(carplayer[i].joints[1].position, carplayer[i].joints[0].position);
      TPE_vec3Normalize(&diff);
      carplayer[i].forward = (T3DVec3){.x = diff.x, .y = diff.y, .z = diff.z};
      t3d_vec3_norm(&carplayer[i].forward);
    }

    for(int i = 0; i < 4; i++){
      carplayer[i].yaw = atan2f(carplayer[i].forward.z, carplayer[i].forward.x);
      TPE_Vec3 diff = TPE_vec3Subtract(carplayer[i].joints[1].position, carplayer[i].joints[0].position);
      // handle if there was a collision
      if(carplayer[i].carcollided >= 0)
        if(t3d_vec3_distance2(&carplayer[i].Pos_t3d, &carplayer[carplayer[i].carcollided].Pos_t3d) > 16) carplayer[i].carcollided = -1;  
      
      if(carplayer[i].ballcollided)
        if(t3d_vec3_distance2(&carplayer[i].Pos_t3d, &playball.ballPos_t3d) > 8) carplayer[i].ballcollided = false;  
    
      // apply back axel acceleration
      joypad_inputs_t input;
      joypad_buttons_t pressed;
      if(carplayer[i].playercontroller < 0) {carplayer_ai(i, &input, &pressed);}
      else{
        input = joypad_get_inputs(carplayer[i].playercontroller);
        pressed = joypad_get_buttons_pressed(carplayer[i].playercontroller); }
      if(matchinfo.state != MATCH_PLAY) {
        memset(&input, 0, sizeof(joypad_inputs_t));
        memset(&pressed, 0, sizeof(joypad_buttons_t));}

      carplayer[i].isnitro = false;
      if(input.btn.z) carplayer[i].nitro -= display_get_delta_time();
      else carplayer[i].nitro += display_get_delta_time() / 2;
      if(pressed.z && carplayer[i].nitro > 0) sound_play("boost", false);
      if(carplayer[i].nitro < 0) carplayer[i].nitro = 0;
      if(carplayer[i].nitro > 1) carplayer[i].nitro = 1;
      if(input.btn.z && carplayer[i].nitro > 0) carplayer[i].isnitro = true;

      float inputyaw = 0;
      float inputvel = 0;
      fm_vec3_t vec; vec.x = input.stick_x; vec.y = input.stick_y; 
      if(!gfx_pos_within_rect(vec.x, vec.y, -10, -10, 10, 10)){
        inputyaw = fm_atan2f(vec.y, -vec.x);
        inputvel = fm_vec3_len(&vec);
        float a = carplayer[i].yaw - inputyaw;
        if (a > FM_PI) a -= FM_PI*2;
        if (a < -FM_PI) a += FM_PI*2;
        inputyaw = a * 180 / FM_PI;
        if(abs(inputyaw) < 5.0f) inputyaw = 0;
      }
      {
        TPE_Vec3 vel = TPE_bodyGetLinearVelocity(&bodies[carplayer[i].bodyidx]);
        int vellen = TPE_vec3Len(vel);
        int forwardaccel = inputvel; 
        if(forwardaccel < gamestatus.state.game.settings.deadzone * 65) forwardaccel = 0;
        int siderotation = inputyaw;
        if(carplayer[i].isnitro) forwardaccel = 65;
        if(carplayer[i].joints[0].position.y < (TPE_F) + 20){
          if(pressed.l || pressed.r){
            TPE_Vec3 ax = {.x = 0, .y = TPE_F / 4, .z = 0};
            TPE_bodyAccelerate(&bodies[carplayer[i].bodyidx], ax);
          }
          if(vellen > 0){
            if(siderotation > 65) siderotation = 65; 
            if(siderotation < -65) siderotation = -65;
            int vellen_siderot = vellen; if(vellen_siderot > (TPE_F / 5)) vellen_siderot = (TPE_F / 5);
            siderotation = siderotation * vellen_siderot / TPE_F;
            if(carplayer[i].playercontroller < 0) siderotation *= carplayer[i].ai_maxturnspeed;
            TPE_bodyRotateByAxis(&bodies[carplayer[i].bodyidx], (TPE_Vec3){.y = siderotation});
          }
        }
        if((carplayer[i].isnitro && vellen < (TPE_F / 4)) ||  (forwardaccel > 0 && vellen < (TPE_F / 5)) || (forwardaccel < 0 && vellen < (TPE_F / 10))){
          if(forwardaccel > 65) forwardaccel = 65; 
          if(forwardaccel < -65) forwardaccel = -65;
          if(forwardaccel < 0) forwardaccel >>= 1;
          TPE_Vec3 ax = TPE_vec3Times(diff, carplayer[i].isnitro? forwardaccel : forwardaccel >> 2); 
          TPE_bodyAccelerate(&bodies[carplayer[i].bodyidx], ax);
        }
      }
    }
  
    for(int i = 0; i < 4; i++){
      t3d_mat4_from_srt_euler(&carplayer[i].modelMat, 
        (float[3]){1, 1, 1},
        (float[3]){0, carplayer[i].yaw, 0}, 
        (float[3]){carplayer[i].Pos_t3d.x * 64, carplayer[i].Pos_t3d.y * 64, carplayer[i].Pos_t3d.z * 64}
      );
      t3d_mat4_from_srt_euler(&carplayer[i].shadowmodelMat, 
        (float[3]){1, 1, 1},
        (float[3]){0, carplayer[i].yaw, 0}, 
        (float[3]){carplayer[i].Pos_t3d.x * 64, 0, carplayer[i].Pos_t3d.z * 64}
      );
      // ======== Update ======== //  
      t3d_mat4_to_fixed(&carplayer[i].modelMatFP[(frame) % 6], &carplayer[i].modelMat);
      t3d_mat4_to_fixed(&carplayer[i].shadowmodelMatFP[(frame) % 6], &carplayer[i].shadowmodelMat);
    }
  }
  
  void carplayer_draw(carplayer_t* car){
    t3d_matrix_set(&car->modelMatFP[(frame) % 6], true); 
    if(car->isnitro){
      rdpq_set_prim_color(car->playercontroller >= 0? color_from_packed32(playercolors[car->playercontroller]) : RGBA32(255,255,255,255));
      t3d_model_draw(car->nitromodel);
    }
    if(!car->blockmodel){
      rspq_block_begin();
      rdpq_set_prim_color(car->playercontroller >= 0? color_from_packed32(playercolors[car->playercontroller]) : RGBA32(255,255,255,255));
      t3d_model_draw(car->model);
      rdpq_sync_pipe();
      car->blockmodel = rspq_block_end();
    } rspq_block_run(car->blockmodel);
    //t3d_matrix_pop(1);

  }

  void carplayer_draw_teleport_particles(carplayer_t* car){
      t3d_matrix_set(&car->modelMatFP[(frame) % 6], true); 
      // Upload texture for the following particles.
      // The ucode itself never loads or switches any textures,
      // so you can only use what you have uploaded before in a single draw call.
      rdpq_texparms_t p = {0};
      p.s.repeats = REPEAT_INFINITE;
      p.t.repeats = REPEAT_INFINITE;
      p.s.scale_log = -2;
      p.t.scale_log = -2;
      rdpq_sprite_upload(TILE0, car->part_glowspr, &p);

      tpx_state_from_t3d();
      tpx_state_set_scale(0.45f, 0.45f);

      tpx_state_set_tex_params(0, 0);
      tpx_particle_draw_tex(car->particles, PARTICLES_MAX);
  }

  void carplayer_draw_shadow(carplayer_t* car){
    t3d_matrix_set(&car->shadowmodelMatFP[(frame) % 6], true); 
    if(!car->blockshadow){
      rspq_block_begin();
      t3d_model_draw(car->shadowmodel);
      car->blockshadow = rspq_block_end();
    } rspq_block_run(car->blockshadow);
    //t3d_matrix_pop(1);
}
