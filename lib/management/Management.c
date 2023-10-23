#include "Management.h"

/*
    see description of the functions in Management.h
*/

simplepim_management_t* table_management_init(uint32_t num_dpus){

    struct dpu_set_t set;
    DPU_ASSERT(dpu_alloc(num_dpus, NULL, &set));
    
    small_table_init(set);
    simplepim_management_t* management = malloc(sizeof(simplepim_management_t));
    management->set = set;
    management->num_dpus = num_dpus;
    management->num_tables = 0;
    management->curr_space = 16;
    management->tables = malloc(sizeof(table_host_t*)*16);
    management->zip_args = malloc(sizeof(zip_arguments_t)*num_dpus);
    management->map_args = malloc(sizeof(map_arguments_t)*num_dpus);
    management->red_args = malloc(sizeof(gen_red_arguments_t)*num_dpus);
    management->free_space_start_pos = 0;
    return management;
}

void add_table(table_host_t* table, simplepim_management_t* management){
    uint32_t num_tables = management->num_tables;
    for(int i=0; i<num_tables; i++){
        if(strcmp("", management->tables[i]->name)==0){
            free(management->tables[i]->lens_each_dpu);
            free(management->tables[i]);
            management->tables[i] = table;
            return;
        }
    }

    uint32_t curr_space = management->curr_space;   
    if(curr_space == num_tables){
        management->tables = realloc(management->tables, (curr_space+16)*(sizeof(table_host_t*)));
        management->tables[num_tables] = table;
        management->num_tables++;
        management->curr_space+=16;
    }
    else{
        management->tables[num_tables] = table;
        management->num_tables++;
    }
}


uint32_t contains_table(const char* name, simplepim_management_t* management){
    uint32_t num_tables = management->num_tables;
    for(int i=0; i<num_tables; i++){
        if(strcmp(name, management->tables[i]->name)==0){
            return 1;
        }
    }

    return 0;
}

void free_table(const char* name, simplepim_management_t* management){
    if(!contains_table(name, management)){
        return;
    }
    lookup_table(name, management)->name = "";
}

table_host_t* lookup_table(const char* name, simplepim_management_t* management){
    uint32_t num_tables = management->num_tables;
    for(int i=0; i<num_tables; i++){
        if(strcmp(name, management->tables[i]->name)==0){
            return management->tables[i];
        }
    }

    printf("table ");
    printf(name);
    printf(" is not contains in current management unit\n");
    return NULL;
}

uint32_t max_len_dpu(uint32_t num_dpus, table_host_t* table){
    uint32_t max_len = 0;
    for(int i=0; i<num_dpus; i++){
        max_len = table->lens_each_dpu[i]>max_len?table->lens_each_dpu[i]:max_len;
    }
    return max_len;

}
