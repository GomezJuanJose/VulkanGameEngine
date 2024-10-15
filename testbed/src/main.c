#include <core/logger.h>
#include <core/asserts.h>

int main(void){
    TFATAL("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TERROR("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TWARN("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TINFO("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TDEBUG("Se abre Taller %i-%i-%i", 15, 10, 2024);
    TTRACE("Se abre Taller %i-%i-%i", 15, 10, 2024);

    TASSERT(1 == 0)

    return 0;
}