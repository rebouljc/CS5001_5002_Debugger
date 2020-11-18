#include "imgui.h"

struct Persistant_Vars {
	bool show_demo_window;
	bool show_another_window;
	bool show_yet_another_window;
	ImVec4 clear_color;
};

void init_vars(Persistant_Vars *vars);
void main_ui_loop(Persistant_Vars *vars);