#ifndef MAPPROCESSING_H
#define MAPPROCESSING_H
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <defs.h>
#include <mram.h>
#include <barrier.h>
#include "../ProcessingHelper.h"
#include __mapfunc_filename__

BARRIER_INIT(barrier_p, NR_TASKLETS);
void map_dpu(__mram_ptr void* inputs, __mram_ptr void* outputs, uint32_t input_type, uint32_t output_type, uint32_t len){
    uint32_t elem_type_size = input_type;
    uint32_t inter_type_size = output_type;
    uint32_t num_tasklets = NR_TASKLETS; 
    uint32_t pid = me();
    uint64_t tuple = copy_block_size_fun(elem_type_size, inter_type_size, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];
    // try malloc/free for performance
    fsb_allocator_t elems_block_allocator = fsb_alloc(elem_type_size<<copy_block_size_shiftbits, 1);
    __dma_aligned void* elems_block = fsb_get(elems_block_allocator);

    fsb_allocator_t inter_block_allocator = fsb_alloc(inter_type_size<<copy_block_size_shiftbits, 1);
    __dma_aligned void* inter_block = fsb_get(inter_block_allocator);

    uint32_t block_size_inputs = elem_type_size<<copy_block_size_shiftbits;
    uint32_t block_size_outputs = inter_type_size<<copy_block_size_shiftbits;
    uint32_t block_times_tasklets = num_tasklets<<copy_block_size_shiftbits;

    uint32_t divisible_len = (len>>copy_block_size_shiftbits)<<copy_block_size_shiftbits;
    uint32_t rest_len = len - divisible_len;

    void* inter, *elem;


    uint32_t unroll_block_size = (copy_block_size>>2)<<2;
    uint32_t unroll_block_rest = copy_block_size-unroll_block_rest;

    uint32_t i_init = pid<<copy_block_size_shiftbits;
    uint32_t i_elem = (uint32_t)inputs+i_init*elem_type_size;
    uint32_t i_inter = (uint32_t)outputs+i_init*inter_type_size;

    uint32_t i_elem_stride = block_times_tasklets * elem_type_size;
    uint32_t i_inter_stride = block_times_tasklets * inter_type_size;

    for(int i=pid*copy_block_size; i<divisible_len; i+=block_times_tasklets){
        
        mram_read((__mram_ptr void*)(i_elem), elems_block, block_size_inputs);
        inter = inter_block;
        elem = elems_block;

        for(uint32_t j=0; j<unroll_block_size; j+=4){

            map_func(elem, inter);

            elem += elem_type_size;
            inter += inter_type_size;
            map_func(elem, inter);

            elem += elem_type_size;
            inter += inter_type_size;
            map_func(elem, inter);

            elem += elem_type_size;
            inter += inter_type_size;
            map_func(elem, inter);

            elem += elem_type_size;
            inter += inter_type_size;
        }
        

        for(uint32_t j=0; j<unroll_block_rest; j++){
            map_func(elem, inter);
            inter += inter_type_size;
            elem += elem_type_size;
        }

        mram_write(inter_block, (__mram_ptr void*)(i_inter),  block_size_outputs);
        i_inter += i_inter_stride;
        i_elem += i_elem_stride;
        
    }
    

    
    // handle last block 
    if(pid==NR_TASKLETS-1 && rest_len != 0){
        uint32_t last_block_elem_addr = divisible_len * elem_type_size;
        uint32_t last_block_inter_addr = divisible_len * inter_type_size;

        mram_read((__mram_ptr void*)(outputs+last_block_inter_addr), inter_block, block_size_outputs);
        mram_read((__mram_ptr void*)(inputs+last_block_elem_addr), elems_block, block_size_inputs);


        elem = elems_block;
        inter = inter_block;
        for(uint32_t j=0; j<rest_len; j++){
            map_func(elem, inter);
            elem += elem_type_size;
            inter += inter_type_size;
        }


        mram_write(inter_block, (__mram_ptr void*)(outputs+last_block_inter_addr),  block_size_outputs);
    }
    
    

    fsb_free(elems_block_allocator, elems_block);
    fsb_free(inter_block_allocator, inter_block);

    barrier_wait(&barrier_p);
}

void zip_map_dpu(__mram_ptr void* inputs1, __mram_ptr void* inputs2, __mram_ptr void* outputs, uint32_t input_type1, uint32_t input_type2, uint32_t output_type, uint32_t len){
    uint32_t elem_type_size = input_type1+input_type2;
    uint32_t inter_type_size = output_type;
    uint32_t num_tasklets = NR_TASKLETS; 
    uint32_t pid = me();
    uint64_t tuple = copy_block_size_fun(elem_type_size, inter_type_size, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];
    // try malloc/free for performance
    fsb_allocator_t elem_allocator = fsb_alloc(elem_type_size, 1);
    __dma_aligned void* elem = fsb_get(elem_allocator);

    fsb_allocator_t elems_block_allocator_tmp = fsb_alloc(elem_type_size<<copy_block_size_shiftbits, 1);
    __dma_aligned void* elems_block_tmp = fsb_get(elems_block_allocator_tmp);

    uint32_t input1_block_size = input_type1<<copy_block_size_shiftbits;
    uint32_t input2_block_size = input_type2<<copy_block_size_shiftbits;

    void* inputs1_block = elems_block_tmp;
    void* inputs2_block = elems_block_tmp + input1_block_size;

    fsb_allocator_t inter_block_allocator = fsb_alloc(inter_type_size<<copy_block_size_shiftbits, 1);
    __dma_aligned void* inter_block = fsb_get(inter_block_allocator);

    uint32_t block_size_inputs = elem_type_size<<copy_block_size_shiftbits;
    uint32_t block_size_outputs = inter_type_size<<copy_block_size_shiftbits;
    uint32_t block_times_tasklets = num_tasklets<<copy_block_size_shiftbits;

    uint32_t divisible_len = (len>>copy_block_size_shiftbits)<<copy_block_size_shiftbits;
    uint32_t rest_len = len - divisible_len;

    void* inter, *input_elem1, *input_elem2;


    uint32_t i_init = pid<<copy_block_size_shiftbits;
    uint32_t i_input1 = (uint32_t)inputs1+i_init*input_type1;
    uint32_t i_input2 = (uint32_t)inputs2+i_init*input_type2;
    uint32_t i_inter = (uint32_t)outputs+i_init*inter_type_size;

    uint32_t i_input1_stride = block_times_tasklets * input_type1;
    uint32_t i_input2_stride = block_times_tasklets * input_type2;
    uint32_t i_inter_stride = block_times_tasklets * inter_type_size;

    uint32_t input_type_1_div_4 = input_type1 >> 2;
    uint32_t input_type_2_div_4 = input_type2 >> 2;
    uint32_t input_type_1_rest_4 = input_type1 - (input_type_1_div_4<<2);
    uint32_t input_type_2_rest_4 = input_type2 - (input_type_2_div_4<<2);


    void* elem_plus_input1 = elem + input_type1;

    uint32_t unroll_block_size = (copy_block_size>>2)<<2;
    uint32_t unroll_block_rest = copy_block_size-unroll_block_rest;

    if(input_type1 ==4 && input_type2 == 4){
        for(int i=pid*copy_block_size; i<divisible_len; i+=block_times_tasklets){
        
            mram_read((__mram_ptr void*)(i_input1), inputs1_block, input1_block_size);
            mram_read((__mram_ptr void*)(i_input2), inputs2_block, input2_block_size);
            inter = inter_block;
            input_elem1 = inputs1_block;
            input_elem2 = inputs2_block;
            for(uint32_t j=0; j<unroll_block_size; j+=4){

                ((int32_t*)elem)[0] = *(int32_t*)input_elem1;
                ((int32_t*)elem)[1] = *(int32_t*)input_elem2;
                map_func(elem, inter);
                inter += inter_type_size;
                input_elem1 += input_type1;
                input_elem2 += input_type2; 

                ((int32_t*)elem)[0] = *(int32_t*)input_elem1;
                ((int32_t*)elem)[1] = *(int32_t*)input_elem2;
                map_func(elem, inter);
                inter += inter_type_size;
                input_elem1 += input_type1;
                input_elem2 += input_type2;

                ((int32_t*)elem)[0] = *(int32_t*)input_elem1;
                ((int32_t*)elem)[1] = *(int32_t*)input_elem2;
                map_func(elem, inter);
                inter += inter_type_size;
                input_elem1 += input_type1;
                input_elem2 += input_type2;

                ((int32_t*)elem)[0] = *(int32_t*)input_elem1;
                ((int32_t*)elem)[1] = *(int32_t*)input_elem2;
                map_func(elem, inter);
                inter += inter_type_size;
                input_elem1 += input_type1;
                input_elem2 += input_type2;    
            }

            for(uint32_t j=0; j<unroll_block_rest; j++){
                ((int32_t*)elem)[0] = *(int32_t*)input_elem1;
                ((int32_t*)elem)[1] = *(int32_t*)input_elem2;
                map_func(elem, inter);
                inter += inter_type_size;
                input_elem1 += input_type1;
                input_elem2 += input_type2; 
            }

            mram_write(inter_block, (__mram_ptr void*)(i_inter),  block_size_outputs);
            i_inter += i_inter_stride;
            i_input1 += i_input1_stride;
            i_input2 += i_input2_stride;
        }
    }
    else{
        for(int i=pid*copy_block_size; i<divisible_len; i+=block_times_tasklets){
        
            mram_read((__mram_ptr void*)(i_input1), inputs1_block, input1_block_size);
            mram_read((__mram_ptr void*)(i_input2), inputs2_block, input2_block_size);
            inter = inter_block;
            input_elem1 = inputs1_block;
            input_elem2 = inputs2_block;
            for(uint32_t j=0; j<copy_block_size; j++){

            
                for(int k=0; k<input_type_1_div_4; k++){
                    ((int32_t*)elem)[k] = ((int32_t*)input_elem1)[k];
                }

                for(int k=input_type_1_rest_4; k<input_type1; k++){
                    ((char*)elem)[k] = ((char*)input_elem1)[k];
                }


                for(int k=0; k<input_type_2_div_4; k++){
                    ((int32_t*)elem_plus_input1)[k] = ((int32_t*)input_elem2)[k];
                }

                for(int k=input_type_2_rest_4; k<input_type2; k++){
                    ((char*)elem_plus_input1)[k] = ((char*)input_elem2)[k];
                }
            

                map_func(elem, inter);
                inter += inter_type_size;
                input_elem1 += input_type1;
                input_elem2 += input_type2;     
            }

            mram_write(inter_block, (__mram_ptr void*)(i_inter),  block_size_outputs);
            i_inter += i_inter_stride;
            i_input1 += i_input1_stride;
            i_input2 += i_input2_stride;
        }
    }
    

    
    // handle last block 
    
    if(pid==NR_TASKLETS-1 && rest_len != 0){
        uint32_t last_block_elem1_addr = divisible_len * input_type1;
        uint32_t last_block_elem2_addr = divisible_len * input_type2;
        uint32_t last_block_inter_addr = divisible_len * inter_type_size;

        mram_read((__mram_ptr void*)(outputs+last_block_inter_addr), inter_block, block_size_outputs);
        mram_read((__mram_ptr void*)(inputs1+last_block_elem1_addr), inputs1_block, input1_block_size);
        mram_read((__mram_ptr void*)(inputs2+last_block_elem2_addr), inputs2_block, input2_block_size);

        inter = inter_block;
        input_elem1 = inputs1_block;
        input_elem2 = inputs2_block;
        
        for(uint32_t j=0; j<rest_len; j++){

            for(int k=0; k<input_type_1_div_4; k++){
                ((int32_t*)elem)[k] = ((int32_t*)input_elem1)[k];
            }

            for(int k=input_type_1_rest_4; k<input_type1; k++){
                ((char*)elem)[k] = ((char*)input_elem1)[k];
            }


            for(int k=0; k<input_type_2_div_4; k++){
                ((int32_t*)elem_plus_input1)[k] = ((int32_t*)input_elem2)[k];
            }

            for(int k=input_type_2_rest_4; k<input_type2; k++){
                ((char*)elem_plus_input1)[k] = ((char*)input_elem2)[k];
            }

            map_func(elem, inter);
            inter += inter_type_size;
            input_elem1 += input_type1;
            input_elem2 += input_type2;
        }
       

        mram_write(inter_block, (__mram_ptr void*)(outputs+last_block_inter_addr),  block_size_outputs);

    }

    
    

    fsb_free(elem_allocator, elem);
    fsb_free(elems_block_allocator_tmp, elems_block_tmp);
    fsb_free(inter_block_allocator, inter_block);

    barrier_wait(&barrier_p);
}
#endif 