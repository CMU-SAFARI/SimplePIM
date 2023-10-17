#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


void read_csv_to_arr(FILE* fp, T* arr, int32_t len, int32_t d){
  if (fp == NULL) {
        fprintf(stderr, "Error reading file\n");
        return;
  }

  float tmp;
  for (size_t i = 0; i < len; i++){
      for(size_t j = 0; j < d-1; j++){
        fscanf(fp, "%f,", &tmp);
        arr[i*d+j] = (T)tmp;
      }
      fscanf(fp, "%f,", &tmp);
      arr[i*d+d-1] = (T)tmp;
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

void get_output_file(int num_dpus, int dim, int num_elem){
  char str1[10];
  char str2[10];
  char str3[10];
  sprintf(str1, "%d", num_dpus);
  sprintf(str2, "%d", dim);
  sprintf(str3, "%d", num_elem);
  char out[100] = "results/framework_";
  strcat(out, str1);
  strcat(out,"_");
  strcat(out, str2);
  strcat(out,"_");
  strcat(out, str3);
  fp = fopen (out, "w");
}


void compute_gradients(const T*arr){

  // [X|Y] -> [X], [Y]
  T* X = malloc(num_elements*dim*sizeof(T));
  T* Y = malloc(num_elements*sizeof(T));
  for(uint32_t i=0; i<num_elements; i++){
    for(uint32_t j=0; j<dim; j++){
      X[i*dim+j] = arr[i*(dim+1)+j];
    }
    Y[i] = arr[i*(dim+1)+dim];
  }

  // actual code
  T* weights = malloc(dim*sizeof(T));

  for (uint32_t n = 0; n < dim; n++) {
    weights[n] = 0;
  }

  int64_t dot_product; 
  int64_t e;
  int64_t* gradient_tmp = (int64_t*) calloc(dim, sizeof(int64_t)); 
  for (uint32_t i = 0; i < iter; ++i){
    for (uint32_t n = 0; n < dim; ++n) {
      gradient_tmp[i] = 0;
    }

    for (uint32_t j = 0; j < num_elements; ++j) {
        dot_product = 0; 
        for (uint32_t k = 0; k < dim; k++) {
          dot_product += X[j*dim + k] * weights[k]; 
        }

        e = dot_product-(Y[j]<<shift_amount);
        for (uint32_t l = 0; l < dim; l++) {
                gradient_tmp[l] += X[j*dim + l] * e >> prevent_overflow_shift_amount; 
        }
    }
  }

  printf("\nthe gradients on host: \n");
  for(int i=0; i<dim; i++){
    printf("%lld ", gradient_tmp[i]);
  }
  printf("\n");

}





int main(){
  simplepim_management_t* table_management = table_management_init(dpu_number);

  printf("dim: %d, num_elem: %d, iter: %d, lr: %f \n", dim, num_elements, iter, lr);
  // data contains y also as last element

  // inputs
  //printf("reading the input data\n");
  T* elements = (T*)malloc_scatter_aligned(num_elements, (dim+1)*sizeof(T), table_management);

  
  fp = fopen ("data/input.csv", "r");
  read_csv_to_arr(fp, elements, num_elements, dim+1);

  if(print_info){
    for(int i=0; i<num_elements; i++){

      for(int j=0; j<dim+1; j++){
        printf("%f ", elements[i*dim+j]);
      }
      printf("\n");
      printf("\n");
    }
  }
  
  // weights data
  printf("init weight data \n");
  T* weights = (T*)malloc_broadcast_aligned(1, sizeof(T)*dim, table_management);
  for(int i=0; i<dim; i++){
      weights[i] = 0.0;
  }

  printf("end of reading data from file\n");

  simplepim_scatter("t1", elements, num_elements, (dim+1)*sizeof(T), table_management);
  uint32_t data_offset = lookup_table("t1", table_management)->end; 
  simplepim_broadcast("t2", weights, 1, dim*sizeof(T),  table_management);

  handle_t* va_handle = create_handle("log_reg_funcs", REDUCE);
  T* res = table_gen_red("t1", "t3", dim*sizeof(T), 1, va_handle, table_management, data_offset);

  printf("the weights of linear model: \n");
  for(int i=0; i<dim; i++){
    res[i] /= num_elements;
    weights[i] -= lr * res[i];
    printf("%4.3e ", weights[i]);
  }

  free(res);

  //printf("end of data transfer\n");

  /*
  // preparing and parsing argument
  dpu_arguments_t* input_args = (dpu_arguments_t*) malloc(num_dpus * sizeof(dpu_arguments_t));

  for(int i=0; i<num_dpus; i++){
     input_args[i].input_start_offset = 0;
     input_args[i].input_type_size = dim*sizeof(T);
     input_args[i].data_start_offset = data_offset;
     input_args[i].data_len = 1;
     input_args[i].data_type_size = X_dim*sizeof(T);
     input_args[i].end_offset = end_offset;
     input_args[i].table_type_size = X_dim*sizeof(T);
     input_args[i].table_len = 1;
  }


  prepare_input_len_and_parse_args(set, input_args, num_elements, dim*sizeof(T), num_dpus);
  stop(&timer, 0);
  //printf("end of parsing arguments to upmem\n");

  //init buffer for gathering all weights data
  T* grads_table = (T*)malloc(sizeof(T)*X_dim);

  // launch actual dpu code
  for(int it=0; it<iter; it++){
    start(&timer, 1, it);
    DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
    stop(&timer, 1);

    if(print_info){
      DPU_FOREACH(set, dpu) {
        DPU_ASSERT(dpu_log_read(dpu, stdout));
      }
    }

    start(&timer, 2, it);
    gather_tables_to_host(set, grads_table, 1, X_dim*sizeof(T), end_offset, num_dpus, zero_init, add);
    stop(&timer, 2);

    //print_table_entries(grads_table, 1, X_dim*sizeof(float), print_floats);

    start(&timer, 3, it);
    for(int i=0; i<X_dim; i++){
      grads_table[i] /= num_elements;
      //printf("%f ", grads_table[i]);
    }
    //printf("\n");

    //update gradients
    for(int i=0; i<X_dim; i++){
        weights[i] -= lr * grads_table[i];
    }
    stop(&timer, 3);

    start(&timer, 4, it);
    host_broadcast_to_dpu(set, weights, 1, X_dim*sizeof(T), data_offset);
    stop(&timer, 4);
  }
  stop(&timer, 5);

  printf("the total time with timing consumed is (ms): ");
  print(&timer, 5, 1);
  printf("\n");
  printf("initial CPU-DPU input transfer (ms): ");
	print(&timer, 0, 1);
  printf("\n");
	printf("DPU Kernel Time (ms): ");
	print(&timer, 1, iter);
  printf("\n");
	printf("DPU-CPU Time (ms): ");
	print(&timer, 2, iter);
  printf("\n");
	printf("CPU combine table Time (ms): ");
	print(&timer, 3, iter);
  printf("\n");
	printf("CPU-DPU Time (ms): ");
	print(&timer, 4, iter);
  printf("\n");

  float total_time = timer.time[0];
  for(int i=1; i<5; i++){
    total_time += timer.time[i];
  }
  
  printf("total time added up (ms): %f\n", total_time/1000);

  double* times = (double*)malloc(sizeof(double)*6);
  for(int i=0; i<5; i++){
    times[i] = timer.time[i];
  }
  times[5] = total_time;
  get_output_file(num_dpus, X_dim, num_elements);
  write_time_to_csv(times, 6);


  printf("the weights of linear model: \n");
  for(int i=0; i<X_dim; i++){
    printf("%4.3e ", weights[i]);
  }
  printf("\n");

  free(grads_table);
  return 0;
  */
}
