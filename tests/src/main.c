#include "test_manager.h"

#include "memory/linear_allocator_tests.h"
#include "memory/dynamic_allocator_tests.h"

#include "containers/hashtable_tests.h"
#include "containers/freelist_tests.h"

#include <core/logger.h>

int main(){
    // Always initialize the test manager first.
    test_manager_init();

    //TODO: add test registrations here.
    linear_allocator_register_tests();
    dynamic_allocator_register_tests();
    hashtable_register_tests();
    freelist_register_tests();

    TDEBUG("Starting tests...");

    // Execute tests
    test_manager_run_tests();

    return 0;
}