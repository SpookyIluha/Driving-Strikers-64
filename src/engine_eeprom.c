#include <libdragon.h>
#include "intro.h"
#include "engine_gamestatus.h"
#include "audioutils.h"
#include "engine_eeprom.h"

void print_eeprom_error(int result){
    switch ( result )
    {
        case EEPFS_ESUCCESS:
        debugf( "Success!\n" );
            break;
        case EEPFS_EBADFS:
        debugf( "Failed with error: bad filesystem\n" );
        break;
        case EEPFS_ENOMEM: 
        debugf( "Failed with error: not enough memory\n" );
        break;
        case EEPFS_EBADINPUT:
        debugf( "Failed with error: Input parameters invalid\n" );
        break;
        case EEPFS_ENOFILE:
        debugf( "Failed with error: File does not exist\n" );
        break;
        case EEPFS_EBADHANDLE:
        debugf( "Failed with error: Invalid file handle\n" );
        break;
        case EEPFS_ECONFLICT:
        debugf( "Failed with error: Filesystem already initialized\n" );
        break;
        default:
        debugf( "Failed with error: Failed in an unexpected manner\n" );
        break;
    }
}

void engine_eeprom_init(){
    int result = 0;
    const eeprom_type_t eeprom_type = eeprom_present();
    if(eeprom_type == EEPROM_4K){
        const eepfs_entry_t eeprom_16k_files[] = {
            { "/persistent.sv",     (size_t)(sizeof(gamestatus.state_persistent))  },
            { "/manualsave.sv",     (size_t)(sizeof(gamestatus.state))  },
        };
        
        debugf( "EEPROM Detected: 4 Kibit (16 blocks)\n" );
        debugf( "Initializing EEPROM Filesystem...\n" );
        result = eepfs_init(eeprom_16k_files, 2);
    } else assertf(0, "EEPROM wrong format, expected 4KB. Please check your flashcart/emulator settings.");
    assertf(result == EEPFS_ESUCCESS, "EEPROM not found: 4 Kibit (16 blocks)\nPlease check your flashcart/emulator settings.");

    print_eeprom_error(result);
    joypad_poll(); auto held = joypad_get_buttons_held(JOYPAD_PORT_1);
    if ( !eepfs_verify_signature() || (held.l && held.r) )
    {
        /* If not, erase it and start from scratch */
        debugf( "Filesystem signature is invalid!\n" );
        debugf( "Wiping EEPROM...\n" );
        eepfs_wipe();
    }
}

void engine_eeprom_checksaves(){

}

void engine_eeprom_delete_saves(){
    gamestatus.state_persistent.manualsaved = false;
    gamestatus.state_persistent.autosaved = false;
    gamestatus.state_persistent.lastsavetype = SAVE_NONE;
    engine_eeprom_save_persistent();
}

void engine_eeprom_delete_persistent(){
    eepfs_wipe();
}

bool engine_eeprom_save_manual(){
    gamestatus.state_persistent.manualsaved = true;
    gamestatus.state_persistent.lastsavetype = SAVE_MANUALSAVE;
    engine_eeprom_save_persistent();
    debugf( "Writing '%s'\n", "/manualsave.sv" );
    const int result = eepfs_write("/manualsave.sv", &gamestatus.state, (size_t)(sizeof(gamestatus.state)));
    if(result != EEPFS_ESUCCESS || gamestatus.state.magicnumber != STATE_MAGIC_NUMBER) {
            print_eeprom_error(result);
            assertf(0, "eeprom file write unsuccessful: %s\n", gamestatus.state.magicnumber != STATE_MAGIC_NUMBER? "MAGIC number mismatch" : "eeprom error");
        }
    return result == EEPFS_ESUCCESS;
}

bool engine_eeprom_load_manual(){
    debugf( "Reading '%s'\n", "/manualsave.sv" );
    
    const int result = eepfs_read("/manualsave.sv", &gamestatus.state, (size_t)(sizeof(gamestatus.state)));
    if(result != EEPFS_ESUCCESS || gamestatus.state.magicnumber != STATE_MAGIC_NUMBER) 
        {
            print_eeprom_error(result);
            debugf("eeprom file read unsuccessful: %s\n", gamestatus.state.magicnumber != STATE_MAGIC_NUMBER? "MAGIC number mismatch" : "eeprom error");
            state_init();
        }
    return result == EEPFS_ESUCCESS;
}

bool engine_eeprom_save_persistent(){
    debugf( "Writing '%s'\n", "/persistent.sv" );
    
    const int result = eepfs_write("/persistent.sv", &gamestatus.state_persistent, (size_t)(sizeof(gamestatus.state_persistent)));
    if(result != EEPFS_ESUCCESS || gamestatus.state_persistent.magicnumber != STATE_PERSISTENT_MAGIC_NUMBER) 
        {
            print_eeprom_error(result);
            debugf("eeprom file write unsuccessful: %s\n", gamestatus.state_persistent.magicnumber != STATE_MAGIC_NUMBER? "MAGIC number mismatch" : "eeprom error");
        }
    return result == EEPFS_ESUCCESS;
}

bool engine_eeprom_load_persistent(){
    debugf( "Reading '%s'\n", "/persistent.sv" );
    
    const int result = eepfs_read("/persistent.sv", &gamestatus.state_persistent, (size_t)(sizeof(gamestatus.state_persistent)));
    if(result != EEPFS_ESUCCESS || gamestatus.state_persistent.magicnumber != STATE_PERSISTENT_MAGIC_NUMBER) 
        {
            print_eeprom_error(result);
            debugf("eeprom file read unsuccessful: %s\n", gamestatus.state_persistent.magicnumber != STATE_MAGIC_NUMBER? "MAGIC number mismatch" : "eeprom error");
            state_init();
        }
    return result == EEPFS_ESUCCESS;
}