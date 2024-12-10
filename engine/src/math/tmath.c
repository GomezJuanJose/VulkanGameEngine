#include "tmath.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 rand_seeded = FALSE;

/**
 * Note that these are here in order to prevent having to import the
 * entire <math.h> everywhere.
 */
f32 tsin(f32 x){
    return sinf(x);
}

f32 tcos(f32 x){
    return cosf(x);
}

f32 ttan(f32 x){
    return tanf(x);
}

f32 tacos(f32 x){
    return acosf(x);
}

f32 tsqrt(f32 x){
    return sqrtf(x);
}

f32 tabs(f32 x){
    return fabsf(x);
}

i32 trandom(){
    if(!rand_seeded){
        srand((u32)platform_get_absolute_time());
        rand_seeded = TRUE;
    }
    return rand();
}

i32 trandom_in_range(i32 min, i32 max){
    if(!rand_seeded){
        srand((u32)platform_get_absolute_time());
        rand_seeded = TRUE;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 ftrandom(){
    return (float)trandom() / (f32)RAND_MAX;
}

f32 ftrandom_in_range(f32 min, f32 max){
    return min + ((float)trandom() / ((f32)RAND_MAX / (max - min)));
}