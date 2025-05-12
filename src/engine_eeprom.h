#ifndef ENGINE_EEPROM
#define ENGINE_EEPROM
/// made by SpookyIluha
/// EEPROM save functions

#include <libdragon.h>
#include "engine_gamestatus.h"
#include "audioutils.h"

/// @brief Init the eeprom system and saves
void engine_eeprom_init();

/// @brief Unused
void engine_eeprom_checksaves();

/// @brief Save a manual file, should only be used by the game, or inside the game loop (i.e after calling newgame)
bool engine_eeprom_save_manual();

/// @brief Load a manual file replacing the gamestatus
bool engine_eeprom_load_manual();

/// Save any persistent data, such as language, global game state etc.
bool engine_eeprom_save_persistent();

/// Load any persistent data into gamestatus, such as language, global game state etc.
bool engine_eeprom_load_persistent();

/// Delete all saves except persistent
void engine_eeprom_delete_saves();

/// Delete persistent save (and all saves as well)
void engine_eeprom_delete_persistent();

#endif