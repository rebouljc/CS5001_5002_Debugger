#include "SourceCodeViewer.h"
#include <iostream>
#include <windows.h>



//James C. Reboulet
//Default Constructor

SourceCodeViewer::SourceCodeViewer()
{
	
}

void SourceCodeViewer::displayLoop()
{
	this->drawCodeViewerWindow();
}

void SourceCodeViewer::drawCodeViewerWindow()
{

	if (ImGui::Begin("Source Code Viewer"))
	{

		if (ImGui::BeginMenu("Options Menu"))
		{

			if (ImGui::MenuItem("Open"))
			{
				std::cout << "\nI will now open a file.";
				//Call the private OpenSourceFile method.
				this->openSourceFile();

			}
			else if (ImGui::MenuItem("Save"))
			{
			}
			else if (ImGui::MenuItem("Close"))
			{

			}

			ImGui::EndMenu();

		}

		ImGui::End();
	}
}

bool SourceCodeViewer::openSourceFile()
{
	
	
    
	return true;
}






