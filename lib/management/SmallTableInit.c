#include "SmallTableInit.h"
#define DPU_BINARY "bin/dpu_init_binary"

void small_table_init(struct dpu_set_t set){
    system("dpu-upmem-dpurte-clang -O2 -DNR_TASKLETS=1 -o bin/dpu_init_binary ../../lib/management/SmallTableInit_dpu.c");
    DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));
    DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
}