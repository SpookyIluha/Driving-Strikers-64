#ifndef PLAYTIME_LOGIC_H
#define PLAYTIME_LOGIC_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "engine_gfx.h"
#include "teams.h"
#include "entity_logic.h"
#include "audioutils.h"
#include "maps.h"

#define DEBUG_RDP 0

typedef enum{
  MATCH_INTRO,
  MATCH_COUNTDOWN,
  MATCH_PLAY,
  MATCH_PAUSE,
  MATCH_SCORE,
  MATCH_END
} matchstate_t;

typedef struct{
  teamstats_t tleft, tright;
  float matchtimeleft;
  matchstate_t state;
  float countdown;
  int playercontrollers[4];
  bool exited;
  bool demomatch;
} match_t;

extern match_t matchinfo;

void matchinfo_init();

void game_start_demo(mapinfo_t* map);

void game_start(mapinfo_t* map);

#endif