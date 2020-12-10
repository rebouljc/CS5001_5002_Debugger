#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/module.h>
#include <linux/sched.h>

#include "debugger.h"


int Debugger::get_number_registers(){
        return 10;
}

unsigned long Debugger::list_of_processes(Debugger::Process* out_processes, unsigned long max){

        struct dirent* de_DirEntity = 0;
        DIR* dir_proc = 0;
        unsigned long num_processes = 0;

        dir_proc = opendir("/proc");
        if(dir_proc == 0){
                return 0;
        }

        while( (de_DirEntity = readdir(dir_proc)) && num_processes < max){
                char c = de_DirEntity->d_name[0];
                for(int i = 0; i < 50 && c != 0; i++){
                    c = de_DirEntity->d_name[i];
                    out_processes[num_processes].short_name[i] = c;
                }
                num_processes++;
        }

        return num_processes;
}
