#include "TableHost.h"
void combine_table_entries(void* table1, void* table2, uint32_t table_size, uint32_t value_size, void (*combineFunc)(void*, void*)){
    
    uint32_t curr_entry;
 
    for(int i=0; i<table_size; ++i){
        curr_entry = i*(value_size);
        (*combineFunc)(table1+curr_entry, table2+curr_entry);
        
    } 

}



void print_table_entries(void* table, uint32_t table_size, uint32_t value_size, void (*printFunc)(void*)){
   uint32_t curr_entry;
   void* value;

   for(int i=0; i<table_size; ++i){
        curr_entry = i*(value_size);
        value = (void*)(& ((char*)table)[curr_entry]);
        printf("at position %d : ", i);
        (*printFunc)(value);
        
    } 
}