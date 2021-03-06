#pragma once 
#include <stdint.h>
#include <vector>

#include "imgui.h"
#include "debugger.h"
#include "SourceCodeViewer.h"

// Forward declare the SourceCodeViewer class so that we can use the pointer in the Persistant_Vars struct
// this works becuase pointers are always the same size, so the compiler knows what size SourceCodeViewer*
// should be, no matter what the size SourceCodeViewer is. 
class SourceCodeViewer;

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
	Debugger::DebuggerData debug_data;
	std::vector<SourceCodeViewer*> srcCodeViewWindow;
};

void init_vars(Persistant_Vars *vars);
void main_ui_loop(Persistant_Vars *vars);
