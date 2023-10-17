#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <dpu.h>

#include "../../lib/processing/gen_red/GenRed.h"
#include "../../lib/processing/ProcessingHelperHost.h"
#include "../../lib/timer.h"
#include "Param.h"



void init_data(T* A){
    for(unsigned long i=0; i<nr_elements; i++){
        A[i] = i % 4096;
    }
} 

void zero_out_hist(uint32_t* histo){
    for(int i=0; i<bins; i++){
        histo[i] = 0;
    }
}

void histogram_host(uint32_t* histo, T* A){
    for (unsigned int j = 0; j < nr_elements; j++) {
            T d = A[j];
            histo[(d * bins) >> DEPTH] += 1;
        }
}

void printf_hist(uint32_t* histo){
    printf("the bins :\n");
    for(int i=0; i<bins; i++){
        printf("%d ", histo[i]);
        if(i%8 == 7){
            printf("\n");
        } 
    }
}

void add(void* p1, void* p2){
    uint32_t* ptr1 = (uint32_t*)(p1);
    uint32_t* ptr2 = (uint32_t*)(p2);
    *ptr1 += *ptr2;  
}

void run(){
    printf("the number of elements %lu\n", nr_elements);
    simplepim_management_t* table_management = table_management_init(dpu_number);
    T* A = (T*)malloc_scatter_aligned(nr_elements, sizeof(T), table_management);

    uint32_t* histo = (uint32_t*)malloc(sizeof(uint32_t)*bins);
    init_data(A);
    zero_out_hist(histo);
    histogram_host(histo, A);
    printf_hist(histo);

    simplepim_scatter("t1", A, nr_elements, sizeof(T), table_management);
    printf("end of data transfer\n");
 
    handle_t* va_handle = create_handle("hist_funcs", REDUCE);
	

    T* res = table_gen_red("t1", "t2", sizeof(uint32_t), bins, va_handle, table_management, 0);

    printf("dpu results:\n");
    printf_hist(res);

}

int main(int argc, char **argv){
    srand(17); 
    run();
    return 0;
}
