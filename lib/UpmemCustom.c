#include "UpmemCustom.h"
// allocate aligned arr and zero out rest
uint32_t calculate_pad_len(uint64_t len, uint32_t type_size, uint32_t num_dpus){
    uint64_t len_in_byte = (uint64_t)len*type_size;

    // calculate lcm of typesize and 8, each dpu gets %8
    uint32_t lcm = (type_size > 8) ? type_size : 8;

    while (1) {
        if (lcm % type_size == 0 && lcm % 8 == 0) {
            break;
        }
        ++lcm;
    }

    // divisible by typesize
    uint64_t padded_len = len_in_byte;

    while (1) {
        if (padded_len % num_dpus == 0 && (padded_len/num_dpus) % lcm == 0) {
            break;
        }
        ++padded_len;
    }

    uint64_t pad_len = padded_len - len_in_byte;
    return (uint32_t)pad_len;
}

void* malloc_split_aligned(uint32_t len, uint32_t type_size, uint32_t num_dpus){
    uint32_t pad_len = calculate_pad_len(len , type_size, num_dpus);
    uint64_t len_in_byte = (uint64_t)len * type_size;

    void* ptr = malloc(len_in_byte+pad_len);

    for(int i=0; i<len_in_byte+pad_len; i++){
        ((char*)ptr)[i] = 0;
    }

    return ptr;
}

void* malloc_broadcast_aligned(uint32_t len, uint32_t type_size){
    uint64_t len_in_byte = (uint64_t)len * type_size;
    uint32_t pad_len = len_in_byte%8;

    void* ptr = malloc(len_in_byte+pad_len);

    for(int i=0; i<pad_len+len_in_byte; i++){
        ((char*)ptr)[i] = 0;
    }

    return ptr;
}

void prepare_input_len_and_parse_args(struct dpu_set_t set, dpu_arguments_t* input_args, uint32_t input_len, uint32_t input_type_size, uint32_t num_dpus){
    uint32_t pad_len_in_elem = calculate_pad_len(input_len, input_type_size, num_dpus)/input_type_size;
    uint32_t num_transfered_elem_per_dpu = (input_len + pad_len_in_elem)/num_dpus;

    struct dpu_set_t dpu;

    int i;
	DPU_FOREACH(set, dpu, i) {
	    // prepare and copy input arguments to DPU
        if((i+1)*num_transfered_elem_per_dpu<input_len){
            input_args[i].input_len = num_transfered_elem_per_dpu;
        }
        else if(i*num_transfered_elem_per_dpu<input_len){
            input_args[i].input_len = input_len - i*num_transfered_elem_per_dpu;
        }
        else{
            input_args[i].input_len = 0;
        }
		DPU_ASSERT(dpu_prepare_xfer(dpu, input_args + i));
	}
    //DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, "DPU_INPUT_ARGUMENTS", 0, sizeof(dpu_arguments_t), DPU_XFER_DEFAULT));

    printf("%d DPUs and %u elements\n", num_dpus, input_len);
    uint32_t max = 0;
    for(int i=0; i<num_dpus; i++){
        max = max>input_args[i].input_len?max:input_args[i].input_len;
    }
    printf("assigning %d elements at max for each dpu\n", max);
}




uint32_t host_split_to_dpu(struct dpu_set_t set, void* elements, uint32_t len, uint32_t type_size, uint32_t num_dpus, uint32_t curr_offset){
    //elements must be 8 bytes aligned and divisible by num_dpus， pad not aglined, use malloc_aligned
    uint32_t pad_len = calculate_pad_len(len , type_size, num_dpus);

    //split elements to dpu and rest remains on host
    uint32_t len_per_dpu_in_byte = ((uint64_t)len*type_size+pad_len)/num_dpus;

    //transfer to dpu
    int i;
    struct dpu_set_t dpu;


    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &((char*)elements)[i * len_per_dpu_in_byte]));
    }

    // offset
    
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, curr_offset, len_per_dpu_in_byte, DPU_XFER_DEFAULT));
    
    return curr_offset+len_per_dpu_in_byte;

}

uint32_t host_broadcast_to_dpu(struct dpu_set_t set, void* elements, uint32_t len, uint32_t type_size, uint32_t curr_offset){
    uint32_t broadcast_size = (len*type_size)+(len*type_size)%8;
    DPU_ASSERT(dpu_broadcast_to(set, DPU_MRAM_HEAP_POINTER_NAME, curr_offset, elements, broadcast_size, DPU_XFER_DEFAULT));
    return curr_offset+broadcast_size;
}

void* malloc_gather_aligned(uint32_t len, uint32_t type_size, uint32_t num_dpus){
    uint32_t len_in_byte = len * type_size;
    uint32_t pad_len = len_in_byte%8;

    void* ptr = calloc((len_in_byte+pad_len)*num_dpus);

    return ptr;
}

void gather_tables_to_host(struct dpu_set_t set, void* my_table, uint32_t len, uint32_t type_size, uint32_t curr_offset_on_mram, uint32_t num_dpus, void (*init_func)(uint32_t, void*) ,void (*combineFunc)(void*, void*)){
    void* tables = malloc_gather_aligned(len, type_size, num_dpus);
    uint32_t aligned_table_size = (len*type_size)+(len*type_size)%8;

    for(int i=0; i<len; i++){
        (*init_func)(type_size, my_table+i*type_size);
    }


    int i;
    struct dpu_set_t dpu;

	DPU_FOREACH(set, dpu, i) {
	    
		DPU_ASSERT(dpu_prepare_xfer(dpu, tables+i*aligned_table_size));
	}
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, curr_offset_on_mram, aligned_table_size, DPU_XFER_DEFAULT));
    
    uint32_t omp_threads = 8;
    uint32_t thread_id;
    uint32_t table_size = len*type_size;

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

void* gather_to_host(struct dpu_set_t set, uint32_t* lens, uint32_t type_size, uint32_t curr_offset_on_mram, uint32_t num_dpus){
    uint64_t total_size=0;
    uint32_t max_len=0;
    uint32_t aligned_max_len;
    uint32_t len;
    for(int i=0; i<num_dpus; i++){
        len = lens[i];
        max_len = max_len>len?max_len:len;
        total_size += (uint64_t)len*type_size;
    }
    aligned_max_len = (max_len*type_size)+(max_len*type_size)%8;
    

    void* tmp_buffer = malloc((uint64_t)num_dpus*aligned_max_len);
    void* res = malloc(total_size);
    printf("max len per dpu %u\n", aligned_max_len);
    printf("transfer buffer size %u\n", num_dpus*aligned_max_len);
    int i;
    struct dpu_set_t dpu;
	DPU_FOREACH(set, dpu, i) {  
		DPU_ASSERT(dpu_prepare_xfer(dpu, tmp_buffer+i*aligned_max_len));
	}
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, curr_offset_on_mram, aligned_max_len, DPU_XFER_DEFAULT));


    void* buff_ptr = (void*)tmp_buffer;
    void* ptr_in_res = (void*)res;
    uint32_t curr_size;

    //printf("\n-----\n%d\n-----\n", *(int*)buff_ptr);
    for(int i=0; i<num_dpus; i++){
        curr_size = type_size*lens[i];
        memcpy(ptr_in_res, buff_ptr, curr_size);
        buff_ptr += aligned_max_len;
        ptr_in_res += curr_size;
    }
    

    free(tmp_buffer);
    return res;

}

