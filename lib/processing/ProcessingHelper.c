#include "ProcessingHelper.h"
uint64_t copy_block_size_fun(uint32_t type_size1, uint32_t type_size2,  uint32_t num_elem){
    //1024
    uint64_t res=0;
    uint32_t res_arr[2];
    uint32_t max_type_size = type_size1>type_size2?type_size1:type_size2;
    if(type_size1%8 == 0 && type_size2%8 == 0 && (num_elem <= NR_TASKLETS || max_type_size > 512)){
        res_arr[0] = 1;
        res_arr[1] = 0;
    }
    else if(type_size1%4 == 0 && type_size2%4 == 0 &&(num_elem <= 2*NR_TASKLETS || max_type_size > 256)){
        res_arr[0] = 2;
        res_arr[1] = 1;
    }
    else if(type_size1%2 == 0 && type_size2%2 == 0 && max_type_size > 128){
        res_arr[0] = 4;
        res_arr[1] = 2;
    }
    else if(max_type_size < 16){
        res_arr[0] = 256;
        res_arr[1] = 8;
    }
    else if(max_type_size < 32){
        res_arr[0] = 128;
        res_arr[1] = 7;
    }
    else{
        res_arr[0] = 16;
        res_arr[1] = 4;
    }

    res = *(uint64_t*)res_arr;
    return res;

}

