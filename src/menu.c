#include <libdragon.h>
#include "audioutils.h"
#include "intro.h"
#include "playtime_logic.h"
#include "maps.h"
#include "effects.h"
#include "engine_eeprom.h"
#include "engine_gamestatus.h"
#include "engine_locale.h"

bool cont = false;
sprite_t* background;
rspq_block_t* background_block;
sprite_t* selector;
sprite_t* button_a;
sprite_t* button_b;

float bg_time = 0;
void render_background(){
    bg_time += display_get_delta_time();
    int alpha = 255 * (0.25f * sinf(bg_time) + 0.25);
    rdpq_set_prim_color(RGBA32(alpha,alpha,alpha,255));
    rdpq_set_env_color(RGBA32(127,127,127,255));
    if(!background_block){
        rspq_block_begin();
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER1((ENV,PRIM,TEX0,TEX0), (0,0,0,TEX0)));
        rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
        rdpq_sprite_blit(background,0,0,NULL);
        background_block = rspq_block_end();
    } rspq_block_run(background_block);

   // rdpq_text_printf(NULL, 1, 30, 30, "^02FPS: %.2f", display_get_fps());
    //heap_stats_t stats; sys_get_heap_stats(&stats);
    //rdpq_text_printf(NULL, 1, 30, 50, "^02MEM: %i total, %i used", stats.total, stats.used);
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER1((ENV,PRIM,TEX0,TEX0), (0,0,0,TEX0)));
    rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
}

float spr_time = 0;
void render_sprite(sprite_t* sprite, rspq_block_t** block, float x, float y){
    spr_time += display_get_delta_time();
    int alpha = 255 * (0.5f * sinf(spr_time * 4) + 0.5f);
    rdpq_set_env_color(RGBA32(alpha,alpha,alpha,255));
    if(!*block){
        rspq_block_begin();
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,ENV,TEX0,TEX0), (TEX0,0,PRIM,0)));
        rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_sprite_blit_anchor(sprite,ALIGN_CENTER,VALIGN_CENTER,x,y,NULL);
        *block = rspq_block_end();
    } rspq_block_run(*block);
}

void menu_credits(){
    rspq_wait();
    float time = 0;
    time = 0;
    bgm_play("7_theme", true, 1); 
    while(time < 2){
        audioutils_mixer_update();
        joypad_poll();
        joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        if(pressed.b){
            bgm_play("1_select", true, 1);
            return;
        }
        rdpq_attach(display_get(), NULL);
        render_background();
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
        rdpq_sprite_blit(button_b, 500, 420, NULL);
        rdpq_textparms_t parms2; parms2.style_id = 2;
        rdpq_text_printf(&parms2, 3, 540, 445, dictstr("mm_back")); 
        rdpq_detach_show();
        time += display_get_delta_time();
    }
    char keyt[32]; char keyd[32]; size_t index = 0;
    const char* title = NULL;
    do {
        time = 0;
        sprintf(keyt, "mm_c_t%i", index + 1); sprintf(keyd, "mm_c_d%i", index + 1);
        title = inistr("Credits", keyt);
        const char* description = inistr("Credits", keyd);
        char description_arr[512]; if(description) strcpy(description_arr, description);
        int lines = 1;
        for(int c = 0; description_arr[c] != '\0'; c++){if(description_arr[c] == '\n') { lines++;}}
        while(time < 4.5f){
            audioutils_mixer_update();
            joypad_poll();
            joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
            if(pressed.b){
                bgm_play("1_select", true, 1);
                return;
            }
            float alpha = 255;
            if(time < 1) alpha = time * 255;
            if(time > 2.5f) alpha = (4.5f - time) * 127;
            rdpq_attach(display_get(), NULL);
            render_background();
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_TEX);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
            rdpq_sprite_blit(button_b, 500, 420, NULL);
            rdpq_fontstyle_t style; style.color = RGBA32(255,255,255, alpha);
            rdpq_fontstyle_t style2; style2.color = RGBA32(180,180,180, alpha);
            for(int i = 0; i < 4; i++) {rdpq_font_style(fonts[i], 3, &style); rdpq_font_style(fonts[i], 4, &style2);}
            rdpq_textparms_t textparms; textparms.align = ALIGN_CENTER; textparms.style_id = 3;
            textparms.width = display_get_width();
            if(title)       rdpq_text_printf(&textparms, 3, 0, display_get_height() / 2 - 12*lines ,        title);
            textparms.style_id = 4;
            if(description) rdpq_text_printf(&textparms, 4, 0, display_get_height() / 2 - 12*lines + 24 ,   description_arr);

            rdpq_textparms_t parms2; parms2.style_id = 2;
            rdpq_text_printf(&parms2, 3, 540, 445, dictstr("mm_back")); 
            rdpq_detach_show();
            time += display_get_delta_time();
        }
        rspq_wait();
        index++;
    } while(title);
    bool pressed_b = false;
    while(!pressed_b){
        audioutils_mixer_update();
        joypad_poll();
        joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        if(pressed.b){
            pressed_b = true;
        }
        rdpq_attach(display_get(), NULL);
        render_background();
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
        rdpq_sprite_blit(button_b, 500, 420, NULL);
        rdpq_textparms_t parms2; parms2.style_id = 2;
        rdpq_text_printf(&parms2, 3, 540, 445, dictstr("mm_back")); 
        rdpq_detach_show();
        time += display_get_delta_time();
    }
    bgm_play("1_select", true, 1);
}

void menu_logos(){
    const char* logos_fn[] = {
        "rom:/textures/logos/logo.sprite",
        "rom:/textures/logos/realityjump.sprite"
    };
    for(int i = 0; i < 2; i++){
        float logotime = 0;
        if(background_block) {rspq_block_free(background_block); background_block = NULL;}
        if(background) {sprite_free(background); background = NULL;}
        background = sprite_load(logos_fn[i]);
        while(logotime < 4){
            audioutils_mixer_update();
            rdpq_attach(display_get(), NULL);
            float modulate = logotime < 1? logotime * 250 : 250;
            if(logotime > 3.1f) modulate = (4.1f - logotime) * 250;
            rdpq_set_prim_color(RGBA32(modulate,modulate,modulate,255));
            if(!background_block){
                rspq_block_begin();
                rdpq_set_mode_standard();
                rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
                rdpq_mode_filter(FILTER_BILINEAR);
                rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
                rdpq_sprite_blit(background,0,0,NULL);
                background_block = rspq_block_end();
            } rspq_block_run(background_block);
            rdpq_detach_show();
            logotime += display_get_delta_time();
        }
        rdpq_attach(display_get(), NULL);
        rdpq_clear(RGBA32(0,0,0,0));
        rdpq_detach_show();
        rspq_wait();
        if(background_block) {rspq_block_free(background_block); background_block = NULL;}
        if(background) {sprite_free(background); background = NULL;}
    }   
}

void menu_cover(){
    if(background_block) {rspq_block_free(background_block); background_block = NULL;}
    if(background) {sprite_free(background); background = NULL;}
    background = sprite_load("rom:/textures/logos/cover.sprite");
    if(bgmusic_name[0] == 0) bgm_play("1_select", true, 0);
    bool pressed_start = false;
    float logotime = 1;
    float gtime = 0;
    while(logotime > 0){
        joypad_poll();
        joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        if(pressed.start && !pressed_start) {
            pressed_start = true;
            sound_play("menu_confirm", false);
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            sound_play("DrivingStrikers", false);
        }
        if(pressed_start) logotime -= display_get_delta_time();
        gtime += display_get_delta_time();

        audioutils_mixer_update();
        effects_update();

        rdpq_attach(display_get(), NULL);
        render_background();

        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
        rdpq_set_prim_color(RGBA32(0,0,0, logotime * 120 + 10));
        rdpq_fill_rectangle(0, 350, display_get_width(), 430);

        rdpq_textparms_t parmstext = {0}; parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
        if(((int)gtime % 2) == 0 && !pressed_start) rdpq_text_printf(&parmstext, 2, 0,350, dictstr("press_start"));
        rdpq_detach_show();
    }
    rspq_wait();
    if(background_block) {rspq_block_free(background_block); background_block = NULL;}
    if(background) {sprite_free(background); background = NULL;}
}

void wait_sec_audio(float s){
    while(s > 0){
        s -= display_get_delta_time();
        audioutils_mixer_update();
        rdpq_attach(display_get(), NULL);
        rdpq_detach_show();
    }
}

// cursed thing
void menu_quick_game(){
    int selection = 0;
    float offset = 400;
    sprite_t* selected_spr = sprite_load(teams[selection].logofilename);
    sprite_t* arrow_spr = sprite_load("rom:/textures/ui/chevron_right.rgba32.sprite");
    sprite_t* controller_spr[4]; for(int i = 0; i < MAXPLAYERS; i++) {char csprfn[256]; sprintf(csprfn, "rom:/textures/ui/controller%i.rgba32.sprite", i+1); controller_spr[i] = sprite_load(csprfn);}
    rspq_block_t* spr_block = NULL;
    int selection_stage = 0;
    float  stage_time = -1;
    bool map_start = false;
    teamdef_t*  teams_selected[2]= {NULL}; int teams_selected_index[2] = {0};
    mapinfo_t* map_selected = NULL;
    struct{
        int c_team[MAXPLAYERS];
        bool c_pressedstart[MAXPLAYERS];
        float c_position[MAXPLAYERS];
        int controllers[MAXPLAYERS];
        bool can_start;
    } team_assignment = {0};
    if(joypad_is_connected(0)) {team_assignment.c_pressedstart[0] = true; team_assignment.c_team[0] = -1;} // first controller player is joined by default

    effects_rumble_stop();
    while(true){
        offset = fm_lerp(offset, 0, 0.25f);
        joypad_poll(); 
        // team selection
        int axis_stick_x = joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_X);
        bool changed_selection = false;
        if(stage_time > 0) {
            stage_time -= display_get_delta_time();
            if(stage_time <= 0 && selection_stage == 0) {selection_stage++; selection = 5; changed_selection = true;}
            else if(stage_time <= 0 && selection_stage == 1) {selection_stage++; if(selection == 5) selection = randr(0,4); map_selected = &maps[selection];}
            else if(stage_time <= 0 && selection_stage == 2) {selection_stage++; map_start = true; break;}
        }

        if(selection_stage == 1 || selection_stage == 0)
        if(axis_stick_x != 0 && stage_time <= 0){
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            sound_play("menu_navigate", false);
            selection += axis_stick_x;
            if(&teams[selection] == teams_selected[0] && selection_stage == 0) selection += axis_stick_x;
            if(selection_stage == 1) selection = iwrap(selection, 0, 5);
            else selection = iwrap(selection, 0, 7);
            changed_selection = true;
        }
        if(selection_stage == 2 && stage_time <= 0){
            for(int i = 0; i < MAXPLAYERS; i++){
                if(joypad_is_connected(i)){
                    joypad_buttons_t pressed = joypad_get_buttons_pressed(i);
                    int axis_stick_x = joypad_get_axis_pressed(i, JOYPAD_AXIS_STICK_X);
                    if(pressed.start)  {
                        effects_add_rumble(i, 0.1f); 
                        sound_play("menu_navigate", false); 
                        team_assignment.c_pressedstart[i] = !team_assignment.c_pressedstart[i];}
                    if(axis_stick_x != 0 && team_assignment.c_pressedstart[i]){
                        effects_add_rumble(i, 0.1f);
                        sound_play("menu_navigate", false);
                        team_assignment.c_team[i] += axis_stick_x; team_assignment.c_team[i] = iwrap(team_assignment.c_team[i], -1, 1);
                    }
                } else {team_assignment.c_pressedstart[i] = false; team_assignment.c_team[i] = 0;}
                team_assignment.c_position[i] = fm_lerp(team_assignment.c_position[i], 100 * team_assignment.c_team[i], 0.15f);
            }
            int left_amount = 0, right_amount = 0;
            team_assignment.can_start = false;
            for(int i = 0; i < MAXPLAYERS; i++){
                team_assignment.controllers[i] = -1;
                if(team_assignment.c_pressedstart[i]) team_assignment.can_start = true;
            }
            if(team_assignment.can_start) 
                for(int i = 0; i < MAXPLAYERS; i++){
                    if(team_assignment.c_pressedstart[i] && !team_assignment.c_team[i]) team_assignment.can_start = false;
            } if(team_assignment.can_start){
                for(int i = 0; i < MAXPLAYERS; i++){
                    if(team_assignment.c_pressedstart[i]){
                        if(team_assignment.c_team[i] == 1) {if(right_amount < 2) team_assignment.controllers[2+right_amount] = i; right_amount++;}
                        else if(team_assignment.c_team[i] == -1) {if(left_amount < 2) team_assignment.controllers[left_amount] = i; left_amount++;}
                    }
                }
                if(left_amount > 2 || right_amount > 2) team_assignment.can_start = false;
            }
        }
        // select the team from the menu if A pressed
        joypad_buttons_t pressed = {0};
        if(stage_time <= 0) pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);

        if(pressed.a) {
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            sound_play("menu_confirm", false);
            if(selection_stage == 0){
                if(!teams_selected[0]) {teams_selected[0] = &teams[selection]; sound_play(teams_selected[0]->voicenamefn, false); teams_selected_index[0] = selection; selection++; selection = iwrap(selection, 0, 7); changed_selection = true;}
                else {teams_selected[1] = &teams[selection]; sound_play(teams_selected[1]->voicenamefn, false); teams_selected_index[1] = selection; stage_time = 2.0f;}
            } else if(selection_stage == 1){
                if(maps[selection].unlocked || gamestatus.state.stadium_unlocked){
                    stage_time = 2.0f; if(maps[selection].voicenamefn) sound_play(maps[selection].voicenamefn, false);
                }
            } else if(selection_stage == 2 && team_assignment.can_start) {stage_time = 2.0f; sound_play("StartYourEngines", false);}
        }        // back from the menu if B pressed
        if(pressed.b) {
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            sound_play("menu_confirm", false);
            break;
        }

        rdpq_attach(display_get(), NULL);
        // background drawing
        render_background();

        // team sprite drawing
        if(selection_stage == 1 || selection_stage == 0){
            if(stage_time <= 0 || stage_time > 1.5f) rdpq_set_prim_color(RGBA32(127,127,127,255));
            else rdpq_set_prim_color(RGBA32(127,127,127,(int)(stage_time*30)%3 == 0? 255 : 50));
            render_sprite(selected_spr, &spr_block, display_get_width() / 2, display_get_height() / 2);
        }
        if(selection_stage == 2){
            if(stage_time <= 0 || stage_time > 1.5f) rdpq_set_env_color(RGBA32(127,127,127,255));
            else rdpq_set_env_color(RGBA32(127,127,127,(int)(stage_time*30)%3 == 0? 255 : 50));
            rdpq_mode_combiner(RDPQ_COMBINER1((ENV,PRIM,TEX0,TEX0), (TEX0,0,ENV,0)));
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            for(int i = 0; i < MAXPLAYERS; i++){
                if(joypad_is_connected(i)){
                    if(stage_time <= 0 || stage_time > 1.5f) rdpq_set_env_color(RGBA32(127,127,127,team_assignment.c_pressedstart[i]? 255 : 50));
                    rdpq_sprite_blit_anchor(controller_spr[i], ALIGN_CENTER, VALIGN_CENTER, team_assignment.c_position[i] + (display_get_width() / 2), 150 + 64*i, NULL);
                }
            }
            //rdpq_textparms_t parmstext = {0};  parmstext.style_id = 1;
            //rdpq_text_printf(&parmstext, 3, 40, 40, "%i %i %i %i", team_assignment.controllers[0], team_assignment.controllers[1],team_assignment.controllers[2],team_assignment.controllers[3]);
        }

        // various sprites
        rdpq_blitparms_t bparms;
        rdpq_sprite_blit(arrow_spr, 440, 200, &bparms); bparms.flip_x = true;
        rdpq_sprite_blit(arrow_spr, 135, 200, &bparms);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
        if(selection_stage != 2 || team_assignment.can_start) rdpq_sprite_blit(button_a, 360 - offset, 420, NULL);
        rdpq_sprite_blit(button_b, 500 - offset, 420, NULL);

        rdpq_textparms_t parmstext = {0}; parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
        if(selection_stage == 0){
            rdpq_text_printf(&parmstext, 2, 0, 40, dictstr("mm_qm_choose_teams"));
            rdpq_text_printf(&parmstext, 3, 0,360, "%s  VS  %s", teams_selected[0]? teams_selected[0]->teamname : "?", teams_selected[1]? teams_selected[1]->teamname : "?");
        } else if (selection_stage == 1){
            rdpq_text_printf(&parmstext, 2, 0, 40, dictstr("mm_qm_choose_map"));
            
            rdpq_text_printf(&parmstext, 3, 0,330, maps[selection].unlocked || gamestatus.state.stadium_unlocked? maps[selection].name : dictstr("mm_qm_map_unknown"));
        } else{
            rdpq_text_printf(&parmstext, 2, 0, 40, dictstr("mm_qm_choose_assign"));
            rdpq_text_printf(&parmstext, 3, 0,360, "%s      %s", teams_selected[0]? teams_selected[0]->teamname : "?", teams_selected[1]? teams_selected[1]->teamname : "?");
        }
        // team selection description

        rdpq_textparms_t parms2; parms2.style_id = 2;
        rdpq_text_printf(&parms2, 3, 540  - offset, 445, dictstr("mm_back")); 
        if(selection_stage != 2) rdpq_text_printf(&parms2, 3, 400 - offset, 445, dictstr("mm_select")); 
        else if(team_assignment.can_start) rdpq_text_printf(&parms2, 3, 400 - offset, 445, dictstr("mm_play")); 

        rdpq_detach_show();

        if(changed_selection){ // load the appropriate team image if selected
            rspq_wait();
            sprite_free(selected_spr); rspq_block_free(spr_block); spr_block = NULL;
            if(selection_stage == 0)
                 selected_spr = sprite_load(teams[selection].logofilename);
            else {
                if(maps[selection].unlocked || gamestatus.state.stadium_unlocked)
                    selected_spr = sprite_load(maps[selection].previewimgfn);
                else selected_spr = sprite_load("rom:/textures/ui/stadiums/UNL.rgba32.sprite");
            }
        }

        audioutils_mixer_update();
        effects_update();
    }
    rspq_wait();
    sprite_free(arrow_spr);
    sprite_free(selected_spr); rspq_block_free(spr_block); spr_block = NULL;
    for(int i = 0; i < MAXPLAYERS; i++) sprite_free(controller_spr[i]);
    if(map_start){
        if(background_block) {rspq_block_free(background_block); background_block = NULL;}
        if(background) {sprite_free(background); background = NULL;}
        matchinfo_init();
        teaminfo_init_controllers(team_assignment.controllers);
        teaminfo_init(teams_selected_index[0], TEAM_LEFT);
        teaminfo_init(teams_selected_index[1], TEAM_RIGHT);
        rspq_wait();
        display_close();
        display_init(RESOLUTION_640x480, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_DEDITHER);
        game_start(map_selected);
        rspq_wait();
        display_close();
        display_init(RESOLUTION_640x480, DEPTH_16_BPP, is_memory_expanded()? 3 : 2, GAMMA_NONE, FILTERS_DEDITHER);
        background = sprite_load("rom:/textures/ui/menu_bg.sprite");
        bgm_play("1_select", true, 1);
    }
}

typedef struct{
    teamdef_t* team;
    int teamindex;
    int wins, draws, losts;
    int goals_for, goals_against;
    int points;
    int plays;
} teams_league_t;

int teams_league_compare( const void* a, const void* b){
    teams_league_t* A = ( (teams_league_t*) a );
    teams_league_t* B = ( (teams_league_t*) b );
            
    if ( A->points == B->points ) return 0;
    else if ( A->points < B->points ) return 1;
    else return -1;
}

// cursed thing number 2
void menu_league(){
    int selection = 0;
    sprite_t* selected_spr = sprite_load(teams[selection].logofilename);
    sprite_t* arrow_spr = sprite_load("rom:/textures/ui/chevron_right.rgba32.sprite");
    rspq_block_t* spr_block = NULL;
    int selection_stage = 0;
    float  stage_time = -1;
    bool map_start = false;
    teamdef_t*  team_selected  = NULL; int teams_selected_index = -1;
    mapinfo_t* map_selected = NULL;

    effects_rumble_stop();
    float offset = 400;
    while(selection_stage == 0){
        offset = fm_lerp(offset,0,0.25f);
        joypad_poll(); 
        // team selection
        int axis_stick_x = joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_X);
        bool changed_selection = false;
        if(stage_time > 0) {
            stage_time -= display_get_delta_time();
            if(stage_time <= 0 && selection_stage == 0) {selection_stage++; break;}
        }

        if(selection_stage == 1 || selection_stage == 0)
        if(axis_stick_x != 0 && stage_time <= 0){
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            sound_play("menu_navigate", false);
            selection += axis_stick_x;
            if(selection_stage == 1) selection = iwrap(selection, 0, 5);
            else selection = iwrap(selection, 0, 7);
            changed_selection = true;
        }
        
        // select the team from the menu if A pressed
        joypad_buttons_t pressed = {0};
        if(stage_time <= 0) pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);

        if(pressed.a && stage_time <= 0) {
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            sound_play("menu_confirm", false);
            if(selection_stage == 0){
                {team_selected = &teams[selection]; sound_play(team_selected->voicenamefn, false); teams_selected_index = selection; stage_time = 2.0f;}
            }
        } // back from the menu if B pressed
        if(pressed.b) {
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            sound_play("menu_confirm", false);
            break;
        }

        rdpq_attach(display_get(), NULL);
        // background drawing
        render_background();

        // team sprite drawing
        if(selection_stage == 1 || selection_stage == 0){
            if(stage_time <= 0 || stage_time > 1.5f) rdpq_set_prim_color(RGBA32(127,127,127,255));
            else rdpq_set_prim_color(RGBA32(127,127,127,(int)(stage_time*30)%3 == 0? 255 : 50));
            render_sprite(selected_spr, &spr_block, display_get_width() / 2, display_get_height() / 2);
        }

        // various sprites
        rdpq_blitparms_t bparms;
        rdpq_sprite_blit(arrow_spr, 440, 200, &bparms); bparms.flip_x = true;
        rdpq_sprite_blit(arrow_spr, 135, 200, &bparms);
        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
        rdpq_sprite_blit(button_a, 360 - offset, 420, NULL);
        rdpq_sprite_blit(button_b, 500 - offset, 420, NULL);

        rdpq_textparms_t parmstext = {0}; parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
        if(selection_stage == 0){
            rdpq_text_printf(&parmstext, 2, 0, 40, dictstr("mm_qm_choose_teams"));
            rdpq_text_printf(&parmstext, 3, 0,360, "%s", team_selected? team_selected->teamname : "?");
        }
        // team selection description

        rdpq_textparms_t parms2; parms2.style_id = 2;
        rdpq_text_printf(&parms2, 3, 540 - offset, 445, dictstr("mm_back")); 
        rdpq_text_printf(&parms2, 3, 400 - offset, 445, dictstr("mm_select")); 

        rdpq_detach_show();

        if(changed_selection){ // load the appropriate team image if selected
            rspq_wait();
            sprite_free(selected_spr); rspq_block_free(spr_block); spr_block = NULL;
            if(selection_stage == 0)
                 selected_spr = sprite_load(teams[selection].logofilename);
        }

        audioutils_mixer_update();
        effects_update();
    }
    rspq_wait();
    sprite_free(arrow_spr);
    sprite_free(selected_spr); rspq_block_free(spr_block); spr_block = NULL;
    if(teams_selected_index >= 0){
        teams_league_t teams_league[8] = {0};{
            int curteam = teams_selected_index;
            for(int i = 0; i < 8; i++){
                teams_league[i].team = &teams[curteam];
                teams_league[i].teamindex = curteam;
                curteam += 1; curteam = iwrap(curteam, 0, 7);
            }
        }
        teams_league_t teams_league_sorted[8];

        int lastmapplayed = -1;
        int curteamagainst = teams_selected_index;
        int curteamagainst_lli = 0;
        for(int i = 0; i < 8; i++){
            float offset = 400;
            memcpy(teams_league_sorted, teams_league, sizeof(teams_league));
            qsort(teams_league_sorted, 8, sizeof(teams_league_t), teams_league_compare);

            curteamagainst = iwrap(curteamagainst + 1, 0, 7);
            curteamagainst_lli++;
            rspq_block_t* tableblock = NULL;
            while(true){
                offset = fm_lerp(offset, 0, 0.25f);
                joypad_poll(); 
                audioutils_mixer_update();

                joypad_buttons_t pressed = {0};
                pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
                if(pressed.b && i < 7) {i = 8; teams_league[0].points = -1; break;}
                if(pressed.a && i < 7) {sound_play(teams[curteamagainst].voicenamefn, false); break;}
                if(pressed.b && i >= 7) break;

                rdpq_attach(display_get(), NULL);
                // background drawing
                render_background();

                rdpq_textparms_t parmstext = {0}; parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
                rdpq_text_printf(&parmstext, 2, 0, 40, dictstr("mm_league"));
                // team selection description
                rdpq_fontstyle_t style; style.color = RGBA32(255,0,0,255);
                rdpq_font_style(fonts[2], 5, &style);

                rdpq_textparms_t parms2; parms2.style_id = 2;
                rdpq_text_printf(&parms2, 3, (i < 7? 540 : 500) - offset, 445, i < 7? dictstr("mm_back") : dictstr("match_s_continue")); 
                if(i < 7) rdpq_text_printf(&parms2, 3, 400 - offset, 445, dictstr("mm_play")); 
                
                rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                rdpq_mode_combiner(RDPQ_COMBINER_TEX);
                if(i < 7) rdpq_sprite_blit(button_a, 360 - offset, 420, NULL);
                rdpq_sprite_blit(button_b, (i < 7? 500 : 460) - offset, 420, NULL);

                if(!tableblock){
                    rspq_block_begin();
                    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
                    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                    rdpq_set_prim_color(RGBA32(0,0,0,128));
                    rdpq_fill_rectangle(60 , 140, 600 , 400);
                    rdpq_set_prim_color(RGBA32(255,255,0,128));
                    for(int i = 0; i < 8; i++)
                        if(teams_league_sorted[i].teamindex == teams_selected_index)
                            rdpq_fill_rectangle(60 , 200 + 25*i, 600 , 225 + 25*i);

                    parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_LEFT; parmstext.width = display_get_width(); parmstext.height = 40; parmstext.style_id = 1;
                    const char* tablenames[7] = {"P", "W", "D", "L", "GF", "GA", "PTS"};
                    for(int i = 0; i < 7; i++){
                        parmstext.style_id = 5;
                        rdpq_text_printf(&parmstext, 3, 330 + i*35, 150, tablenames[i]);
                    }
                    for(int i = 0; i < 8; i++){
                        parmstext.style_id = 2;
                        rdpq_text_printf(&parmstext, 3, 80 ,  195 + 25*i, "%s", teams_league_sorted[i].team->teamname);
                        rdpq_text_printf(&parmstext, 3, 300 + 30 , 195 + 25*i, "%i", teams_league_sorted[i].plays);
                        rdpq_text_printf(&parmstext, 3, 335 + 30 , 195 + 25*i, "%i", teams_league_sorted[i].wins);
                        rdpq_text_printf(&parmstext, 3, 370 + 30 , 195 + 25*i, "%i", teams_league_sorted[i].draws);
                        rdpq_text_printf(&parmstext, 3, 405 + 30 , 195 + 25*i, "%i", teams_league_sorted[i].losts);
                        rdpq_text_printf(&parmstext, 3, 440 + 30 , 195 + 25*i, "%i", teams_league_sorted[i].goals_for);
                        rdpq_text_printf(&parmstext, 3, 475 + 30 , 195 + 25*i, "%i", teams_league_sorted[i].goals_against);
                        parmstext.style_id = 5;
                        rdpq_text_printf(&parmstext, 3, 510 + 30 , 195 + 25*i, "%i", teams_league_sorted[i].points);
                    }
                    tableblock = rspq_block_end();
                } rspq_block_run(tableblock);

                rdpq_detach_show();
            }
            rspq_wait();
            rspq_block_free(tableblock);
            if(i < 7){
                if(background_block) {rspq_block_free(background_block); background_block = NULL;}
                if(background) {sprite_free(background); background = NULL;}
                matchinfo_init();
                int controllers[4] = {0,-1,-1,-1};
                teaminfo_init_controllers(controllers);
                teaminfo_init(teams_selected_index, TEAM_LEFT);
                teaminfo_init(curteamagainst, TEAM_RIGHT);
                rspq_wait();
                display_close();
                display_init(RESOLUTION_640x480, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_DEDITHER);
                int curmap = randm(4); while(curmap == lastmapplayed) curmap = randm(4); // random map, don't repeat consecutevly
                game_start(&maps[curmap]);
                rspq_wait();
                display_close();
                display_init(RESOLUTION_640x480, DEPTH_16_BPP,is_memory_expanded()? 3 : 2, GAMMA_NONE, FILTERS_DEDITHER);
                background = sprite_load("rom:/textures/ui/menu_bg.sprite");
                bgm_play("1_select", true, 1);

                teams_league[0].wins += matchinfo.tleft.score > matchinfo.tright.score? 1 : 0;
                teams_league[0].draws += matchinfo.tleft.score == matchinfo.tright.score? 1 : 0;
                teams_league[0].losts += matchinfo.tleft.score < matchinfo.tright.score? 1 : 0;
                teams_league[0].goals_for += matchinfo.tleft.score;
                teams_league[0].goals_against += matchinfo.tright.score;
                teams_league[0].points += matchinfo.tleft.score > matchinfo.tright.score? 2 : (matchinfo.tleft.score == matchinfo.tright.score? 1 : 0);
                teams_league[0].plays += 1;

                teams_league[i + 1].wins += matchinfo.tleft.score < matchinfo.tright.score? 1 : 0;
                teams_league[i + 1].draws += matchinfo.tleft.score == matchinfo.tright.score? 1 : 0;
                teams_league[i + 1].losts += matchinfo.tleft.score > matchinfo.tright.score? 1 : 0;
                teams_league[i + 1].goals_for += matchinfo.tright.score;
                teams_league[i + 1].goals_against += matchinfo.tleft.score;
                teams_league[i + 1].points += matchinfo.tleft.score < matchinfo.tright.score? 2 : (matchinfo.tleft.score == matchinfo.tright.score? 1 : 0);
                teams_league[i + 1].plays += 1;

                if(matchinfo.exited) {teams_league[0].points = 0; i = 6;}
            }if(i < 7){
                int compteam = 1;
                for(int i = 0; i < 3; i++){ // add random scores to the rest of the teams
                    match_t info = {0};
                    info.tleft.score = randm(6);
                    info.tright.score = randm(6);

                    if(compteam == curteamagainst_lli) compteam++;

                    if(teams_league[compteam].points >= 5 && info.tleft.score > info.tright.score) 
                        iswap(&info.tleft.score, &info.tright.score); // classic trollface moment

                    teams_league[compteam].wins += info.tleft.score > info.tright.score? 1 : 0;
                    teams_league[compteam].draws += info.tleft.score == info.tright.score? 1 : 0;
                    teams_league[compteam].losts += info.tleft.score < info.tright.score? 1 : 0;
                    teams_league[compteam].goals_for += info.tleft.score;
                    teams_league[compteam].goals_against += info.tright.score;
                    teams_league[compteam].points += info.tleft.score > info.tright.score? 2 : (info.tleft.score == info.tright.score? 1 : 0);
                    teams_league[compteam].plays += 1;

                    compteam++; if(compteam == curteamagainst_lli) compteam++;

                    if(teams_league[compteam].points > 6) info.tleft.score = info.tright.score; // trollface moment no 2

                    teams_league[compteam].wins += info.tleft.score < info.tright.score? 1 : 0;
                    teams_league[compteam].draws += info.tleft.score == info.tright.score? 1 : 0;
                    teams_league[compteam].losts += info.tleft.score > info.tright.score? 1 : 0;
                    teams_league[compteam].goals_for += info.tright.score;
                    teams_league[compteam].goals_against += info.tleft.score;
                    teams_league[compteam].points += info.tleft.score < info.tright.score? 2 : (info.tleft.score == info.tright.score? 1 : 0);
                    teams_league[compteam].plays += 1;

                    compteam++; if(compteam == curteamagainst_lli) compteam++;
                }
            } else{
                int maxpoints = 0; bool player_winner = false;
                for(int i = 0; i < 8; i++){
                    if(teams_league[i].points > maxpoints) maxpoints = teams_league[i].points;
                } if(maxpoints == teams_league[0].points) player_winner = true;

                if(player_winner){
                    gamestatus.state.stadium_unlocked = true;
                    engine_eeprom_save_manual();
                    engine_eeprom_save_persistent();
                    float offset = 400;
                    sound_play("Congratulations", false);
                    bgm_play("9_congrats", true, 1);
                    bool pressed_start = false;
                    float logotime = 1;
                    float gtime = 0;
                    while(logotime > 0){
                        offset = fm_lerp(offset,0,0.25f);
                        joypad_poll();
                        joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
                        if(pressed.a && !pressed_start) {
                            pressed_start = true;
                            sound_play("menu_confirm", false);
                            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
                        }
                        if(pressed_start) logotime -= display_get_delta_time();
                        gtime += display_get_delta_time();

                        audioutils_mixer_update();
                        effects_update();

                        rdpq_attach(display_get(), NULL);
                        render_background();

                        rdpq_set_mode_standard();
                        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
                        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                        rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
                        rdpq_set_prim_color(RGBA32(0,0,0, logotime * 128));
                        rdpq_fill_rectangle(0, 250, display_get_width(), 330);

                        rdpq_textparms_t parmstext = {0}; parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
                        if(((int)gtime % 2) == 0 && !pressed_start) rdpq_text_printf(&parmstext, 2, 0,250, dictstr("lg_congratulations"));
                        
                        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
                        rdpq_sprite_blit(button_a, 460 - offset, 420, NULL);

                        parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
                        // team selection description
                        rdpq_fontstyle_t style; style.color = RGBA32(255,0,0,255);
                        rdpq_font_style(fonts[2], 5, &style);

                        rdpq_textparms_t parms2; parms2.style_id = 2;
                        rdpq_text_printf(&parms2, 3, 500 - offset, 445, dictstr("match_s_continue")); 

                        parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
                        if(((int)gtime % 2) == 0 && !pressed_start) rdpq_text_printf(&parmstext, 3, 0 + offset,130, "%s", dictstr("lg_youwon"));
                        if(((int)gtime % 2) == 0 && !pressed_start) rdpq_text_printf(&parmstext, 3, 0 + offset,160, "%s", dictstr("lg_stadiumunlocked"));

                        rdpq_detach_show();
                    }
                    menu_credits(); 
                }
            }
        }

    }
}


void menu_settings(){
    int selection = 0;
    float offset = 400;
    while(true){
        offset = fm_lerp(offset, 0, 0.25f);
        joypad_poll();
        joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        if(joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_Y)){
            sound_play("menu_navigate", false);
            selection -= joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_Y);
        }
        selection = iwrap(selection, 0, 4);

        if(joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_X)){
            sound_play("menu_navigate", false);
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            int incr = joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_X);
            switch(selection){
                case 0:{ gamestatus.state.game.settings.duration = iwrap(gamestatus.state.game.settings.duration + incr, ONE_MINUTE, FIVE_MINUTES); break; }
                case 1:{ gamestatus.state.game.settings.graphics = iwrap(gamestatus.state.game.settings.graphics + incr, FASTEST, NICEST); break; }
                case 2:{ gamestatus.state.game.settings.vibration = !gamestatus.state.game.settings.vibration; break;}
                case 3:{ gamestatus.state.game.settings.deadzone = fwrap(gamestatus.state.game.settings.deadzone + ((float)incr * 0.05f), 0.0f, 0.9f); break; }
                case 4:{ 
                    gamestatus.state_persistent.current_language = iwrap(gamestatus.state_persistent.current_language + incr, 0, language_count - 1);
                    rspq_wait();
                    font_clear();
                    engine_set_language(gamestatus.state_persistent.current_language);
                    engine_load_dictionary();
                    font_setup();
                    break; }
            }
        }

        if(pressed.b) {
            sound_play("menu_confirm", false);
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            engine_eeprom_save_manual();
            engine_eeprom_save_persistent();
            return;
        }

        audioutils_mixer_update();
        effects_update();

        rdpq_attach(display_get(), NULL);
        render_background();

        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
        rdpq_blitparms_t parms; parms.cx = selector->width / 2; parms.scale_x = 2;
        rdpq_sprite_blit(selector, display_get_width() / 2 + offset, 160 + selection*40, &parms);
        rdpq_sprite_blit(button_b, 500 - offset, 420, NULL);

        rdpq_textparms_t parmstext = {0}; parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
        rdpq_text_printf(&parmstext, 2, 0,40, dictstr("mm_o_options"));
        parmstext.height = 40;
        parmstext.align = ALIGN_RIGHT; parmstext.width = 200;
        rdpq_text_printf(&parmstext, 3, 80 + offset,160, dictstr("mm_o_duration"));
        rdpq_text_printf(&parmstext, 3, 80 + offset,200, dictstr("mm_o_graphics"));
        rdpq_text_printf(&parmstext, 3, 80 + offset,240, dictstr("mm_o_vibration"));
        rdpq_text_printf(&parmstext, 3, 80 + offset,280, dictstr("mm_o_deadzone"));
        rdpq_text_printf(&parmstext, 3, 80 + offset,320, dictstr("mm_o_language"));

        parmstext.align = ALIGN_CENTER; parmstext.width = 200; parmstext.style_id = 2;
        const char* text = "null";
        {switch(gamestatus.state.game.settings.duration){case ONE_MINUTE: text = dictstr("mm_o_duration_1"); break; case TWO_MINUTES: text = dictstr("mm_o_duration_2"); break; case THREE_MINUTES: text = dictstr("mm_o_duration_3"); break; case FOUR_MINUTES: text = dictstr("mm_o_duration_4"); break; case FIVE_MINUTES: text = dictstr("mm_o_duration_5"); break;}
        rdpq_text_printf(&parmstext, 4, 320 + offset,160, text);}
        {switch(gamestatus.state.game.settings.graphics){case FASTEST: text =  dictstr("mm_o_graphics_0"); break; case DEFAULT: text = dictstr("mm_o_graphics_1"); break; case NICEST: text = dictstr("mm_o_graphics_2"); break;}
        rdpq_text_printf(&parmstext, 4, 320 + offset,200, text);}
        rdpq_text_printf(&parmstext, 4, 320 + offset,240, gamestatus.state.game.settings.vibration? dictstr("mm_o_vibration_1") : dictstr("mm_o_vibration_0"));
        rdpq_text_printf(&parmstext, 4, 320 + offset,280, "%i%%", (int)(gamestatus.state.game.settings.deadzone * 100));
        rdpq_text_printf(&parmstext, 4, 320 + offset,320, engine_get_language());

        rdpq_textparms_t parms2; parms2.style_id = 2;
        rdpq_text_printf(&parms2, 3, 540 - offset, 445, dictstr("mm_save")); 
        rdpq_detach_show();
    }
}

void menu_main(){
    if(background_block) {rspq_block_free(background_block); background_block = NULL;}
    if(background) {sprite_free(background); background = NULL;}
    background = sprite_load("rom:/textures/ui/menu_bg.sprite");
    selector = sprite_load("rom:/textures/ui/selector.rgba32.sprite");
    button_a = sprite_load("rom:/textures/ui/button_a.rgba32.sprite");
    button_b = sprite_load("rom:/textures/ui/button_b.rgba32.sprite");

    if(bgmusic_name[0] == 0) bgm_play("1_select", true, 0);
    int selection = 0;
    float offset = 400;

    while(true){
        offset = fm_lerp(offset,0, 0.25f);
        joypad_poll();
        joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        if(joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_Y)){
            sound_play("menu_navigate", false);
            selection -= joypad_get_axis_pressed(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_Y);
        }
        selection = iwrap(selection, 0, 3);

        if(pressed.a){
            sound_play("menu_confirm", false);
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            switch(selection){
                case 0: 
                    menu_quick_game();
                    break;
                case 1: 
                    menu_league(); 
                    break;
                case 2:
                    menu_settings();
                    break;
                case 3: 
                    menu_credits(); 
                    break;
            }
            offset = 400;
        }

        if(pressed.b) {
            sound_play("menu_confirm", false);
            effects_add_rumble(JOYPAD_PORT_1, 0.1f);
            menu_cover();
            if(!background) background = sprite_load("rom:/textures/ui/menu_bg.sprite");
        }

        audioutils_mixer_update();
        effects_update();

        rdpq_attach(display_get(), NULL);
        render_background();

        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_TEX);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_dithering(DITHER_BAYER_INVBAYER);
        rdpq_sprite_blit_anchor(selector, ALIGN_CENTER, VALIGN_TOP, display_get_width() / 2 + offset, 160 + selection*40, NULL);
        rdpq_sprite_blit(button_a, 360 - offset, 420, NULL);
        rdpq_sprite_blit(button_b, 500 - offset, 420, NULL);

        rdpq_textparms_t parmstext = {0}; parmstext.valign = VALIGN_CENTER; parmstext.align = ALIGN_CENTER; parmstext.width = display_get_width(); parmstext.height = 80; parmstext.style_id = 1;
        rdpq_text_printf(&parmstext, 2, 0,40, dictstr("mm_title"));
        parmstext.height = 40;
        rdpq_text_printf(&parmstext, 3, 0 + offset,160, dictstr("mm_quick"));
        rdpq_text_printf(&parmstext, 3, 0 + offset,200, dictstr("mm_league"));
        rdpq_text_printf(&parmstext, 3, 0 + offset,240, dictstr("mm_options"));
        rdpq_text_printf(&parmstext, 3, 0 + offset,280, dictstr("mm_credits"));

        rdpq_textparms_t parms2; parms2.style_id = 2;
        rdpq_text_printf(&parms2, 3, 540- offset, 445, dictstr("mm_back")); 
        rdpq_text_printf(&parms2, 3, 400- offset, 445, dictstr("mm_select")); 
        rdpq_detach_show();
    }
    rspq_wait();
    if(background_block) {rspq_block_free(background_block); background_block = NULL;}
    if(background) {sprite_free(background); background = NULL;}

}

void menu_start(){
    if(!engine_eeprom_load_persistent() || !engine_eeprom_load_manual())
        state_init();
    engine_load_dictionary();
    font_setup();
    menu_logos();
    menu_cover();
    menu_main();
    rspq_wait();
    display_close();
    game_start(&maps[0]);
}
