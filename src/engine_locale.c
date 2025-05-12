#include <libdragon.h>
#include "ctype.h"
#include "inih/ini.h"
#include "audioutils.h"
#include "engine_gamestatus.h"
#include "engine_eeprom.h"
#include "engine_gfx.h"
#include "engine_locale.h"

ini_t* inifile = NULL;
char languages[64][64];
char languages_names[64][64];
int language_count = 0;

rdpq_font_t*  fonts[16];

void font_clear(){
    for(int i = 0; i < 16; i++) if(fonts[i]) {rdpq_text_unregister_font(i+1); rdpq_font_free(fonts[i]); fonts[i] = NULL;}
}

void font_setup(){
    for(int i = 0; i < 16; i++) if(fonts[i]) {rdpq_text_unregister_font(i+1); rdpq_font_free(fonts[i]); fonts[i] = NULL;}
    const char* fontname = NULL;
    char  fontkey[64];
    int i = 0;
    do{
        sprintf(fontkey, "font%i", i);
        fontname = inistr("Fonts", fontkey);
        if(fontname){
            fonts[i] = rdpq_font_load(fontname);
            rdpq_text_register_font(i+1, fonts[i]); 
            rdpq_fontstyle_t style; style.color = RGBA32(0,0,0,255);
            rdpq_font_style(fonts[i], 0, &style);  
            style.color = RGBA32(255,255,255,255);
            rdpq_font_style(fonts[i], 1, &style);
            style.color = RGBA32(200,200,200,255);
            rdpq_font_style(fonts[i], 2, &style);
        }
        i++;
    } while(fontname);
}

void engine_load_dictionary(){
    if(inifile)  {ini_free(inifile); inifile = NULL;}
    inifile = ini_load(languages[gamestatus.state_persistent.current_language]);
}

const char* dictstr(const char* name){
    const char* str = ini_get(inifile, "Dictionary", name);
    if(!str) return "null";
    else return str;
}

const char* inistr(const char* section, const char* name){
    const char* str = ini_get(inifile, section, name);
    if(!str) return NULL;
    else return str;
}

int engine_load_languages_callback(const char *fn, dir_t *dir, void *data){
    debugf("Found language %s\n", fn);
    strcpy(&(languages[language_count][0]), fn);

    char nameonly[128] = {0};
    strcpy(nameonly, strrchr(fn, '/') + 1);
    *strrchr(nameonly, '.') = '\0';
    char *s = nameonly; while (*s) { *s = toupper((unsigned char) *s);s++;}
    strcpy(languages_names[language_count], nameonly);

    language_count++;
    return DIR_WALK_CONTINUE;
}

void engine_load_languages(){
    language_count = 0;
    gamestatus.state_persistent.current_language = 0;
    dir_glob("*.ini", "rom:/locale/", engine_load_languages_callback, NULL);
}

void engine_set_language(int index){
    gamestatus.state_persistent.current_language = index;
    engine_load_dictionary();
}

char* engine_get_language(){
    return &(languages_names[gamestatus.state_persistent.current_language][0]);
}