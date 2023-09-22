#include "TableShared.h"

// table in this file must be the same table seen by all threads

BARRIER_INIT(barrier_shared, NR_TASKLETS);
MUTEX_INIT(mutex_shared);

// multithreaded load and store, len must be 8 bytes aligned, can operate on more than 2kB data
uint32_t curr_block_shared;
void load_arr_aligned(void* arr, __mram_ptr void* heap_ptr, uint32_t len){
    uint32_t transfer_each_time = 256; //must be multiple of 8
    uint32_t transfer_size = len;
    curr_block_shared = 0;

    uint32_t curr_offset;
    uint32_t curr_block_local;
    uint32_t last_transfer_size = (transfer_size%transfer_each_time==0)?transfer_each_time:transfer_size/transfer_size%transfer_each_time;
    uint32_t num_blocks = (transfer_size%transfer_each_time==0)?transfer_size/transfer_each_time:transfer_size/transfer_each_time+1;
    
    
    barrier_wait(&barrier_shared);
    while (curr_block_shared<num_blocks)
    {
        mutex_lock(mutex_shared);
        curr_block_local = curr_block_shared;
        curr_block_shared++; // current lock being processed
        mutex_unlock(mutex_shared);

        
        curr_offset = curr_block_local*transfer_each_time;
        if(curr_block_local == num_blocks-1){
            mram_read(heap_ptr+curr_offset, arr+curr_offset,  last_transfer_size);
        }
        else{
            mram_read(heap_ptr+curr_offset, arr+curr_offset,  transfer_each_time);
        }
        

    }
    barrier_wait(&barrier_shared);
    
}

void store_arr_aligned(void* arr, __mram_ptr void* heap_ptr, uint32_t len){
    uint32_t transfer_each_time = 256; //must be multiple of 8
    uint32_t transfer_size = len;
    curr_block_shared = 0;

    uint32_t curr_offset;
    uint32_t curr_block_local;
    uint32_t last_transfer_size = (transfer_size%transfer_each_time==0)?transfer_each_time:transfer_size/transfer_size%transfer_each_time;
    uint32_t num_blocks = (transfer_size%transfer_each_time==0)?transfer_size/transfer_each_time:transfer_size/transfer_each_time+1;

    barrier_wait(&barrier_shared);
    while (curr_block_shared<num_blocks)
    {
        mutex_lock(mutex_shared);
        curr_block_local = curr_block_shared;
        curr_block_shared++; // current lock being processed
        mutex_unlock(mutex_shared);
        curr_offset = curr_block_local*transfer_each_time;

        if(curr_block_local == num_blocks-1){
            mram_write(arr+curr_offset, heap_ptr+curr_offset,  last_transfer_size);
        }
        else{
            mram_write(arr+curr_offset, heap_ptr+curr_offset,  transfer_each_time);
        }

    }
    barrier_wait(&barrier_shared);
}

// shared table operations
void init_shared_table(table_t* t, uint32_t table_size, uint32_t value_size, void (*init_func)(uint32_t, void*)){
    uint32_t pid = me();
    if(pid==0){
        t->table_size = table_size;
        t->value_size = value_size;
        t->table_allocator = fsb_alloc(table_size*value_size+(table_size*value_size)%8, 1);
        t->locks_allocator = fsb_alloc(table_size*sizeof(uint8_t), 1);
        t->table = fsb_get(t->table_allocator);
        t->locks = fsb_get(t->locks_allocator);
        t->heap_ptr = NULL;
        void* tmp = t->table;

        for(int i=0; i<table_size; i++){
            init_func(value_size, tmp);
            tmp += value_size;
        }

        for(int i=0; i<table_size; i++){
            mutex_unlock(&(t->locks[i]));
        }
    }

    barrier_wait(&barrier_shared);
}

void insert_shared_table(table_t* t, uint32_t key, void* value, void (*combineFunc)(void*, void*)){
    uint32_t value_size = t->value_size;
    uint32_t curr_entry = key*value_size;
    void* value_ptr_in_table = (t->table)+curr_entry;
    mutex_lock(&(t->locks[key]));
    (*combineFunc)(value_ptr_in_table, value);
    mutex_unlock(&(t->locks[key]));
}

void print_shared_table(table_t* t, void (*printFunc)(void*)){
    uint32_t pid = me();
    if(pid==0){
        uint32_t table_size = t->table_size;
        uint32_t value_size = t->value_size;
        print_table_entries(t->table, table_size, value_size, printFunc); 
    }
    barrier_wait(&barrier_shared);
}

void free_shared_table(table_t* t){
    fsb_free(t->table_allocator, t->table);
    fsb_free(t->locks_allocator, t->locks);
}

uint32_t store_shared_table_on_heap(table_t* table, __mram_ptr void* heap_ptr){
    uint32_t table_size = table->table_size;
    uint32_t value_size = table->value_size;
    uint32_t transfer_size = table_size*value_size+(table_size*value_size)%8;
    store_arr_aligned(table->table, heap_ptr, transfer_size);
    return (uint32_t)(heap_ptr-DPU_MRAM_HEAP_POINTER+transfer_size);
}

void load_shared_table_from_heap(table_t* table,  __mram_ptr void* heap_ptr){
    uint32_t table_size = table->table_size;
    uint32_t value_size = table->value_size;
    uint32_t transfer_size = table_size*value_size+(table_size*value_size)%8;
    load_arr_aligned(table->table, heap_ptr, transfer_size);
}
