#ifndef ENGINE_GAMESTATUS_H
#define ENGINE_GAMESTATUS_H
/// made by SpookyIluha
/// The global gamestatus variables that can be used at runtime and EEPROM saving

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_FONTS 32
#define MAX_CHARACTERS_LIMIT 8
#define SHORTSTR_LENGTH 32
#define LONGSTR_LENGTH 64
#define CHARACTER_MAX_VARS 8

#define STATE_MAGIC_NUMBER ((uint16_t)sizeof(gamestate_t))
#define STATE_PERSISTENT_MAGIC_NUMBER ((uint16_t)sizeof(gamestate_persistent_t))

typedef enum {
    ONE_MINUTE = 1,
    TWO_MINUTES = 2,
    THREE_MINUTES = 3,
    FOUR_MINUTES = 4,
    FIVE_MINUTES = 5
} matchduration_t;

typedef enum {
    FASTEST = 0,
    DEFAULT = 1,
    NICEST = 2,
} graphicssetting_t;

typedef struct gamestate_s{
    unsigned short magicnumber;
    struct{
        float bgmusic_vol;
        float sound_vol;
    } audio;

    struct{
        struct{
            matchduration_t duration;
            graphicssetting_t graphics;
            int vibration;
            float deadzone;
        } settings;
    } game;

    bool stadium_unlocked;

} gamestate_t;

typedef enum{
    SAVE_NONE = 0,
    SAVE_AUTOSAVE,
    SAVE_MANUALSAVE
} saveenum_t;

typedef struct{
    unsigned short        magicnumber;
    uint64_t     global_game_state; // game incomplete, complete, broken etc.
    saveenum_t   lastsavetype; // true - manual, false - autosave
    bool         autosaved;
    bool         manualsaved;
    int          current_language; // "en" - english, etc.
    bool         modded;
} gamestate_persistent_t;

typedef struct gamestatus_s{
    double currenttime;
    double realtime;
    double deltatime;
    double deltarealtime;
    double fixeddeltatime;
    double fixedtime;
    double fixedframerate;

    double gamespeed;
    bool   paused;

    gamestate_t state;
    gamestate_persistent_t state_persistent;

    float statetime;
} gamestatus_t;

/// @brief Global game state, includes a persistent state, a state for the game, state of the engine's datapoints
extern gamestatus_t gamestatus;

#define CURRENT_TIME            gamestatus.currenttime
#define CURRENT_TIME_REAL       gamestatus.realtime

#define GAMESPEED               gamestatus.gamespeed
#define GAME_PAUSED             gamestatus.paused

#define DELTA_TIME              gamestatus.deltatime
#define DELTA_TIME_REAL         gamestatus.deltarealtime

#define DELTA_TIME_FIXED        gamestatus.fixeddeltatime
#define CURRENT_TIME_FIXED      gamestatus.fixedtime

/// Init the global game state to 0
void state_init();

#ifdef __cplusplus
}
#endif

#endif