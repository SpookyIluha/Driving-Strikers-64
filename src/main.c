#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "engine_gfx.h"
#include "teams.h"
#include "entity_logic.h"
#include "audioutils.h"
#include "effects.h"
#include "playtime_logic.h"
#include "maps.h"
#include "menu.h"
#include "intro.h"
#include "engine_eeprom.h"
#include "engine_locale.h"

void setup(){
  //debug_init_isviewer();
	//debug_init_usblog();
  wav64_init_compression(3);

  dfs_init(DFS_DEFAULT_LOCATION);
  joypad_init();
  audio_init(48000, 6);
  mixer_init(10);
  vi_init();

  rdpq_init();
  state_init();
  effects_init();
  engine_eeprom_init();
  engine_load_languages();

  t3d_init((T3DInitParams){});
  tpx_init((TPXInitParams){});

  srand(getentropy32());
  register_VI_handler((void(*)())rand);

  effects_rumble_stop();

  libdragon_logo();
  effects_rumble_stop();
  if(is_memory_expanded()){
      display_init((resolution_t){.width = 640, .height = 480, .interlaced = INTERLACE_RDP}, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_DEDITHER);
  }
  else display_init(RESOLUTION_640x480, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_DEDITHER);

  if(get_tv_type() == TV_PAL) {
      vi_set_borders((vi_borders_t){.up = 48, .down = 48});
      vi_set_yscale_factor(2.0f);
  }
  audio_prewarm_all();

#if DEBUG_RDP
    rdpq_debug_start();
    rdpq_debug_log(true);
#endif
}

int main()
{
  setup();
  menu_start();
  return 0;
}
