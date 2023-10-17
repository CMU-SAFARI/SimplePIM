#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <dpu.h>
#include <omp.h>

#include "../../lib/processing/map/Map.h"
#include "../../lib/processing/zip/Zip.h"
#include "../../lib/processing/ProcessingHelperHost.h"
#include "../../lib/communication/CommOps.h"
#include "../../lib/management/Management.h"
#include "../../lib/timer.h"
#include "Param.h"



void init(T* A, uint32_t salt){
    for (uint64_t i = 0; i < nr_elements; i++) {
        A[i] = (i + salt)%128;
    }
}

void zip(T* A, T* B, T* res){
    for (uint64_t  i = 0; i < nr_elements; i++){
        res[2*i] = A[i];
        res[2*i+1] = B[i];
    }
}

void vec_add(T* A, T* res){
    for (uint64_t i = 0; i < nr_elements; i++){
        res[i] = A[i*2] + A[i*2+1];
    }
}

void vector_addition_host(T* A, T* B, T* res) {
    omp_set_num_threads(16);
    #pragma omp parallel for
    for (uint64_t  i = 0; i < nr_elements; i++) {
        res[i] = A[i] + B[i];
    }
}


void run(){
    simplepim_management_t* table_management = table_management_init(dpu_number);
    T* A = (T*)malloc_scatter_aligned(nr_elements, sizeof(T), table_management);
    T* B = (T*)malloc_scatter_aligned(nr_elements, sizeof(T), table_management);

    T* correct_res = (T*)malloc((uint64_t)sizeof(T)*nr_elements);
    init(A, 0);
    init(B, 1);
    vector_addition_host(A, B, correct_res);

    Timer timer;
    start(&timer, 0, 0);
    start(&timer, 5, 0);
    simplepim_scatter("t1", A, nr_elements,  sizeof(T), table_management);
    simplepim_scatter("t2", B, nr_elements,  sizeof(T),  table_management);
    stop(&timer, 0);
    printf("end of data transfer\n");

    handle_t* add_handle = create_handle("va_funcs", MAP);
    handle_t* zip_handle = create_handle("", ZIP);


    start(&timer, 1, 0);
    table_zip("t1", "t2", "t3",  zip_handle, table_management);
    table_map("t3", "t4", sizeof(T), add_handle, table_management, 0);
    stop(&timer, 1);

    
    if(print_info){
        struct dpu_set_t set, dpu;
        set = table_management->set;
      DPU_FOREACH(set, dpu) {
        DPU_ASSERT(dpu_log_read(dpu, stdout));
      }
    }
    

    start(&timer, 2, 0);
    T* res = simplepim_gather("t4", table_management);
    stop(&timer, 2);
    
    printf("the total time with timing consumed is (ms): ");
    print(&timer, 5, 1);
    printf("\n");
    printf("initial CPU-DPU input transfer (ms): ");
	print(&timer, 0, 1);
    printf("\n");
	printf("DPU Kernel Time (ms): ");
	print(&timer, 1, 1);
    printf("\n");
    printf("DPU-CPU Time (ms): ");
	print(&timer, 2, 1);
    printf("\n");

    
    int32_t is_correct = 1;

    for(int i=0; i<nr_elements; i++){
        if(res[i]!=correct_res[i]){
            is_correct = 0;
            printf("result mismatch at position %d, got %d, expected %d \n", i, res[i], correct_res[i]);
            break;
        }
    } 

    if(is_correct){
        printf("the result is correct \n");
    }  
    
    

}

int main(int argc, char *argv[]){
  run();
  return 0;
}
