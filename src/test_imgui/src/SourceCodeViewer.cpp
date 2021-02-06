#include "SourceCodeViewer.h"
#include <iostream>
#include <windows.storage.pickers.h>
#include "OSPlatformUI.h"
#include "Persistant_Vars.h"
#include "debugger.h"




//James C. Reboulet
//Default Constructor
using namespace ABI::Windows::Storage::Pickers;
SourceCodeViewer::SourceCodeViewer()
{
	//We have to go through all of this, allocating the string on the heap and initializing all of its values to NULL
	//so we don't get junk at the end of the file path.
	
	this->openFilePath = (char*)malloc((size_t)DEBUGGER_MAX_PATH);
	for (int i = 0; i < DEBUGGER_MAX_PATH; ++i)
	{
		this->openFilePath[i] = NULL;
	}
	

}

SourceCodeViewer::~SourceCodeViewer()
{
	free(this->openFilePath);
}
void SourceCodeViewer::displayLoop()
{
	this->drawCodeViewerWindow();
}

void SourceCodeViewer::setCurrentHandle(HWND& currentHandle)
{
	this->currentHandle = currentHandle;
}

void SourceCodeViewer::drawCodeViewerWindow()
{

	if (ImGui::Begin("Source Code Viewer",0, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{

				if (ImGui::MenuItem("Open"))
				{
					std::cout << "\nI will now open a file.";
					//Call the private OpenSourceFile method.
					
					
					OSPlatformUI::open_file(this->openFilePath, this->currentHandle, this->pathSize);
					printf("\n Open File Path: ");
					for(int i = 0; i < this->pathSize; ++i)
					{
						
					      printf("%c", this->openFilePath[i]);
						
					}
					
					
				
				}
				else if (ImGui::MenuItem("Save"))
				{
				}
				else if (ImGui::MenuItem("Close"))
				{

				}

				ImGui::EndMenu();

			}
			ImGui::EndMenuBar();
		}

		ImGui::End();
	}
}




	







