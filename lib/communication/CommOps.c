#include "CommOps.h"

/*
    see description of the functions in CommOps.h
*/
void* malloc_scatter_aligned(uint64_t len, uint32_t type_size, simplepim_management_t* table_management){
    uint32_t num_dpus = table_management->num_dpus;
    uint32_t pad_len = calculate_pad_len(len , type_size, num_dpus);
    uint64_t len_in_byte = len * (uint64_t)type_size;

    void* ptr = calloc(1, len_in_byte+pad_len);

    return ptr;
}

void* malloc_reduce_aligned(uint32_t len, uint32_t type_size, simplepim_management_t* table_management){
    uint32_t num_dpus = table_management->num_dpus;
    uint64_t len_in_byte = len * type_size;
    uint64_t pad_len = 8-len_in_byte%8;

    void* ptr = calloc(num_dpus, (len_in_byte+pad_len));

    return ptr;
}

void* malloc_broadcast_aligned(uint32_t len, uint32_t type_size, simplepim_management_t* table_management){
    uint64_t len_in_byte = (uint64_t)len * type_size;
    uint64_t pad_len = 8-len_in_byte%8;

    void* ptr = calloc(1, len_in_byte+pad_len);

    return ptr;
}

void simplepim_scatter(char* const table_id, void* elements, uint64_t len, uint32_t type_size, simplepim_management_t* table_management){
    uint32_t curr_offset = table_management->free_space_start_pos;
    if(contains_table(table_id, table_management)){
        curr_offset = lookup_table(table_id, table_management) -> start;
    }
    // elements must be 8 bytes aligned and divisible by num_dpus， pad not aglined, use malloc_aligned
    if(contains_table(table_id, table_management)){
        printf(table_id);
        printf(" is contained in table management unit, invalid scatter\n");
        return;
    }
    uint32_t num_dpus = table_management->num_dpus;
    uint32_t pad_len = calculate_pad_len(len , type_size, num_dpus);

    // split elements to dpu and rest remains on host
    uint32_t len_per_dpu_in_byte = (len*type_size+pad_len)/num_dpus;

    // transfer to dpu
    int i;
    struct dpu_set_t dpu;
    struct dpu_set_t set = table_management->set;


    DPU_FOREACH(set, dpu, i) {
        DPU_ASSERT(dpu_prepare_xfer(dpu, &((char*)elements)[i * len_per_dpu_in_byte]));
    }

    // offset
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_TO_DPU, DPU_MRAM_HEAP_POINTER_NAME, curr_offset, len_per_dpu_in_byte, DPU_XFER_DEFAULT));
    
    // calculate lens per dpu
    uint32_t pad_len_in_elem = pad_len/type_size;
    uint32_t num_transfered_elem_per_dpu = (len + pad_len_in_elem)/num_dpus;
    uint32_t* lens = malloc(sizeof(int32_t)*num_dpus);
    for(int i=0; i<num_dpus; i++){
        if((i+1)*num_transfered_elem_per_dpu<len){
            lens[i] = num_transfered_elem_per_dpu;
        }
        else if(i*num_transfered_elem_per_dpu<len){
            lens[i] = len - i*num_transfered_elem_per_dpu;
        }
        else{
            lens[i] = 0;
        }
    }


    // table information to management unit
    table_host_t* t = malloc(sizeof(table_host_t));
    t->name = malloc(strlen(table_id)+1);
    memcpy(t->name, table_id, strlen(table_id)+1);
    t->start = curr_offset;
    t->end = curr_offset+len_per_dpu_in_byte;
    t->len = len;
    t->table_type_size = type_size;
    t->lens_each_dpu = lens;
    t->is_virtual_zipped = 0;

    add_table(t, table_management);
    table_management->free_space_start_pos = table_management->free_space_start_pos > t->end ? table_management->free_space_start_pos : t->end;
}

void* simplepim_gather(char* const table_id, simplepim_management_t* table_management){
    if(!contains_table(table_id, table_management)){
        printf(table_id);
        printf(" is not contained in table management unit, invalid scatter\n");
        return NULL;
    }

    uint32_t num_dpus = table_management->num_dpus; 
    table_host_t* t = lookup_table(table_id, table_management);
    uint32_t* lens = t->lens_each_dpu;
    uint32_t type_size = t->table_type_size;
    uint32_t start_addr = t->start;
    uint32_t max_len = 0;
    for(int i=0; i<num_dpus; i++){
        max_len = max_len>lens[i]?max_len:lens[i];
    }

    uint64_t aligned_max_len = (max_len*type_size)+(8-(max_len*type_size)%8);
    uint64_t buff_size = aligned_max_len*num_dpus;
    uint64_t total_size = t->len*t->table_type_size;
    struct dpu_set_t set = table_management->set;
    void* tmp_buffer = malloc((uint64_t)num_dpus*aligned_max_len+2048);
    void* res = malloc(total_size + 2048);

    int i;
    struct dpu_set_t dpu;
	DPU_FOREACH(set, dpu, i) {  
		DPU_ASSERT(dpu_prepare_xfer(dpu, tmp_buffer+i*aligned_max_len));
	}
    DPU_ASSERT(dpu_push_xfer(set, DPU_XFER_FROM_DPU, DPU_MRAM_HEAP_POINTER_NAME, start_addr, aligned_max_len, DPU_XFER_DEFAULT));

    void* buff_ptr = (void*)tmp_buffer;
    void* ptr_in_res = (void*)res;
    uint32_t curr_size;

    
    for(int j=0; j<num_dpus; j++){
        curr_size = type_size*lens[j];
        memcpy(ptr_in_res, buff_ptr, curr_size);
        buff_ptr += aligned_max_len;
        ptr_in_res += curr_size;
    }
    

    free(tmp_buffer);
    return res;

}

void simplepim_broadcast(char* const table_id, void* elements, uint64_t len, uint32_t type_size, simplepim_management_t* table_management){
    uint32_t curr_offset = table_management->free_space_start_pos;
    if(contains_table(table_id, table_management)){
    	curr_offset = lookup_table(table_id, table_management) -> start;
    }
   
    uint64_t broadcast_size = (len*type_size)+8-(len*type_size)%8;
    uint32_t num_dpus = table_management->num_dpus; 
    struct dpu_set_t set = table_management->set;
    DPU_ASSERT(dpu_broadcast_to(set, DPU_MRAM_HEAP_POINTER_NAME, curr_offset, elements, broadcast_size, DPU_XFER_DEFAULT));
    // table information to management unit
    table_host_t* t = malloc(sizeof(table_host_t));
    t->name = malloc(strlen(table_id)+1);
    memcpy(t->name, table_id, strlen(table_id)+1);
    t->start = curr_offset;
    t->end = curr_offset+broadcast_size;
    t->len = len;
    t->table_type_size = type_size;

    uint32_t* lens = malloc(sizeof(int32_t)*num_dpus);
    for(int i=0; i<num_dpus; i++){
        lens[i] = len;
    }

    t->lens_each_dpu = lens;
    t->is_virtual_zipped = 0;
    add_table(t, table_management);	
    table_management->free_space_start_pos = t->end > table_management->free_space_start_pos ? t->end : table_management->free_space_start_pos;
}

void simplepim_allgather(char* const table_id, char* const new_table_id, simplepim_management_t* table_management){
    if(!contains_table(table_id, table_management)){
        printf(table_id);
        printf(" is not contained in table management unit, invalid allgather\n");
        return;
    }

    uint32_t num_dpus = table_management->num_dpus; 
    table_host_t* t = lookup_table(table_id, table_management);
    uint32_t* lens = t->lens_each_dpu;
    uint32_t type_size = t->table_type_size;

    uint32_t total_len = 0;

    for(int i=0; i<num_dpus; i++){
        total_len += lens[i];
    }

    void* bc_buffer = malloc_broadcast_aligned(total_len, type_size, table_management);
    void* res = simplepim_gather(table_id, table_management);
    memcpy(bc_buffer, res, total_len*type_size);
    simplepim_broadcast(new_table_id, bc_buffer, total_len, type_size, table_management);


    free(bc_buffer);

}


void simplepim_allreduce(char* const table_id, handle_t* binary_handle, simplepim_management_t* table_management){
    if(binary_handle->func_type == 1){
        if(!contains_table(table_id, table_management)){
            printf("source table ");
            printf(table_id);
            printf(" is not contains in current management unit\n");
            return;
        }

        uint32_t num_dpus = table_management->num_dpus; 
        table_host_t* t = lookup_table(table_id, table_management);
        uint32_t* lens = t->lens_each_dpu;
        uint32_t type_size = t->table_type_size;
        uint32_t len = lens[0];

        for(int i=0; i<num_dpus; i++){
            if(lens[i] != len){
                printf("can not call allreduce on inequal length array\n");
                return;
            }
        }

        void* lib=dlopen(binary_handle->so_bin_location, RTLD_NOW);
        void (*combine_func)(void*, void*) = dlsym(lib, "combine_func");

        void* res = simplepim_gather(table_id, table_management);
        for(int i=1; i<num_dpus; i++){
            int offset = i*len*type_size;
            for(int j=0; j<len; j++){
                combine_func(res+j*type_size, res+offset+j*type_size);
            }
        }

        void* bc_buffer = malloc_broadcast_aligned(len, type_size, table_management);

        memcpy(bc_buffer, res, len*type_size);

        uint64_t outputs_pos = lookup_table(table_id, table_management) -> start;
        uint64_t broadcast_size = (len*type_size)+8-(len*type_size)%8;
        struct dpu_set_t set = table_management->set;
        DPU_ASSERT(dpu_broadcast_to(set, DPU_MRAM_HEAP_POINTER_NAME, outputs_pos, bc_buffer, broadcast_size, DPU_XFER_DEFAULT));

        free(bc_buffer);
    
    }
    else{
        printf("ERROR: compiled binary ");
        printf(binary_handle->bin_location);
        printf(" does not contain general reduction functions\n");
    }

}

