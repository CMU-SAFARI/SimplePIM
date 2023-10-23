#ifndef SMALLTABLEINIT_H
#define SMALLTABLEINIT_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <dpu.h>

/*
    the UPMEM hardware needs to call some setup code before running
    the code is called once management_init is called
*/

void small_table_init(struct dpu_set_t set);

#endif 