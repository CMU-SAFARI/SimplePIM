#include "Zip.h"

void table_zip(const char* src1_name, const char* src2_name, const char* dest_name, handle_t* binary_handle, simplepim_management_t* table_management){
     int i;
     struct dpu_set_t dpu;
     struct timeval start_time;
     struct timeval end_time;
     uint32_t outputs = table_management->free_space_start_pos;
     if(contains_table(dest_name, table_management)){
        outputs = lookup_table(dest_name, table_management) -> start;
     }

     if(binary_handle->func_type == 2){

        // timing
        double kernel_time = 0;

        gettimeofday(&start_time, NULL);

        if(!contains_table(src1_name, table_management)){
            printf("source table ");
            printf(src1_name);
            printf(" is not contains in current management unit\n");
            return;
        }

        if(!contains_table(src2_name, table_management)){
            printf("source table ");
            printf(src2_name);
            printf(" is not contains in current management unit\n");
            return;
        }

        struct dpu_set_t set = table_management->set;
        uint32_t num_dpus = table_management->num_dpus;
        table_host_t* src1_table = lookup_table(src1_name, table_management);
        table_host_t* src2_table = lookup_table(src2_name, table_management);

        if(src1_table->len != src2_table->len){
            printf("zip length does not match !!!");
            return;
        }

        zip_arguments_t* input_args = table_management->zip_args;

        if(src1_table->is_virtual_zipped == 1){
            src1_table->is_virtual_zipped = 0;
            const char* binary = binary_handle->bin_location;
            DPU_ASSERT(dpu_load(set, binary, NULL));

            for(uint32_t i=0; i<num_dpus; i++){
                input_args[i].input_start_offset1 = src1_table->start1;
                input_args[i].input_start_offset2 = src1_table->start2;
                input_args[i].input_type_size1 = src1_table->type1;
                input_args[i].input_type_size2 = src1_table->type2;
                input_args[i].outputs = src1_table->start;
            
            }

            DPU_FOREACH(set, dpu, i) {
		        DPU_ASSERT(dpu_prepare_xfer(dpu, input_args + i));
	        }

            DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "ZIP_INPUT_ARGUMENTS", 0, sizeof(zip_arguments_t), DPU_XFER_DEFAULT));
            gettimeofday(&start_time, NULL);
            DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
            gettimeofday(&end_time, NULL);

            kernel_time += (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);
        }

        if(src2_table->is_virtual_zipped == 1){
            src2_table->is_virtual_zipped = 0;
            const char* binary = binary_handle->bin_location;
            DPU_ASSERT(dpu_load(set, binary, NULL));

            for(uint32_t i=0; i<num_dpus; i++){
                input_args[i].input_start_offset1 = src2_table->start1;
                input_args[i].input_start_offset2 = src2_table->start2;
                input_args[i].input_type_size1 = src2_table->type1;
                input_args[i].input_type_size2 = src2_table->type2;
                input_args[i].outputs = src2_table->start;
            
            }

            DPU_FOREACH(set, dpu, i) {
		        DPU_ASSERT(dpu_prepare_xfer(dpu, input_args + i));
	        }

            DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "ZIP_INPUT_ARGUMENTS", 0, sizeof(zip_arguments_t), DPU_XFER_DEFAULT));
            gettimeofday(&start_time, NULL);
            DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
            gettimeofday(&end_time, NULL);

            kernel_time += (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);
        }

        // virtually zip two tables
        uint32_t* lens = src2_table->lens_each_dpu;
        uint32_t* lens_ = src2_table->lens_each_dpu;

        uint32_t input_type1 = src1_table->table_type_size;
        uint32_t start1 = src1_table->start;
        uint32_t end1 = src1_table->end;
        uint32_t input_type2 = src2_table->table_type_size;
        uint32_t start2 = src2_table->start;
        uint32_t end2 = src2_table->end;

        for(uint32_t i=0; i<num_dpus; i++){
            if(lens[i]!=lens_[i]){
                printf("zip length does not match on dpu %d!!!", i);
                return;
            }
        }


        // table information to management unit
        // timing
        gettimeofday(&start_time, NULL);

        
        table_host_t* t = malloc(sizeof(table_host_t));
        t->name = malloc(strlen(dest_name)+1);
        memcpy(t->name, dest_name, strlen(dest_name)+1);
        uint32_t output_type = input_type1+input_type2;
        t->start = outputs;
        uint32_t max_end_dpu = max_len_dpu(num_dpus, src1_table)*output_type+outputs;
        t->end = max_end_dpu+(8-max_end_dpu%8);
        t->len = src1_table->len;
        t->table_type_size = output_type;
        t->lens_each_dpu = malloc(num_dpus*sizeof(int32_t));
        memcpy(t->lens_each_dpu, lens, num_dpus*sizeof(int32_t));

        t->is_virtual_zipped = 1;
        t->start1 = start1;
        t->end1 = end1;
        t->start2 = start2;
        t->end2 = end2;
        t->type1 = input_type1;
        t->type2 = input_type2;

        add_table(t, table_management);
        
    	table_management->free_space_start_pos = table_management->free_space_start_pos > t->end ? table_management->free_space_start_pos : t->end;

        gettimeofday(&end_time, NULL);
        printf("\nzip function kernel execution time : %f\n", kernel_time/1000);
        double register_table_time = (end_time.tv_sec - start_time.tv_sec) * 1000000.0 +
                      (end_time.tv_usec - start_time.tv_usec);
        printf("--------------\n");
        printf("zip function call and table management time : %f\n", (register_table_time)/1000);
        printf("--------------\n");
    }
    else{
        printf("ERROR: compiled binary ");
        printf(binary_handle->bin_location);
        printf(" is not a zip function\n");
    }
}
