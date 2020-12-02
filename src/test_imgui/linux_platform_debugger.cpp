#include <linux/module.h>
#include <linux/sched.h>
#include "debugger.h"

int Debugger::get_number_registers(){
        return 10;
}

unsigned long Debugger::list_of_pids(unsigned long** out_pids, unsigned long max){
        return 10;
}
