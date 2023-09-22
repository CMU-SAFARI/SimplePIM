#ifndef ZIP_H
#define ZIP_H
#include "ZipArgs.h"
#include "../ProcessingHelperHost.h"
#include "../../management/Management.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dpu.h>
void table_zip(const char* src1_name, const char* src2_name, const char* dest_name,  handle_t* binary_handle, smalltable_management_t* table_management);
#endif 
