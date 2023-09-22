#ifndef PROCESSINGHELPER_H
#define PROCESSINGHELPER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <alloc.h>
#include <defs.h>
#include <mram.h>
#include <barrier.h>
#include "mutex.h"

uint64_t copy_block_size_fun(uint32_t type_size1, uint32_t type_size2,  uint32_t num_elem);

#endif 