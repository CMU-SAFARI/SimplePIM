#include "Map.h"

void table_map(const char* src_name, const char* dest_name, uint32_t output_type, handle_t* binary_handle, simplepim_management_t* table_management, uint32_t info){
    int i;
    struct dpu_set_t dpu;
    struct timeval start_time;
    struct timeval end_time;
    
    uint32_t outputs = table_management->free_space_start_pos;
    if(contains_table(dest_name, table_management)){
        outputs = lookup_table(dest_name, table_management) -> start;
    }

    if(!contains_table(src_name, table_management)){
            printf("source table ");
            printf(src_name);
            printf(" is not contains in current management unit\n");
            return;
    }
    table_host_t* src_table = lookup_table(src_name, table_management);

    if(binary_handle->func_type == 0 && src_table->is_virtual_zipped == 0){

        //timing
        gettimeofday(&start_time, NULL);

        struct dpu_set_t set = table_management->set;
        uint32_t num_dpus = table_management->num_dpus;
        uint32_t* lens = src_table->lens_each_dpu;
        uint32_t input_type = src_table->table_type_size;
        uint32_t inputs = src_table->start;

        map_arguments_t* input_args = table_management->map_args;
        // use handle for precompiled binaries
        const char* binary = binary_handle->bin_location;
        DPU_ASSERT(dpu_load(set, binary, NULL));
    
        //parse arguments to map function call
	    DPU_FOREACH(set, dpu, i) {
	        input_args[i].input_start_offset = inputs;
            input_args[i].input_type_size = input_type;
            input_args[i].output_start_offset = outputs;
            input_args[i].output_type_size = output_type;
            input_args[i].len = lens[i];
            input_args[i].info = info;
            input_args[i].is_virtually_zipped = 0;
		    DPU_ASSERT(dpu_prepare_xfer(dpu, input_args + i));
	    }

        DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "MAP_INPUT_ARGUMENTS", 0, sizeof(map_arguments_t), DPU_XFER_DEFAULT));
    
        gettimeofday(&end_time, NULL);
        double prepare_args_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);
                
        //call map function
        gettimeofday(&start_time, NULL);
        DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
        gettimeofday(&end_time, NULL);

        double kernel_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);

        // table information to management unit
        // timing
        gettimeofday(&start_time, NULL);

        table_host_t* t = malloc(sizeof(table_host_t));
        t->name = malloc(strlen(dest_name)+1);
        memcpy(t->name, dest_name, strlen(dest_name)+1);
        t->start = outputs;
        uint32_t max_end_dpu = max_len_dpu(num_dpus, src_table)*output_type+outputs;
        t->end = max_end_dpu+(8-max_end_dpu%8);
        t->len = src_table->len;
        t->table_type_size = output_type;
        t->lens_each_dpu = malloc(num_dpus*sizeof(int32_t));
        t->is_virtual_zipped = 0;
        memcpy(t->lens_each_dpu, lens, num_dpus*sizeof(int32_t));
	
        add_table(t, table_management);
    	table_management->free_space_start_pos = table_management->free_space_start_pos > t->end ? table_management->free_space_start_pos : t->end;

        gettimeofday(&end_time, NULL);
        double register_table_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);

        printf("--------------\n");
        printf("map function : ");
        printf(binary);
        printf("\nmap function kernel execution time : %f\n", kernel_time/1000);
        printf("function call and table management time : %f\n", (register_table_time+prepare_args_time)/1000);
        printf("--------------\n");
    }
    else if (binary_handle->func_type == 0 && src_table->is_virtual_zipped == 1)
    {
         //timing
        gettimeofday(&start_time, NULL);

        struct dpu_set_t set = table_management->set;
        uint32_t num_dpus = table_management->num_dpus;
        uint32_t* lens = src_table->lens_each_dpu;
        uint32_t input_type = src_table->table_type_size;
        uint32_t inputs = src_table->start;

        map_arguments_t* input_args = table_management->map_args;
        // use handle for precompiled binaries
        const char* binary = binary_handle->bin_location;
        DPU_ASSERT(dpu_load(set, binary, NULL));
    
        //parse arguments to map function call
	    DPU_FOREACH(set, dpu, i) {
	        input_args[i].input_start_offset = inputs;
            input_args[i].input_type_size = input_type;
            input_args[i].output_start_offset = outputs;
            input_args[i].output_type_size = output_type;
            input_args[i].len = lens[i];
            input_args[i].info = info;
            input_args[i].is_virtually_zipped = 1;
            input_args[i].start1 = src_table->start1;
            input_args[i].start2 = src_table->start2;
            input_args[i].type1 = src_table->type1;
            input_args[i].type2 = src_table->type2;
		    DPU_ASSERT(dpu_prepare_xfer(dpu, input_args + i));
	    }

        DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "MAP_INPUT_ARGUMENTS", 0, sizeof(map_arguments_t), DPU_XFER_DEFAULT));
    
        gettimeofday(&end_time, NULL);
        double prepare_args_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);
                
        //call map function
        gettimeofday(&start_time, NULL);
        DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
        gettimeofday(&end_time, NULL);

        double kernel_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);

        // table information to management unit
        // timing
        gettimeofday(&start_time, NULL);

        table_host_t* t = malloc(sizeof(table_host_t));
        t->name = malloc(strlen(dest_name)+1);
        memcpy(t->name, dest_name, strlen(dest_name)+1);
        t->start = outputs;
        uint32_t max_end_dpu = max_len_dpu(num_dpus, src_table)*output_type+outputs;
        t->end = max_end_dpu+(8-max_end_dpu%8);
        t->len = src_table->len;
        t->table_type_size = output_type;
        t->lens_each_dpu = malloc(num_dpus*sizeof(int32_t));
        t->is_virtual_zipped = 0;
        memcpy(t->lens_each_dpu, lens, num_dpus*sizeof(int32_t));

        add_table(t, table_management);
    

        gettimeofday(&end_time, NULL);
        double register_table_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);

        printf("--------------\n");
        printf("map function : ");
        printf(binary);
        printf("\nmap function kernel execution time : %f\n", kernel_time/1000);
        printf("function call and table management time : %f\n", (register_table_time+prepare_args_time)/1000);
        printf("--------------\n");
    }
    else{
        printf("ERROR: compiled binary ");
        printf(binary_handle->bin_location);
        printf(" does not contain map function\n");
    }
    
}
