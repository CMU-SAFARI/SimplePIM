#include "Table.h"


void init_table(table_t* t, uint32_t table_size, uint32_t value_size, void (*init_func)(uint32_t, void*)){
    t->table_size = table_size;
    t->value_size = value_size;
    t->table_allocator = fsb_alloc(table_size*value_size+(table_size*value_size)%8, 1);
    t->locks_allocator = NULL;
    t->table = fsb_get(t->table_allocator);
    t->locks = NULL;
    t->heap_ptr = NULL;
    void* tmp = t->table;

    for(int i=0; i<table_size; i++){
        init_func(value_size, tmp);
        tmp += value_size;
    }
}

void insert_table(table_t* t, uint32_t key, void* value, void (*combineFunc)(void*, void*)){
    uint32_t value_size = t->value_size;
    uint32_t curr_entry = key*value_size;
    void* value_ptr_in_table = t->table+curr_entry;

    (*combineFunc)(value_ptr_in_table, value);

}

void free_table(table_t* t){
    fsb_free(t->table_allocator, t->table);
}


void print_table(table_t* t, void (*printFunc)(void*)){
    uint32_t table_size = t->table_size;
    uint32_t value_size = t->value_size;
    print_table_entries(t->table, table_size, value_size, printFunc); 
}


void combine_tables(table_t* t1, table_t* t2, void (*combineFunc)(void*, void*)){
    uint32_t table_size = t1->table_size;
    uint32_t value_size = t1->value_size;
    void* table1 = t1->table;
    void* table2 = t2->table;
    combine_table_entries(table1, table2, table_size, value_size, combineFunc);
}

uint32_t d = 32;
void print_entry(void* p){
    int* int_p = (int*)p;
    printf("%d, ", *int_p);

    p += sizeof(int32_t);
    int_p = (int*)p;
    
    for(int i=0; i<d; i++){
        printf(" %d", int_p[i]);
    }
    printf("\n");
    
}

__dma_aligned table_t* global_table_helper;
BARRIER_INIT(barrier, NR_TASKLETS);
void combine_tables_lockfree(__mram_ptr void* table_entries_mram, table_t* local_table, void (*init_func)(uint32_t, void*), void (*combineFunc)(void*, void*)){
    uint32_t num_tasklets = NR_TASKLETS; 
    uint32_t pid = me();

    uint32_t table_size = local_table->table_size;
    uint32_t value_size = local_table->value_size;
    barrier_wait(&barrier);

    
    if(num_tasklets==1){
        //global table is empty&&on heap, and store local table on heap
        store_shared_table_on_heap(local_table, table_entries_mram);
    }
    else{

        
        // a helper table for global merge
        fsb_allocator_t global_table_allocator;
        if(pid==0){
            global_table_allocator = fsb_alloc(sizeof(table_t), 1);
            global_table_helper = (table_t*)fsb_get(global_table_allocator);
            init_table(global_table_helper, table_size, value_size, init_func);
        } 
        barrier_wait(&barrier);
        //
        uint32_t local_len = table_size / num_tasklets;
        uint32_t rest = table_size % num_tasklets;
        uint32_t curr_id;
        uint32_t curr_len;
        uint32_t start_pos;
        void* global_table_ptr = global_table_helper->table;
        void* local_table_ptr = local_table->table;
    
        for(int i=0; i<num_tasklets; i++){
            curr_id = (pid+i)%num_tasklets;
            curr_len = curr_id<rest?(local_len+1):local_len;
            start_pos = curr_id<rest?(local_len+1)*curr_id:local_len*curr_id+rest;
            combine_table_entries(global_table_ptr+start_pos*value_size, local_table_ptr+start_pos*value_size, curr_len, value_size, combineFunc);
            
           barrier_wait(&barrier);
        }

    
        uint32_t transfer_size = table_size*value_size+(table_size*value_size)%8;
        store_arr_aligned(global_table_helper->table, table_entries_mram, transfer_size);
        //print_shared_table(global_table_helper, print_entry);
        if(pid == 0){
            free_table(global_table_helper);
            fsb_free(global_table_allocator, global_table_helper);
        }
        
    }


}


