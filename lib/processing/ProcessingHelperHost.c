#include "ProcessingHelperHost.h"
handle_t* create_handle(const char* func_pathname, uint32_t func_type){

    handle_t* handle = malloc(sizeof(handle_t));
    handle->func_type = func_type;
    handle->bin_location = malloc(2048);
    char func_bodyname[2048];
    strcpy(func_bodyname, func_pathname);
    for(int i=0; i<2048; i++){
            if(func_bodyname[i] == '.'){
                func_bodyname[i] = '_';
            }
    }

    if(func_type == 0){
        char compile_cmd[2048] = "dpu-upmem-dpurte-clang -O2 -DNR_TASKLETS=12 -I./ -D__mapfunc_filename__=\"<"; 
        strcat(compile_cmd, func_pathname);
        strcat(compile_cmd, "/map.h>\" -o bin/dpu_map_");
        strcat(compile_cmd, func_bodyname);
        strcat(compile_cmd, " ../../lib/processing/map/map_dpu.c ../../lib/processing/ProcessingHelper.c");
        int succ = system(compile_cmd);

        char bin_location[2048] = "bin/dpu_map_";
        strcat(bin_location, func_bodyname);
        strcpy(handle->bin_location, bin_location);
    }
    else if(func_type == 1){
        char compile_cmd[2048] = "dpu-upmem-dpurte-clang -O2 -DNR_TASKLETS=12 -I./ -D__mapredfunc_pathname__=\"<"; 
        strcat(compile_cmd, func_pathname);
        strcat(compile_cmd, "/map_to_val_func.h>\" -D__combinefunc_pathname__=\"<");
        strcat(compile_cmd, func_pathname);
        strcat(compile_cmd, "/init_combine_func.h>\" -o bin/dpu_genred_");
        strcat(compile_cmd, func_bodyname);
        strcat(compile_cmd, " ../../lib/processing/gen_red/gen_red_dpu.c ../../lib/processing/ProcessingHelper.c ../../lib/TableHost.c ../../lib/Table.c ../../lib/TableShared.c");
        int succ = system(compile_cmd);

        char bin_location[2048] = "bin/dpu_genred_";
        strcat(bin_location, func_bodyname);
        strcpy(handle->bin_location, bin_location);

        
        // generate .o file for dynamic linking
        char h_fname[2048] = "";
        strcat(h_fname, func_pathname);
        strcat(h_fname, "/init_combine_func.h");

        char c_fname[2048] = "";
        strcat(c_fname, func_pathname);
        strcat(c_fname, "/init_combine_func.c");

        char o_fname[2048] = "";
        strcat(o_fname, func_pathname);
        strcat(o_fname, "_init_combine_func.o");
        
        char so_fname[2048];
        strcpy(so_fname, o_fname);
        so_fname[strlen(so_fname)-1] = '\0';
        strcat(so_fname, "so");

        char cp_cmd[2048] = "cp ";
        strcat(cp_cmd, h_fname);
        strcat(cp_cmd, " ");
        strcat(cp_cmd, c_fname);
        succ = system(cp_cmd);

        char compile_cmd1[2048] = "gcc -c -fPIC -o";
        strcat(compile_cmd1, " bin/");
        strcat(compile_cmd1, o_fname);
        strcat(compile_cmd1, " ");
        strcat(compile_cmd1, c_fname);
        succ = system(compile_cmd1);

 
        char compile_cmd2[2048] = "gcc -shared -o bin/";

        strcat(compile_cmd2, so_fname);
        strcat(compile_cmd2, " bin/");
        strcat(compile_cmd2, o_fname);
        succ = system(compile_cmd2);


        char compile_cmd3[2048] = "rm ";
        strcat(compile_cmd3, c_fname);
        succ = system(compile_cmd3);

        handle->so_bin_location = malloc(2048);
        char so_bin_location[2048] = "bin/";
        strcat(so_bin_location, so_fname);
        strcpy(handle->so_bin_location, so_bin_location);
        


    }
    else if(func_type == 2){
        char compile_cmd[2048] = "dpu-upmem-dpurte-clang -O2 -DNR_TASKLETS=12 -o bin/dpu_zip";
        strcat(compile_cmd, " ../../lib/processing/zip/zip_dpu.c ../../lib/processing/zip/ZipProcessing.c ../../lib/processing/ProcessingHelper.c");
        int succ = system(compile_cmd);

        char bin_location[2048] = "bin/dpu_zip";
        strcpy(handle->bin_location, bin_location);

    }
    else{
        printf("function handle not properly compiled!!!");
        return NULL;
    }

    return handle;

}
