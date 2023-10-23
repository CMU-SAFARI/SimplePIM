#ifndef COMMOPS_H
#define COMMOPS_H
#include "CommHelper.h"
#include "../management/Management.h"
#include "../processing/ProcessingHelperHost.h"
#include <dpu.h>
#include <stdint.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

/*
malloc_*_aligned allocates memory on host for alignent requirements of UPMEM, one can treat the returned pointer 
as normal heap memory obtained by calling "malloc"

simplepim_* (for example simplepim_scatter) implements the communication operators as in the paper
*/


void* malloc_scatter_aligned(uint64_t len, uint32_t type_size, simplepim_management_t* table_management);
void* malloc_reduce_aligned(uint32_t len, uint32_t type_size, simplepim_management_t* table_management);
void* malloc_broadcast_aligned(uint32_t len, uint32_t type_size, simplepim_management_t* table_management);
void simplepim_scatter(char* const table_id, void* elements, uint64_t len, uint32_t type_size, simplepim_management_t* table_management);
void* simplepim_gather(char* const table_id, simplepim_management_t* table_management);
void simplepim_broadcast(char* const table_id, void* elements, uint64_t len, uint32_t type_size, simplepim_management_t* table_management);
void simplepim_allgather(char* const table_id, char* const new_table_id, simplepim_management_t* table_management);
void simplepim_allreduce(char* const table_id, handle_t* binary_handle, simplepim_management_t* table_management);
#endif 
