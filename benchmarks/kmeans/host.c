#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <dpu.h>

#include "../../lib/processing/gen_red/GenRed.h"
#include "../../lib/processing/ProcessingHelperHost.h"
#include "../../lib/communication/CommOps.h"
#include "../../lib/management/Management.h"
#include "../../lib/timer.h"
#include "Param.h"



FILE* fp;

void add(void* p1, void* p2){
    uint32_t* times1 = (uint32_t*)p1;
    uint32_t* times2 = (uint32_t*)p2;
    *times1 += *times2;
    T* ptr1 = (T*)(p1+sizeof(uint32_t));
    T* ptr2 = (T*)(p2+sizeof(uint32_t));
    
    for(int i=0; i<dim; i++){
        ptr1[i] = ptr1[i]+ptr2[i];
    }
}

int divRoundClosest(const int n, const int d)
{
  return ((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d);
}

void average(int32_t times, void* f_dest, void* f){
  T* ptr = (T*)f;
  T* ptr_dest = (T*)f_dest;
  for(int i=0; i<dim; i++){
        ptr_dest[i] = times==0?0:divRoundClosest(ptr[i], times);
  }
}

void average_table_entries_to_arr(void* centroid_table, void* centroids){
  int32_t* times;
  void* dest_ptr;
  void* src_ptr;
  for(int i=0; i<k; i++){
    times = (int32_t*)(centroid_table+i*(dim*sizeof(T)+sizeof(int32_t)));
    src_ptr = centroid_table+i*(dim*sizeof(T)+sizeof(int32_t))+sizeof(uint32_t);
    dest_ptr = centroids + i*dim*sizeof(T);
    average(*times, dest_ptr, src_ptr);
  }
}


void read_csv_to_int_arr(FILE* fp, int32_t* arr, int32_t len, int32_t dim){
  if (fp == NULL) {
        fprintf(stderr, "Error reading file\n");
        return;
    }


    for (size_t i = 0; i < len; i++){
      for(size_t j = 0; j < dim-1; j++){
        //fscanf(fp, "%d,", &arr[i*dim+j]);
        arr[i*dim+j] = (i+j)%1000;
      }
      //fscanf(fp, "%d\n", &arr[i*dim+dim-1]);
      arr[i*dim+dim-1] = i%1000;
    }

    fclose(fp);
}



void write_time_to_csv(double* arr, int32_t len){
  if (fp == NULL) {
        fprintf(stderr, "Error reading file\n");
        return;
    }


    for (size_t i = 0; i < len; i++){
      fprintf(fp,"%f\n", arr[i]/1000.0);
    }

    fclose(fp);
}  

void get_output_file(int num_dpus, int dim, int num_elem, int k){
  char str1[10];
  char str2[10];
  char str3[10];
  char str4[10];
  sprintf(str1, "%d", num_dpus);
  sprintf(str2, "%d", dim);
  sprintf(str3, "%d", num_elem);
  sprintf(str4, "%d", k);
  char out[100] = "results/framework_";
  strcat(out, str1);
  strcat(out,"_");
  strcat(out, str2);
  strcat(out,"_");
  strcat(out, str3);
  strcat(out,"_");
  strcat(out, str4);
  fp = fopen (out, "w");
}





void run(){
  simplepim_management_t* table_management = table_management_init(dpu_number);
  printf("k: %d, dim: %d, num_elem: %d, iter: %d \n", k, dim, num_elements, iter);


  // inputs
  T* elements = (T*)malloc_scatter_aligned(num_elements, dim*sizeof(T), table_management);
  
  fp = fopen ("data/input.csv", "r");
  read_csv_to_int_arr(fp, elements, num_elements, dim);

  //printf("additional centroids init data \n");
  T* centroids = (T*)malloc_broadcast_aligned(k, sizeof(T)*dim, table_management);
  for(int i=0; i<k; i++){
    for(int j=0; j<dim; j++){
      centroids[i*dim+j] = elements[i*dim+j];
    }
  }
  printf("end of reading data from file\n");

  handle_t* va_handle = create_handle("kmeans_funcs", REDUCE);

  simplepim_scatter("t1", elements, num_elements, dim*sizeof(T), table_management);; 
  uint32_t data_offset = lookup_table("t1", table_management)->end;
  // main loop
  for(int m=0; m<iter; m++){
    simplepim_broadcast("t2", centroids, k, dim*sizeof(T), table_management);
    T* res = table_gen_red("t1", "t3", dim*sizeof(T)+sizeof(int32_t), k, va_handle, table_management, data_offset);


    average_table_entries_to_arr(res, centroids);
    free_table("t2", table_management);

    for(int i=0; i<k; i++){
      for(int j=0; j<dim; j++){
        printf("%d ", centroids[i*dim+j]);
      }
      printf("\n");
    }
  }

  
  
}


int main(int argc, char **argv){
  run();
  return 0;
}
