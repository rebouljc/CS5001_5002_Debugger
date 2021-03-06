#include "main_ui.h"
#include <OSPlatformUI.h>


void demo_code(Persistant_Vars *vars);

void init_vars(Persistant_Vars* vars) {
	Debugger::debug_init(&vars->debug_data);
	vars->show_another_window = false;
	vars->show_demo_window = true;
	vars->show_yet_another_window = false;
	vars->rax = 0;
	vars->clear_color = ImVec4(0.60f, 0.55f, 0.60f, 1.00f); // Don't remove this, the platform_main.cpp uses this to draw the background color
	vars->num_registers = Debugger::get_number_registers();
	memset(vars->processes, 0, sizeof(vars->processes));
	vars->num_processes = Debugger::list_of_processes(vars->processes, DEBUGGER_MAX_PROCESSES);
	// Should change that dependency in the future

	//Create SourceCodeViewerWindow Object.  Will do looping in main_ui_loop
	vars->srcCodeViewWindow.push_back(new SourceCodeViewer());
	vars->srcCodeViewWindow.at(0)->setPersistantVars(vars);
	
}

void draw_cpu_registers(int num_registers) {
    // Maybe this can be some sort of widget for showing the cpu registers
    ImGui::Begin("CPU Registers");

	for (int i = 0; i < num_registers; i++) {
		ImGui::Text("r%i", i);
	}

    ImGui::End();
}

void draw_processes(Debugger::Process* processes, unsigned long &num_processes) {


	static ImGuiTextFilter filter("-..."); // TODO: move to PersistantVars.
	static int processes_displayed = 0; // TODO: not sure if we're going to keep this
	ImGui::Begin("Processes"); 
	
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
    ImGui::Text("Num Processes: %i", num_processes);
	ImGui::SameLine();
    ImGui::Text("Num Displayed: %i", processes_displayed);
	ImGui::SameLine();


	if (ImGui::Button("Refresh")) {
		num_processes = Debugger::list_of_processes(processes, DEBUGGER_MAX_PROCESSES);
	}
	// TODO: do something about column widths
	ImGui::Columns(3);
	ImGui::NextColumn();
	ImGui::NextColumn();
	filter.Draw();
	ImGui::NextColumn();
	ImGui::Separator();
	ImGui::Text("#"); ImGui::NextColumn();
	ImGui::Text("PID"); ImGui::NextColumn();
	ImGui::Text("Short Name"); ImGui::NextColumn();
	ImGui::Separator();

	// TODO: move this static variable to Persistant_Vars
	//static int selected = -1;

	processes_displayed = 0;
	for (int i = num_processes-1; i >= 0; i--) {
		//ImGui::Text("Processes: %s\tPID: %i", processes[i].short_name, processes[i].pid);
		if (filter.PassFilter(processes[i].short_name)) {
			processes_displayed++;
			ImGui::Text("%i", i); ImGui::NextColumn();
			ImGui::Text("%u", processes[i].pid); ImGui::NextColumn();
			ImGui::Selectable(processes[i].short_name, false, ImGuiSelectableFlags_SpanAllColumns); ImGui::NextColumn();
			//ImGui::Text("%s", processes[i].short_name); ImGui::NextColumn();
		}
	}
	ImGui::PopStyleVar();
    ImGui::End();
}

void draw_start_window(Persistant_Vars* vars) {
	ImGui::Begin("Start Window");

	int dummy = 0;
	if (ImGui::Button("Select exe...")) {
		OSPlatformUI::open_file(vars->debug_data.exe_path, dummy);
	}
	ImGui::SameLine();
	ImGui::PushItemWidth(-1);
	ImGui::InputText(" ", vars->debug_data.exe_path, DEBUGGER_MAX_PATH);
	ImGui::PopItemWidth();

	if (ImGui::Button("Start Process")) {
		Debugger::start_and_debug_exe(&vars->debug_data);
	}
	if (vars->debug_data.running) {
		ImGui::Text("PID: %u", vars->debug_data.pid);
		ImGui::Text("TID: %u", vars->debug_data.tid);
	}
	ImGui::Text("Debugging: ");
	ImGui::SameLine();
	if (vars->debug_data.debugging) {
		ImGui::Text("True");
	}
	else {
		ImGui::Text("False");
	}
	ImGui::End();
}

void main_ui_loop(Persistant_Vars *vars) {
	
	// Refresh the list of process about every 4 seconds (depending on frame rate)
	if (vars->refresh_processes_timer++ > ImGui::GetIO().Framerate *4) { // TODO: not sure if we're going to keep this
		// There's probably a good time when we shouldn't be updating this all the time.
		// Like when the window is closed
		vars->refresh_processes_timer = 0;
		vars->num_processes = Debugger::list_of_processes(vars->processes, DEBUGGER_MAX_PROCESSES);
	}

	draw_start_window(vars);
	draw_cpu_registers(vars->num_registers);
	draw_processes(vars->processes, vars->num_processes);
    demo_code(vars);
	//Now, we can have multiple SourceCodeViewer windows open at the same time.  We just declare an object for each.
	//We can have an option to open File in new Window.
	for (int i = 0; i < vars->srcCodeViewWindow.size(); ++i)
	{
		vars->srcCodeViewWindow.at(i)->displayLoop();
	}
	
	

	// Should this be here or should we call this from
	// the platform layer???
	Debugger::debug_loop(&vars->debug_data);
}

void demo_code(Persistant_Vars *vars){

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (vars->show_demo_window)
		ImGui::ShowDemoWindow(&vars->show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &vars->show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &vars->show_another_window);
		ImGui::Checkbox("Yet Another Window", &vars->show_yet_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&vars->clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (vars->show_another_window)
	{
		ImGui::Begin("Another Window", &vars->show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			vars->show_another_window = false;
		ImGui::End();
	}

	if (vars->show_yet_another_window)
	{
		ImGui::Begin("Yet Another Window", &vars->show_yet_another_window);
		ImGui::Text("Hello from yet another window!");
		if (ImGui::Button("Close Me"))
			vars->show_yet_another_window = false;
		ImGui::End();

	}

}
