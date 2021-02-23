#pragma once

#include <string>
#include <vector>

#include "imgui.h"
#include "main_ui.h"

using namespace std;
class SourceCodeViewer
{
	public:
		SourceCodeViewer();
		
		void setPersistantVars(struct Persistant_Vars *vars);
		~SourceCodeViewer();
		
		void displayLoop();
		
		
	private:
		SourceCodeViewer(int currentWindowNum);
		void openFile();
		void saveFile();
		void drawCodeViewerWindow();
		void reinit();
		void reinitSaveMethod();
		char* openFilePath;
		int pathSize;
		vector<vector<char>> fileContentsVector;
		vector<char>characterVector;
		bool fileOpenFlag = false;
		int clicksonOpenButton = 0;
		Persistant_Vars* vars;
		int windowNum = 0;
		string scvLabel;
		vector<bool> checkboxCheckedVector;
		
};

