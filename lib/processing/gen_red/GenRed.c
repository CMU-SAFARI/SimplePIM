#include "GenRed.h"

/*
    description of table_gen_red see GenRed.h
    other functions are helper functions used by the framework
*/

void combine_table_entries(void* table1, void* table2, uint32_t table_size, uint32_t value_size, void (*combineFunc)(void*, void*)){
    
    uint32_t curr_entry;
 
    for(int i=0; i<table_size; ++i){
        curr_entry = i*(value_size);
        (*combineFunc)(table1+curr_entry, table2+curr_entry);
        
    } 

}

void gather_tables_to_host(simplepim_management_t* table_management, void* my_table, uint32_t len, uint32_t type_size, uint32_t curr_offset_on_mram, void (*init_func)(uint32_t, void*) ,void (*combineFunc)(void*, void*)){
    int i;
    struct dpu_set_t dpu;
    struct dpu_set_t set = table_management->set;
    uint32_t num_dpus = table_management->num_dpus;
    void* tables = malloc_reduce_aligned(len, type_size, table_management);
    uint32_t aligned_table_size = (len*type_size)+(len*type_size)%8;

    for(int i=0; i<len; i++){
        (*init_func)(type_size, my_table+i*type_size);
    }



	DPU_FOREACH(set, dpu, i) {
	    
		DPU_ASSERT(dpu_prepare_xfer(dpu, tables+i*aligned_table_size));
	}
   DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, curr_offset_on_mram, aligned_table_size, DPU_XFER_DEFAULT));
    
    uint32_t omp_threads = 8;
    uint32_t thread_id;
    uint32_t table_size = len*type_size;
    uint32_t curr_entry;

    void* omp_helper_tables = malloc(table_size*omp_threads);


    
    omp_set_dynamic(0);     // Explicitly disable dynamic teams private(omp_table, thread_id) 
    #pragma omp parallel num_threads(omp_threads) 
    {
        void* omp_table;
        void* curr_table;

        thread_id = omp_get_thread_num();
        omp_table = omp_helper_tables+table_size*thread_id;

        for(int i=0; i<len; i++){
            (*init_func)(type_size, omp_table+i*type_size);
        }

    #pragma omp for
        for(int i=0; i<num_dpus; i++){
            curr_table = (void*)(tables+i*aligned_table_size);
            combine_table_entries(omp_table, curr_table, len, type_size, combineFunc);  
        }
        

        #pragma omp barrier
    }

    void* table;
    for(int i=0; i<omp_threads; i++){
        table = (void*)(omp_helper_tables+i*table_size);
        combine_table_entries(my_table, table, len, type_size, combineFunc);
    }
    

    free(omp_helper_tables);
    free(tables);

}

void* table_gen_red(const char* src_name, const char* dest_name, uint32_t output_type, uint32_t output_len, handle_t* binary_handle, simplepim_management_t* table_management, uint32_t info){
    int i;
    struct dpu_set_t dpu;
    struct timeval start_time;
    struct timeval end_time;
    uint32_t outputs = table_management->free_space_start_pos;
    if(contains_table(dest_name, table_management)){
        outputs = lookup_table(dest_name, table_management) -> start;
    }
    
    if(binary_handle->func_type == 1){
        if(!contains_table(src_name, table_management)){
            printf("source table ");
            printf(src_name);
            printf(" is not contains in current management unit\n");
            return NULL;
        }

        //timing
        gettimeofday(&start_time, NULL);

        struct dpu_set_t set = table_management->set;
        uint32_t num_dpus = table_management->num_dpus;
        table_host_t* src_table = lookup_table(src_name, table_management);
        uint32_t* lens = src_table->lens_each_dpu;
        uint32_t input_type = src_table->table_type_size;
        uint32_t inputs = src_table->start;

        gen_red_arguments_t* input_args = table_management->red_args;
        // use handle for precompiled binaries
        const char* binary = binary_handle->bin_location;
        DPU_ASSERT(dpu_load(set, binary, NULL));

        for(int i=0; i<num_dpus; i++){
            input_args[i].input_start_offset = inputs;
            input_args[i].input_type_size = input_type;
            input_args[i].output_start_offset = outputs;
            input_args[i].output_type_size = output_type;
            input_args[i].len = lens[i];
            input_args[i].table_len = output_len;
            input_args[i].info = info;
        }
    
        //parse arguments to map function call
        int i;
        struct dpu_set_t dpu;
	    DPU_FOREACH(set, dpu, i) {
		    DPU_ASSERT(dpu_prepare_xfer(dpu, input_args + i));
	    }

        DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "GEN_RED_INPUT_ARGUMENTS", 0, sizeof(gen_red_arguments_t), DPU_XFER_DEFAULT));
        
        gettimeofday(&end_time, NULL);
        double prepare_args_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);
        
        //call red function
        gettimeofday(&start_time, NULL);
        DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
        gettimeofday(&end_time, NULL);

        double kernel_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);

        // reduction on cpu
        gettimeofday(&start_time, NULL);
        void* my_table = malloc(output_len*output_type);

        void* lib=dlopen(binary_handle->so_bin_location, RTLD_NOW);
        void (*init_func)(uint32_t, void*) = dlsym(lib,"init_func");
        void (*combine_func)(void*, void*) = dlsym(lib, "combine_func");

        if(lib == NULL){
        printf("dynamic library linking failed!!!\n");
        }

        
        gather_tables_to_host(table_management, my_table, output_len, output_type, outputs, init_func, combine_func);
        dlclose(lib);
        gettimeofday(&end_time, NULL);
        double host_table_reduction_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);
        // back to dpus

        // table info
        gettimeofday(&start_time, NULL);

        int32_t* red_tables_lens = malloc(sizeof(uint32_t)*num_dpus);
        for(int i=0; i<num_dpus; i++){
            red_tables_lens[i] = output_len;
        }

        // table information to management unit
        table_host_t* t = malloc(sizeof(table_host_t));
        t->name = malloc(strlen(dest_name)+1);
        memcpy(t->name, dest_name, strlen(dest_name)+1);
        t->start = outputs;
        uint32_t max_end_dpu = outputs+output_len*output_type;
        t->end = max_end_dpu+(8-max_end_dpu%8);
        t->len = output_len;
        t->table_type_size = output_type;
        t->lens_each_dpu = red_tables_lens;
        t->is_virtual_zipped = 0;

        add_table(t, table_management);
	table_management->free_space_start_pos = table_management->free_space_start_pos > t->end ? table_management->free_space_start_pos : t->end;
        gettimeofday(&end_time, NULL);
        double register_table_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);
        
        printf("--------------\n");
        printf("table reduction function : ");
        printf(binary);
        printf("\nreduction function kernel execution time : %f\n", kernel_time/1000);
        printf("host reduction execution time : %f\n", host_table_reduction_time/1000);
        printf("function call and table management time : %f\n", (register_table_time+prepare_args_time)/1000);
        printf("--------------\n");

        return my_table;
    }
    else{
        printf("ERROR: compiled binary ");
        printf(binary_handle->bin_location);
        printf(" does not contain general reduction functions\n");
    }
    
}
