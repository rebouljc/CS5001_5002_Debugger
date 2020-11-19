
#include "main_ui.h"
#include "imgui.h"


void init_vars(Persistant_Vars *vars) { 
	vars->show_another_window = false;
	vars->show_demo_window= true;
	vars->show_yet_another_window= false;

	vars->clear_color = ImVec4(1.00f, 0.55f, 0.60f, 1.00f); // Don't remove this, the platform_main.cpp uses this to draw the background color
	// Should change that dependency in the future
}

void main_ui_loop(Persistant_Vars *vars) {

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
