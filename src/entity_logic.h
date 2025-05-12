#ifndef ENTITY_LOGIC_H
#define ENTITY_LOGIC_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/tpx.h>
#include "engine_gfx.h"
#include "teams.h"
#include "tinyphysicsengine.h"

typedef struct{
    //TPE_Body body;
    int bodyidx;
    TPE_Joint joint;
  
    T3DVec3 ballPos_t3d;
    T3DVec3 ballPoslast;
    T3DQuat ballquat;
    TPE_Vec3 ballRot, ballPreviousPos;
  
    T3DModel *modelball;
    T3DMat4 modelMatball; // matrix for our model, this is a "normal" float matrix
    T3DMat4 modelshadowMat; // matrix for our model, this is a "normal" float matrix
    T3DMat4FP* modelMatFPball;
  
    T3DModel *modelshadow;
    T3DMat4FP* modelMatFPshadow;

    rspq_block_t* blockmodel;
    rspq_block_t* blockshadow;
    bool init;
  } playball_t;
  extern playball_t playball;
  
  void playball_init();
  
  void playball_reset();

  void playball_update();
  
  void playball_draw();

  void playball_draw_shadow();

  void playball_free();
  
extern TPE_Body bodies[64];
extern int bodiescount;
#define PARTICLES_MAX 16
  
  typedef struct{
    teamdef_t* team;
    bool is_team_left;
    int playercontroller;
    //TPE_Body body;
    int bodyidx;
    TPE_Joint joints[2];
    TPE_Connection connections[1];
  
    T3DVec3 Pos_t3d;
    T3DVec3 Poslast;
    T3DVec3 forward;
    T3DVec3 ai_targetpos;
    T3DQuat quat;
    TPE_Vec3 Rot, PreviousPos, playerDirectionVec;

    TPXParticle *particles;
    sprite_t *part_firespr;
    sprite_t *part_glowspr;
    int particle_type;

    float yaw;
    bool ballcollided;
    int  carcollided;
    float nitro;
    bool isnitro;
    float ai_difficulty;
    float ai_maxturnspeed;
  
    T3DModel *model;
    T3DModel *debugjointmodel;
    T3DModel *shadowmodel;
    T3DModel *nitromodel;
    T3DMat4 modelMat; // matrix for our model, this is a "normal" float matrix
    T3DMat4 shadowmodelMat; // matrix for our model, this is a "normal" float matrix
    T3DMat4FP* modelMatFP;
    T3DMat4FP* debugjointmodelMatFP;
    T3DMat4FP* shadowmodelMatFP;
    rspq_block_t* blockmodel;
    rspq_block_t* blockshadow;
  } carplayer_t;
 extern carplayer_t carplayer[4];
  
  void carplayer_init(carplayer_t* car, int index);

  void carplayer_reset(carplayer_t* car, int index);
  
  void carplayer_update();

  void carplayer_free(carplayer_t* car);
  
  void carplayer_draw(carplayer_t* car);
  
  void carplayer_draw_shadow(carplayer_t* car);

  void carplayer_draw_teleport_particles(carplayer_t* car);

  #endif