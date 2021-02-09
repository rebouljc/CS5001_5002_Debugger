#include "SourceCodeViewer.h"
#include <windows.storage.pickers.h>
#include "OSPlatformUI.h"
#include "Persistant_Vars.h"
#include "debugger.h"
#include <string>
#include <fstream>

//James C. Reboulet
//Default Constructor


SourceCodeViewer::SourceCodeViewer()
{
	//We have to go through all of this, allocating the string on the heap and initializing all of its values to NULL
	//so we don't get junk at the end of the file path.
	
	this->openFilePath = new char[sizeof(char) * (DEBUGGER_MAX_PATH*2)];
	for (int i = 0; i < DEBUGGER_MAX_PATH; ++i)
	{
		this->openFilePath[i] = '\0';
	}

}

SourceCodeViewer::~SourceCodeViewer()
{
	delete [] this->openFilePath;
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
	ImGuiWindowFlags windowFlags = 0;
	windowFlags |= ImGuiWindowFlags_MenuBar;
	windowFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
	windowFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;


	if (ImGui::Begin("Source Code Viewer",0, windowFlags))
	{
		if (this->fileOpenFlag)
		{
			ImGuiID childId = ImGui::GetID((void*)(intptr_t)0);
			float childW = 1.0;
			
			
			if (ImGui::BeginChild(childId, ImVec2(2000.0f, 2000.0f), true, 0))
			{
				
				const int MAX_BUFFER_SIZE = 1000;
				for (int i = 0; i < this->fileContentsVector.size(); ++i)
				{
					int stringSize = 0;
					char* currentString;
					stringSize = this->fileContentsVector.at(i).size();
					
					currentString = new char[MAX_BUFFER_SIZE];
					for (int stIndex = 0; stIndex < MAX_BUFFER_SIZE; ++stIndex)
					{
						currentString[stIndex] = NULL;
					}

					
					

					for (int j = 0; j < stringSize; ++j)
					{
						currentString[j] = this->fileContentsVector.at(i).at(j);
					}

				
					string label = "##";
					label.push_back(i);

					
					ImGui::InputText(label.c_str(), currentString, (size_t)(MAX_BUFFER_SIZE));
					
					//Now, we have to write the info from buffer back into the string.
					this->fileContentsVector.at(i).clear();
					for (int j = 0; j < MAX_BUFFER_SIZE; ++j)
					{
						if (currentString[j] != NULL)
						{
							this->fileContentsVector.at(i).push_back(currentString[j]);
						}
					}
				}

				ImGui::EndChild();
			}
		}
	     
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{

				if (ImGui::MenuItem("Open"))
				{
					
					//Call the private OpenSourceFile method.
					
					
					OSPlatformUI::open_file(this->openFilePath, this->currentHandle, this->pathSize);
					printf("\n Open File Path: ");

					std::string openFileResult;
					for(int i = 0; i < this->pathSize; ++i)
					{
						
					      printf("%c", this->openFilePath[i]);
						  openFileResult.push_back(this->openFilePath[i]);
						 
						
					}

					ifstream input;
					char currentChar;
					input.open(this->openFilePath);
					while (!input.eof())
					{
						currentChar = input.get();
						if(currentChar != '\n')
						{
							this->characterVector.push_back(currentChar);
							continue;
						}
						this->characterVector.push_back(currentChar);
						this->fileContentsVector.push_back(this->characterVector);
						this->characterVector.clear();
					}

					input.close();
					this->fileOpenFlag = true;

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




	







