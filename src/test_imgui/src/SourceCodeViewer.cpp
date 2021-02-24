#include "SourceCodeViewer.h"
#include "OSPlatformUI.h"
#include "debugger.h"
#include <string>
#include <fstream>

//James C. Reboulet
//Default Constructor


SourceCodeViewer::SourceCodeViewer()
{
	//We have to go through all of this, allocating the string on the heap and initializing all of its values to NULL
	//so we don't get junk at the end of the file path.
	this->windowNum = 0;
	this->scvLabel = "Source Code Viewer##" + windowNum;
	this->openFilePath = new char[sizeof(char) * (DEBUGGER_MAX_PATH*2)];
	for (int i = 0; i < DEBUGGER_MAX_PATH; ++i)
	{
		this->openFilePath[i] = '\0';
	}

}

SourceCodeViewer::~SourceCodeViewer()
{
	delete[] this->openFilePath;
}

void SourceCodeViewer::openFile()
{
	//If a file is already open, do a reinit() of the class.
	
	if (this->fileOpenFlag)
	{
		
		this->reinitSaveMethod();
		this->fileOpenFlag = false;
		
	}

	else if (!this->fileOpenFlag)
	{
		this->reinit();
		this->fileOpenFlag = false;
	}


	OSPlatformUI::open_file(this->openFilePath, this->pathSize);
	
		printf("\n Open File Path: ");


		if (this->openFilePath[0] != NULL && this->fileOpenFlag)
		{
			this->reinit();
			this->fileOpenFlag = false;
		}

		
		if (this->openFilePath[0] != NULL) //This fixes the issue of when the user decides to cancel the dialog, which causes a system crash.
		{
			this->fileContentsVector.clear();
			this->checkboxCheckedVector.clear();
			this->characterVector.clear();

			for (int i = 0; i < this->pathSize; ++i)
			{

				printf("%c", this->openFilePath[i]);
				this->openFileResult.push_back(this->openFilePath[i]);


			}

			ifstream input;
			char currentChar;
			input.open(this->openFilePath);
			while (!input.eof())
			{
				currentChar = input.get();
				if (currentChar != '\n')
				{
					this->characterVector.push_back(currentChar);
					continue;
				}
				this->characterVector.push_back(currentChar);
				this->fileContentsVector.push_back(this->characterVector);
				this->checkboxCheckedVector.push_back(false);
				this->characterVector.clear();
			}

			input.close();
			this->fileOpenFlag = true;
			return;
		}

		this->fileOpenFlag = true;
		
}


void SourceCodeViewer::saveFile()
{

	this->reinitSaveMethod();  //We have to delete and reallocate the open file path.
	OSPlatformUI::save_file(this->openFilePath, this->pathSize);
	printf("\n Save File Path: ");

	ofstream output;
	char currentChar;
	output.open(this->openFilePath);
	if (this->openFilePath[0] != NULL) //This fixes the issue of when the user decides to cancel the dialog, which causes a system crash.
	{
		for (int i = 0; i < this->fileContentsVector.size(); ++i)
		{
			for (int j = 0; j < this->fileContentsVector.at(i).size(); ++j)
			{
				output.put(this->fileContentsVector.at(i).at(j));
			}
		}

		output.close();
	}
}

SourceCodeViewer::SourceCodeViewer(int currentWindowNum)
{
	//We have to go through all of this, allocating the string on the heap and initializing all of its values to NULL
	//so we don't get junk at the end of the file path.
	this->windowNum = ++currentWindowNum;
	this->scvLabel = "Source Code Viewer##" + windowNum;
	
	this->openFilePath = new char[sizeof(char) * (DEBUGGER_MAX_PATH * 2)];
	for (int i = 0; i < DEBUGGER_MAX_PATH; ++i)
	{
		this->openFilePath[i] = '\0';
	}

}

void SourceCodeViewer::setPersistantVars(Persistant_Vars* vars)
{
	this->vars = vars;
}

void SourceCodeViewer::reinit()
{
	delete [] this->openFilePath;
	this->fileContentsVector.clear();
	this->characterVector.clear();
	this->checkboxCheckedVector.clear();

	//Reallocate the char* for openfilePath on the heap.

	this->openFilePath = new char[sizeof(char) * (DEBUGGER_MAX_PATH * 2)];
	for (int i = 0; i < DEBUGGER_MAX_PATH; ++i)
	{
		this->openFilePath[i] = '\0';
	}

	this->pathSize = 0;
}

void SourceCodeViewer::reinitSaveMethod()
{
	delete[] this->openFilePath;

	//Reallocate the char* for openfilePath on the heap.

	this->openFilePath = new char[sizeof(char) * (DEBUGGER_MAX_PATH * 2)];
	for (int i = 0; i < DEBUGGER_MAX_PATH; ++i)
	{
		this->openFilePath[i] = '\0';
	}

	this->pathSize = 0;
}

void SourceCodeViewer::displayLoop()
{
	this->drawCodeViewerWindow();
}

void SourceCodeViewer::drawCodeViewerWindow()
{
	ImGuiWindowFlags windowFlags = 0;
	windowFlags |= ImGuiWindowFlags_MenuBar;
	windowFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
	windowFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
	

	if (ImGui::Begin(this->scvLabel.c_str(), 0, windowFlags))
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


					string label = "##String";
					label.push_back(i);
					label += "Window";
					label.push_back(this->windowNum);
					
					string checkboxLabel = "##Checkbox";
					checkboxLabel.push_back(i);
					checkboxLabel += "Window";
					checkboxLabel.push_back(this->windowNum);
					
					bool isChecked = this->checkboxCheckedVector.at(i);
					ImGui::Checkbox(checkboxLabel.c_str(), &isChecked);
					this->checkboxCheckedVector[i] = isChecked;
					ImGui::SameLine();
					ImGui::InputText(label.c_str(), currentString, (size_t)(MAX_BUFFER_SIZE), ImGuiInputTextFlags_CtrlEnterForNewLine);
				
				
					//Now, we have to write the info from buffer back into the string.
					this->fileContentsVector.at(i).clear();
					for (int j = 0; j < MAX_BUFFER_SIZE; ++j)
					{
						if (currentString[j] != NULL)
						{
							this->fileContentsVector.at(i).push_back(currentString[j]);
						}
					}
					delete[] currentString;
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

					this->openFile();
				}
				else if (ImGui::MenuItem("Open File in New Window"))
				{
					this->vars->srcCodeViewWindow.push_back(new SourceCodeViewer(this->windowNum));
					vars->srcCodeViewWindow.at(0)->setPersistantVars(vars);
				}
				else if (ImGui::MenuItem("Save"))
				{
					this->saveFile();
				}
				else if (ImGui::MenuItem("Close"))
				{
					this->reinit();
					this->fileOpenFlag = false;
				}

				

				ImGui::EndMenu();

			}
			ImGui::EndMenuBar();
		}

		ImGui::End();
	}
}




	







