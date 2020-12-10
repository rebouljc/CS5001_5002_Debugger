#include "imgui.h"
#include "debugger.h"
#include <stdint.h>

struct Persistant_Vars {
	bool show_demo_window;
	bool show_another_window;
	bool show_yet_another_window;
    uint64_t rax;
	ImVec4 clear_color;
	int num_registers;

	unsigned long refresh_processes_timer = 0;
	unsigned long num_processes;
	Debugger::Process processes[1000];
};

void init_vars(Persistant_Vars *vars);
void main_ui_loop(Persistant_Vars *vars);