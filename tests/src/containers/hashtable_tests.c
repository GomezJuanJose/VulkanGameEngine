#include "hashtable_tests.h"
#include "../test_manager.h"
#include "../expect.h"

#include <defines.h>
#include <containers/hashtable.h>

u8 hashtable_should_create_and_destroy(){
    hashtable table;
    u64 element_size = sizeof(u64);
    u64 element_count = 3;
    u64 memory[3];
    
    hashtable_create(element_size, element_count, memory, FALSE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(u64), table.element_size);
    expect_should_be(3, table.element_count);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

u8 hashtable_should_set_and_get_successfully(){
    hashtable table;
    u64 element_size = sizeof(u64);
    u64 element_count = 3;
    u64 memory[3];

    hashtable_create(element_size, element_count, memory, FALSE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(u64), table.element_size);
    expect_should_be(3, table.element_count);

    u64 testval1 = 23;
    hashtable_set(&table, "test1", &testval1);
    u64 get_testval_1 = 0;
    hashtable_get(&table, "test1", &get_testval_1);
    expect_should_be(testval1, get_testval_1);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

typedef struct ht_test_struct{
    b8 b_value;
    f32 f_value;
    u64 u_value;
} ht_test_struct;

u8 hashtable_should_set_and_get_ptr_successfully (){
    hashtable table;
    u64 element_size = sizeof(ht_test_struct*);
    u64 element_count = 3;
    ht_test_struct* memory[3];

    hashtable_create(element_size, element_count, memory, TRUE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(ht_test_struct*), table.element_size);
    expect_should_be(3, table.element_count);

    ht_test_struct t;
    ht_test_struct* testval1 = &t;
    testval1->b_value = TRUE;
    testval1->u_value = 63;
    testval1->f_value = 3.1415f;
    hashtable_set_ptr(&table, "test1", (void**)&testval1);

    ht_test_struct* get_testval_1 = 0;
    hashtable_get_ptr(&table, "test1", (void**)&get_testval_1);

    expect_should_be(testval1->b_value, get_testval_1->b_value);
    expect_should_be(testval1->u_value, get_testval_1->u_value);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

u8 hashtable_should_set_and_get_nonexistant(){
    hashtable table;
    u64 element_size = sizeof(u64);
    u64 element_count = 3;
    u64 memory[3];

    hashtable_create(element_size, element_count, memory, FALSE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(u64), table.element_size);
    expect_should_be(3, table.element_count);

    u64 testval1 = 23;
    hashtable_set(&table, "test1", &testval1);
    u64 get_testval_1 = 0;
    hashtable_get(&table, "test2", &get_testval_1);
    expect_should_be(0, get_testval_1);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

u8 hashtable_should_set_and_get_ptr_nonexistant(){
    hashtable table;
    u64 element_size = sizeof(ht_test_struct*);
    u64 element_count = 3;
    ht_test_struct* memory[3];

    hashtable_create(element_size, element_count, memory, TRUE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(ht_test_struct*), table.element_size);
    expect_should_be(3, table.element_count);

    ht_test_struct t;
    ht_test_struct* testval1 = &t;
    testval1->b_value = TRUE;
    testval1->u_value = 63;
    testval1->f_value = 3.1415f;
    b8 result = hashtable_set_ptr(&table, "test1", (void**)&testval1);
    expect_to_be_true(result);

    ht_test_struct* get_testval_1 = 0;
    result = hashtable_get_ptr(&table, "test2", (void**)&get_testval_1);
    expect_to_be_false(result);
    expect_should_be(0, get_testval_1);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

u8 hashtable_should_set_and_unset_ptr(){
    hashtable table;
    u64 element_size = sizeof(ht_test_struct*);
    u64 element_count = 3;
    ht_test_struct* memory[3];

    hashtable_create(element_size, element_count, memory, TRUE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(ht_test_struct*), table.element_size);
    expect_should_be(3, table.element_count);

    ht_test_struct t;
    ht_test_struct* testval1 = &t;
    testval1->b_value = TRUE;
    testval1->u_value = 63;
    testval1->f_value = 3.1415f;
    // Set it
    b8 result = hashtable_set_ptr(&table, "test1", (void**)&testval1);
    expect_to_be_true(result);

    // Check that it exists and is correct
    ht_test_struct* get_testval_1 = 0;
    hashtable_get_ptr(&table, "test1", (void**)&get_testval_1);
    expect_should_be(testval1->b_value, get_testval_1->b_value);
    expect_should_be(testval1->u_value, get_testval_1->u_value);
    
    // Unset it
    result = hashtable_set_ptr(&table, "test1", 0);
    expect_to_be_true(result);

    // Should no longer found.
    ht_test_struct* get_testval_2 = 0;
    result = hashtable_get_ptr(&table, "test1", (void**)&get_testval_2);
    expect_to_be_false(result);
    expect_should_be(0, get_testval_2);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

u8 hashtable_try_call_non_ptr_on_ptr_table (){
    hashtable table;
    u64 element_size = sizeof(ht_test_struct*);
    u64 element_count = 3;
    ht_test_struct* memory[3];

    hashtable_create(element_size, element_count, memory, TRUE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(ht_test_struct*), table.element_size);
    expect_should_be(3, table.element_count);

    TDEBUG("The following 2 error messages are intentional.");
    
    ht_test_struct t;
    t.b_value = TRUE;
    t.u_value = 63;
    t.f_value = 3.1415f;
    // Try setting the record
    b8 result = hashtable_set(&table, "test1", &t);
    expect_to_be_false(result);

    // Try getting the record.
    ht_test_struct* get_testval1 = 0;
    result = hashtable_get(&table, "test1", (void**)&get_testval1);
    expect_to_be_false(result);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

u8 hashtable_try_call_ptr_on_non_ptr_table(){
    hashtable table;
    u64 element_size = sizeof(ht_test_struct);
    u64 element_count = 3;
    ht_test_struct memory[3];

    hashtable_create(element_size, element_count, memory, FALSE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(ht_test_struct), table.element_size);
    expect_should_be(3, table.element_count);

    TDEBUG("The following 2 error messages are intentional.");
    
    ht_test_struct t;
    ht_test_struct* testval1 = &t;
    testval1->b_value = TRUE;
    testval1->u_value = 63;
    testval1->f_value = 3.1415f;
    // Try setting the record
    b8 result = hashtable_set_ptr(&table, "test1", (void**)&testval1);
    expect_to_be_false(result);

    // Try getting the record.
    ht_test_struct* get_testval1 = 0;
    result = hashtable_get_ptr(&table, "test1", (void**)&get_testval1);
    expect_to_be_false(result);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

u8 hashtable_should_set_get_and_update_ptr_successfully(){
    hashtable table;
    u64 element_size = sizeof(ht_test_struct*);
    u64 element_count = 3;
    ht_test_struct* memory[3];

    hashtable_create(element_size, element_count, memory, TRUE, &table);

    expect_should_not_be(0, table.memory);
    expect_should_be(sizeof(ht_test_struct*), table.element_size);
    expect_should_be(3, table.element_count);

    ht_test_struct t;
    ht_test_struct* testval1 = &t;
    testval1->b_value = TRUE;
    testval1->u_value = 63;
    testval1->f_value = 3.1415f;
    hashtable_set_ptr(&table, "test1", (void**)&testval1);
    
    ht_test_struct* get_testval1 = 0;
    hashtable_get_ptr(&table, "test1", (void**)&get_testval1);
    expect_should_be(testval1->b_value, get_testval1->b_value);
    expect_should_be(testval1->u_value, get_testval1->u_value);

    // Update pointed-to values
    get_testval1->b_value = FALSE;
    get_testval1->u_value = 99;
    get_testval1->f_value = 6.69f;

    // Get the pointer again and confirm correct values
    ht_test_struct* get_testval2 = 0;
    hashtable_get_ptr(&table, "test1", (void**)&get_testval2);
    expect_to_be_false(get_testval2->b_value);
    expect_should_be(99, get_testval2->u_value);
    expect_float_to_be(6.69f, get_testval2->f_value);

    hashtable_destroy(&table);

    expect_should_be(0, table.memory);
    expect_should_be(0, table.element_size);
    expect_should_be(0, table.element_count);

    return TRUE;
}

void hashtable_register_tests(){
    test_manager_register_test(hashtable_should_create_and_destroy, "Hashtable should create and destroy");
    test_manager_register_test(hashtable_should_set_and_get_successfully, "Hashtable should set and get");
    test_manager_register_test(hashtable_should_set_and_get_ptr_successfully, "Hashtable should set and get pointer");
    test_manager_register_test(hashtable_should_set_and_get_nonexistant, "Hashtable should set and get non-existent entry as nothing");
    test_manager_register_test(hashtable_should_set_and_get_ptr_nonexistant, "Hashtable should set and get non-existent pointer entry as nothing");
    test_manager_register_test(hashtable_should_set_and_unset_ptr, "Hashtable should set and unset pointer entry as nothing");
    test_manager_register_test(hashtable_try_call_non_ptr_on_ptr_table, "Hashtable try call non-pointer functions on pointer type table");
    test_manager_register_test(hashtable_try_call_ptr_on_non_ptr_table, "Hashtable try call pointer functions on non-pointer type table");
    test_manager_register_test(hashtable_should_set_get_and_update_ptr_successfully, "Hashtable should get pointer, update and get again successfully");
}