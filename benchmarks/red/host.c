#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <dpu.h>

//#include "../../lib/communication/CommOps.h"
//#include "../../lib/management/Management.h"
#include "../../lib/processing/gen_red/GenRed.h"
#include "../../lib/processing/ProcessingHelperHost.h"
#include "../../lib/timer.h"
#include "Param.h"



void init(T* A){
    for (uint64_t i = 0; i < nr_elements; i++) {
        A[i] = i%1000;
    }
}

void add(void* p1, void* p2){
    T* ptr1 = (T*)(p1);
    T* ptr2 = (T*)(p2);
    *ptr1 += *ptr2;  
}

static T reduction_host(T* A) {
    T count = 0;
    for (uint64_t i = 0; i < nr_elements; i++) {
        count += A[i];
    }
    return count;
}

void run(){
    smalltable_management_t* table_management = table_management_init(dpu_number);
    T* A = (T*)malloc_scatter_aligned(nr_elements, sizeof(T), table_management);
    init(A);
    T correct_res = reduction_host(A);

    small_table_scatter("t1", A, nr_elements, sizeof(T), table_management);
    printf("end of data transfer\n");

    handle_t* va_handle = create_handle("red_funcs", REDUCE);

    
    T* res = table_gen_red("t1", "t2", sizeof(T), 1, va_handle, table_management, 0);


    if(print_info){
      struct dpu_set_t set = table_management->set;
      struct dpu_set_t dpu;
      DPU_FOREACH(set, dpu) {
        DPU_ASSERT(dpu_log_read(dpu, stdout));
      }
    }


    // accumulating floats may have difference in precision
    if( ((float)(correct_res-*res))/correct_res < 0.01 && ((float)(correct_res-*res))/correct_res > -0.01){
        printf("the result is correct \n");
    }
    else{
        printf("correct res : %f, got res : %f \n", correct_res, *res);
        printf("cpu result does not match \n");
    }
    
    
}


int main(int argc, char *argv[]){
  run();
  return 0;
}
