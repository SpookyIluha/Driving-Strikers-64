#ifndef ENGINE_LOCALE_H
#define ENGINE_LOCALE_H
/// made by SpookyIluha
/// language code

#include <libdragon.h>
#include "audioutils.h"
#include "engine_gamestatus.h"
#include "inih/ini.h"
#include "engine_eeprom.h"
#include "engine_gfx.h"

extern ini_t* inifile;
extern char languages[64][64];
extern char languages_names[64][64];
extern int language_count;

extern rdpq_font_t*  fonts[16];

void font_clear();
void font_setup();

/// @brief Load a dictionary with selected language
void engine_load_dictionary();

/// @brief Get a translated string with the current language by a key
/// @param name key of the translated string as found in dictionary.ini
/// @return translated string
const char* dictstr(const char* name);

const char* inistr(const char* section, const char* name);

/// @brief Load languages list into the engine
void engine_load_languages();

/// @brief Set the desired language of the game
/// @param lang shortname of the language as found in languages.ini (i.e en/ru/de/fr etc)
void engine_set_language(int index);

/// @brief Get the current language code
/// @return shortname of the language as found in languages.ini (i.e en/ru/de/fr etc)
char* engine_get_language();

#endif