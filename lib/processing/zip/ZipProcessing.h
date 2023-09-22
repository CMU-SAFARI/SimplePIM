#ifndef ZIPPROCESSING_H
#define ZIPPROCESSING_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <defs.h>
#include <mram.h>
#include <barrier.h>
#include "../ProcessingHelper.h"

void zip_dpu(__mram_ptr void* table_entries_1, __mram_ptr void* table_entries_2, __mram_ptr void* table_entries_res, uint32_t input_type_1, uint32_t input_type_2, uint32_t len);
#endif 