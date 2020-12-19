// Maybe eventually we can get rid of all these imports...
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h> // open, read
#include <unistd.h> // close... why is this not in the same .h as open???
#include <dirent.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <errno.h> // getting error codes from waitpid

// temporary includes
#include <cstdio> // printf 
#include <stdio.h> // printing out the errno error text 

#include "debugger.h"


void copy(void* dest, void* src, unsigned long bytes){
    for(unsigned long i = 0; i < bytes; i++){
        ((char*)dest)[i] = ((char*)src)[i];
    }
}

void copy(void* dest, void* src, unsigned long bytes, unsigned long offset){
    for(unsigned long i = offset; i < bytes+offset; i++){
        ((char*)dest)[i] = ((char*)src)[i];
    }
}


int Debugger::get_number_registers(){
        return 10;
}

int Debugger::debug_init(Debugger::DebuggerData* data){ 
    // Same as windows, probably need to have some kind of memset, maybe
    // there's something here that's better than the windows one.
    for(int i = 0; i < DEBUGGER_MAX_PATH; i++){
        data->exe_path[i] = 0;
    }
    return 0;
} 

int Debugger::debug_loop(Debugger::DebuggerData* data){
    int wait_status;

    if(data->debugging){
        int wait_result = waitpid(data->pid, &wait_status, WNOHANG); 
        if(wait_result == -1){
            if(errno == ECHILD){
                // There are no child processes, so the target process must have died.
                data->debugging = false;
                data->running = false;
            }else{
                perror("waitpid encountered an error");
            }
        }
        if(wait_status != 0){
            printf("wait_status: %i\n", wait_status);

            // available macros for interpretting wait_status:
            // WIFEXITED, WEXITSTATUS, WIFSIGNALED, WTERMSIG, WCOREDUMP, WIFSTOPPED, WSTOPSIG, WIFCONTINUED
                
            if(WIFSTOPPED(wait_status)){
                // I believe this is what is called when we hit a breakpoint
                printf("stopped\n");
                ptrace(PTRACE_CONT, data->pid, nullptr, nullptr);
            }
            if(WIFEXITED(wait_status)){
                // Child exited by calling exit(...)
                // we probably want the exit status of a process
                printf("exited\n");
                data->debugging = false;
                data->running = false;
                data->pid = 0;
            }
            if(WIFSIGNALED(wait_status)){
                printf("signaled\n");
            }
            if(WIFCONTINUED(wait_status)){
                printf("continued\n");
            }
        }
    }
    return 0;
} 

int Debugger::start_and_debug_exe(Debugger::DebuggerData* data){ 
    unsigned long pid = fork(); // clone/vfork/posix_spawn should be looked into : https://lwn.net/Articles/360509/

    if(pid == 0){
        // we're in the target process
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr); // Allow the parent process to debug the child 
        execl(data->exe_path, data->exe_path, nullptr); // Is this the right exec function we want to use? probably want the -e option (check man exec) to specify the environment we want to launch the child from.
    }//else if(pid >= 1){
        // the debugger process... Not sure if we have to do anything here?
    //} // leaving this for discussion later
    data->pid = pid;
    if(pid == (unsigned long)getpid()){
        printf("the pid collected is the same as the parents?? %lu, %i\n", pid, getpid());
    }
    // Windows has a thread id. Is that common among other operating systems?
    // Can we specify extra --  OS-specific -- data for the debugger to display somehow?
    data->running = true;
    data->debugging = true;
    return 0;
} 

unsigned long Debugger::list_of_processes(Debugger::Process* out_processes, unsigned long max){
        // This might not work for all Unix machines
        // some Unix machines don't have /proc

        struct dirent* de_DirEntity = 0;
        DIR* dir_proc = 0;
        unsigned long num_processes = 0;

        // /proc contains an entry for each running process
        dir_proc = opendir("/proc");
        if(dir_proc == 0){
                return 0;
        }

        while( (de_DirEntity = readdir(dir_proc)) && num_processes < max){
                //d_name is actully the PID, will need to convert that to an integer...
                char c = de_DirEntity->d_name[0];
                unsigned long pid = 0;
                if(c >= '0' && c <= '9'){ // Is a PID
                    // Get the name from the comm file
                    int i = 0;
                    for(i = 0; i < 50 && c != 0; i++){
                        pid *= 10;
                        pid += c-'0';
                        // This is to get the comm file path to pass to open()
                        out_processes[num_processes].short_name[i+6] = c; // /proc/<pid>/comm
                        c = de_DirEntity->d_name[i+1];
                    }
                    out_processes[num_processes].pid = pid;
                    copy((void*)out_processes[num_processes].short_name, (void*)"/proc/", 6);
                    copy((void*)(out_processes[num_processes].short_name + i + 6), (void*)"/comm", 5);
                    out_processes[num_processes].short_name[i+12] = 0; // /proc/<pid>/comm

                    int comm_fd = open(out_processes[num_processes].short_name, O_RDONLY);
                    if(comm_fd == -1){ // opening the comm file failed. write the name as ...
                        copy((void*)out_processes[num_processes].short_name, (void*)"...\0", 4);
                    }else{
                        int count = read(comm_fd, (void*)out_processes[num_processes].short_name, 50);
                        if(count > 0){
                            out_processes[num_processes].short_name[count] = 0;
                        }else{ // The read failed?
                            copy((void*)out_processes[num_processes].short_name, (void*)"...\0", 4);
                        }
                        close(comm_fd);
                    }

                }else{ // It's a name, and doesn't have a PID(?)
                        for(int i = 0; i < 50 && c != 0; i++){
                            c = de_DirEntity->d_name[i];
                            out_processes[num_processes].short_name[i] = c;
                        }
                }
                num_processes++;
        }

        return num_processes;
}
