#include <core/logger.h>
#include <core/asserts.h>

// TODO: Test
#include <platform/platform.h>

int main(void){
    TFATAL("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TERROR("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TWARN("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TINFO("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TDEBUG("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TTRACE("Se abre Taller %i-%i-%i", 15, 10, 2024);

    platform_state state;
    if(platform_startup(&state, "Taller Engine Testbed", 100, 100, 1280, 720)){
        while(TRUE){
            platform_pump_messages(&state);
        }
    }
    platform_shutdow(&state);

    return 0;
}