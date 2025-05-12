#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "engine_gfx.h"
#include "entity_logic.h"
#include "teams.h"
#include "playtime_logic.h"

teamdef_t teams[8] = {
{ .id = 0, .style = 0, .teamcolor = (0xFFFFFFFF), .difficulty = 0.13f, .maxturnspeed = 0.8f, .teamname = "FROZEN THUNDER",   .teamshortname = "FZT", .logofilename = "rom://textures/ui/teams/fronzen.rgba32.sprite",  .modelfilename = "rom://models/celica/celica_white.t3dm", .voicenamefn = "voice/Team_FrozenThunder",},
{ .id = 1, .style = 1, .teamcolor = (0xFF0000FF), .difficulty = 0.07f, .maxturnspeed = 0.55f,.teamname = "FLAMING WHEELS",   .teamshortname = "FLW", .logofilename = "rom://textures/ui/teams/flaming.rgba32.sprite",  .modelfilename = "rom://models/celica/celica_red.t3dm", .voicenamefn = "voice/Team_FlamingWheels",},
{ .id = 2, .style = 0, .teamcolor = (0x808080FF), .difficulty = 0.09f, .maxturnspeed = 0.6f, .teamname = "STEEL REBELS",     .teamshortname = "STR", .logofilename = "rom://textures/ui/teams/steel.rgba32.sprite",    .modelfilename = "rom://models/celica/celica_gray.t3dm", .voicenamefn = "voice/Team_SteelRebels",},
{ .id = 3, .style = 0, .teamcolor = (0xFF80FFFF), .difficulty = 0.07f, .maxturnspeed = 1.0f, .teamname = "LUCKY VIXENS",     .teamshortname = "LVX", .logofilename = "rom://textures/ui/teams/lucky.rgba32.sprite",    .modelfilename = "rom://models/celica/celica_pink.t3dm", .voicenamefn = "voice/Team_LuckyVixens",},
{ .id = 4, .style = 0, .teamcolor = (0x4287f5FF), .difficulty = 0.05f, .maxturnspeed = 0.4f, .teamname = "ELECTRIC SQUAD",   .teamshortname = "ESQ", .logofilename = "rom://textures/ui/teams/electric.rgba32.sprite", .modelfilename = "rom://models/celica/celica_yellow.t3dm", .voicenamefn = "voice/Team_ElectricSquad",},
{ .id = 5, .style = 1, .teamcolor = (0x000000FF), .difficulty = 0.1f,  .maxturnspeed = 0.6f, .teamname = "CRUEL FUEL",       .teamshortname = "CRF", .logofilename = "rom://textures/ui/teams/cruel.rgba32.sprite",    .modelfilename = "rom://models/celica/celica_black.t3dm", .voicenamefn = "voice/Team_CruelFuel",},
{ .id = 6, .style = 0, .teamcolor = (0x61ff64FF), .difficulty = 0.2f,  .maxturnspeed = 0.5f, .teamname = "VENOMOUS RHINOS",  .teamshortname = "VNR", .logofilename = "rom://textures/ui/teams/venomous.rgba32.sprite", .modelfilename = "rom://models/celica/celica_green.t3dm", .voicenamefn = "voice/Team_VenomousRhinos",},
{ .id = 7, .style = 0, .teamcolor = (0xff5820FF), .difficulty = 0.1f,  .maxturnspeed = 1.0f, .teamname = "SKILLED ALL-STARS",.teamshortname = "SKA", .logofilename = "rom://textures/ui/teams/allstars.rgba32.sprite", .modelfilename = "rom://models/celica/celica_orange.t3dm", .voicenamefn = "voice/Team_SkilledAllstars",},
};

void teaminfo_init_controllers(int playercontrollers[4]){
  for(int i = 0; i < 4; i++)
    matchinfo.playercontrollers[i] = playercontrollers[i];
}

void teaminfo_init(int tindex, bool right){
  teamstats_t stats = {0};
  stats.team = &teams[tindex];
  if(right) matchinfo.tright = stats;
  else matchinfo.tleft = stats;
}
