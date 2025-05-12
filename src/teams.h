#ifndef TEAMS_H
#define TEAMS_H

typedef struct{
  int         id;
  int         style;
  uint32_t    teamcolor;
  const char* teamname;
  const char* teamshortname;
  const char* logofilename;
  const char* modelfilename;
  const char* voicenamefn;
  float       difficulty;
  float       maxturnspeed;
} teamdef_t;

extern teamdef_t teams[8];

typedef struct{
  int score;
  int shots;
  int shotsontarget;
  float posession;
  teamdef_t* team;
} teamstats_t;


#define TEAM_RIGHT 1
#define TEAM_LEFT 0

void teaminfo_init_controllers(int playercontrollers[4]);

void teaminfo_init(int tindex, bool right);
  
  #endif