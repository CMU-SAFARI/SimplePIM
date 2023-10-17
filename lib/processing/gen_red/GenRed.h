#ifndef GENRED_H
#define GENRED_H

#include "GenRedArgs.h"
#include "../ProcessingHelperHost.h"
#include "../../communication/CommOps.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <omp.h>
#include <dpu.h>

void* table_gen_red(const char* src_name, const char* dest_name,  uint32_t output_type, uint32_t output_len, handle_t* binary_handle, simplepim_management_t* table_management, uint32_t info);
#endif 
