#include "ZipProcessing.h"
BARRIER_INIT(barrier_p, NR_TASKLETS);

void zip_dpu(__mram_ptr void* table_entries_1, __mram_ptr void* table_entries_2, __mram_ptr void* table_entries_res, uint32_t input_type_1, uint32_t input_type_2, uint32_t len){
    uint32_t num_tasklets = NR_TASKLETS; 
    uint32_t pid = me();
    uint64_t tuple = copy_block_size_fun(input_type_1, input_type_2, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];

    uint32_t input_block_size_1 = input_type_1<<copy_block_size_shiftbits;
    uint32_t input_block_size_2 = input_type_2<<copy_block_size_shiftbits;
    uint32_t output_type = input_type_1+input_type_2;
    uint32_t output_block_size = input_block_size_1+input_block_size_2;

    fsb_allocator_t input1_block_allocator = fsb_alloc(input_block_size_1, 1);
    __dma_aligned void* input1_block = fsb_get(input1_block_allocator);

    fsb_allocator_t input2_block_allocator = fsb_alloc(input_block_size_2, 1);
    __dma_aligned void* input2_block = fsb_get(input2_block_allocator);

    fsb_allocator_t output_block_allocator = fsb_alloc(output_block_size, 1);
    __dma_aligned void* output_block = fsb_get(output_block_allocator);

    uint32_t divisible_len = (len>>copy_block_size_shiftbits)<<copy_block_size_shiftbits;
    uint32_t rest_len = len - divisible_len;

    uint32_t block_times_tasklets = copy_block_size*num_tasklets;

    uint32_t i_input1;
    uint32_t i_input2;
    uint32_t i_output;
    
    void* j_output;
    void* input1;
    void* input2;
    void* output;

    uint32_t pid_times_block_size = pid*copy_block_size;

    i_input1 = pid_times_block_size*input_type_1;
    i_input2 = pid_times_block_size*input_type_2;
    i_output = pid_times_block_size*output_type;

    uint32_t input1_stride = block_times_tasklets*input_type_1;
    uint32_t input2_stride = block_times_tasklets*input_type_2;
    uint32_t output_stride = block_times_tasklets*output_type;

    uint32_t input_type_1_div_4 = input_type_1 >> 2;
    uint32_t input_type_2_div_4 = input_type_2 >> 2;
    uint32_t input_type_1_rest_4 = input_type_1 - (input_type_1_div_4<<2);
    uint32_t input_type_2_rest_4 = input_type_2 - (input_type_2_div_4<<2);

    uint32_t types_div_4 = (input_type_1_rest_4==0)&&(input_type_2_rest_4==0);
    uint32_t types_are_ints = (input_type_1_div_4==1)&&(input_type_2_div_4==1);

    
    if(types_div_4){
        if(types_are_ints){

            for(int i=pid_times_block_size; i<divisible_len; i+=block_times_tasklets){
        
                mram_read((__mram_ptr void*)table_entries_1+i_input1, input1_block, input_block_size_1);
                mram_read((__mram_ptr void*)table_entries_2+i_input2, input2_block, input_block_size_2);
                mram_read((__mram_ptr void*)table_entries_res+i_output, output_block, output_block_size);

                input1 = input1_block;
                input2 = input2_block;
                output = output_block;

                for(int j=0; j<copy_block_size; j+=2){
            

                    ((int32_t*)output)[0] = ((int32_t*)input1)[0];
                    ((int32_t*)output)[1] = ((int32_t*)input2)[0];

                    ((int32_t*)output)[2] = ((int32_t*)input1)[1];
                    ((int32_t*)output)[3] = ((int32_t*)input2)[1];


                    input1 += 8;
                    input2 += 8;
                    output += 16;
                }

                mram_write(output_block, table_entries_res+i_output, output_block_size);

                i_input1 += input1_stride;
                i_input2 += input2_stride;
                i_output += output_stride;
            }

        }
        else{

            for(int i=pid_times_block_size; i<divisible_len; i+=block_times_tasklets){
        
                mram_read((__mram_ptr void*)table_entries_1+i_input1, input1_block, input_block_size_1);
                mram_read((__mram_ptr void*)table_entries_2+i_input2, input2_block, input_block_size_2);
                mram_read((__mram_ptr void*)table_entries_res+i_output, output_block, output_block_size);

                input1 = input1_block;
                input2 = input2_block;
                output = output_block;

                for(int j=0; j<copy_block_size; j++){
            

                    for(int k=0; k<input_type_1_div_4; k++){
                        ((int32_t*)output)[k] = ((int32_t*)input1)[k];
                    }

                    output += input_type_1;

                    for(int k=0; k<input_type_2_div_4; k++){
                        ((int32_t*)output)[k] = ((int32_t*)input2)[k];
                    }

                    input1 += input_type_1;
                    input2 += input_type_2;
                    output += input_type_2;
                }

                mram_write(output_block, table_entries_res+i_output, output_block_size);

                i_input1 += input1_stride;
                i_input2 += input2_stride;
                i_output += output_stride;
            }

        }

    }
    else{
        for(int i=pid_times_block_size; i<divisible_len; i+=block_times_tasklets){
        
            mram_read((__mram_ptr void*)table_entries_1+i_input1, input1_block, input_block_size_1);
            mram_read((__mram_ptr void*)table_entries_2+i_input2, input2_block, input_block_size_2);
            mram_read((__mram_ptr void*)table_entries_res+i_output, output_block, output_block_size);

            input1 = input1_block;
            input2 = input2_block;
            output = output_block;

            for(int j=0; j<copy_block_size; j++){
            

                for(int k=0; k<input_type_1_div_4; k++){
                    ((int32_t*)output)[k] = ((int32_t*)input1)[k];
                }

                for(int k=input_type_1_rest_4; k<input_type_1; k++){
                    ((char*)output)[k] = ((char*)input1)[k];
                }

                output += input_type_1;

                for(int k=0; k<input_type_2_div_4; k++){
                    ((int32_t*)output)[k] = ((int32_t*)input2)[k];
                }

                for(int k=input_type_2_rest_4; k<input_type_2; k++){
                    ((char*)output)[k] = ((char*)input2)[k];
                }

                input1 += input_type_1;
                input2 += input_type_2;
                output += input_type_2;
            }

            mram_write(output_block, table_entries_res+i_output, output_block_size);

            i_input1 += input1_stride;
            i_input2 += input2_stride;
            i_output += output_stride;
        }

    }


    // handle last block 
    if(pid==NR_TASKLETS-1 && rest_len != 0){
        uint32_t last_block_input1_addr = divisible_len * input_type_1;
        uint32_t last_block_input2_addr = divisible_len * input_type_2;
        uint32_t last_block_output_addr = divisible_len * output_type;

        mram_read((__mram_ptr void*)table_entries_1+last_block_input1_addr, input1_block, input_block_size_1);
        mram_read((__mram_ptr void*)table_entries_2+last_block_input2_addr, input2_block, input_block_size_2);
        mram_read((__mram_ptr void*)table_entries_res+last_block_output_addr, output_block, output_block_size);

        input1 = input1_block;
        input2 = input2_block;
        output = output_block;

        for(int j=0; j<rest_len; j++){

            for(int k=0; k<input_type_1_div_4; k++){
                ((int32_t*)output)[k] = ((int32_t*)input1)[k];
            }

            
            for(int k=input_type_1_rest_4; k<input_type_1; k++){
                ((char*)output)[k] = ((char*)input1)[k];
            }
        

            output += input_type_1;

            for(int k=0; k<input_type_2_div_4; k++){
                ((int32_t*)output)[k] = ((int32_t*)input2)[k];
            }

            
            for(int k=input_type_2_rest_4; k<input_type_2; k++){
                ((char*)output)[k] = ((char*)input2)[k];
            }
            

            input1 += input_type_1;
            input2 += input_type_2;
            output += input_type_2;

        }


        mram_write(output_block, table_entries_res+last_block_output_addr, output_block_size);

    }

    fsb_free(output_block_allocator, output_block);
    fsb_free(input1_block_allocator, input1_block);
    fsb_free(input2_block_allocator, input2_block);
    barrier_wait(&barrier_p);
}