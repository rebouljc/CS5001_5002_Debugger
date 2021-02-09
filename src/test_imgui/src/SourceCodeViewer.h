#pragma once


#include "imgui.h"
#include<shobjidl_core.h>
#include <string>
#include <vector>


using namespace std;
class SourceCodeViewer
{
	public:
		SourceCodeViewer();
		void setCurrentHandle(HWND& currentHandle);
		~SourceCodeViewer();
		
		void displayLoop();
		
		
	private:
		
		void drawCodeViewerWindow();
		HWND currentHandle;
		char* openFilePath;
		int pathSize;
		vector<vector<char>> fileContentsVector;
		vector<char>characterVector;
		bool fileOpenFlag = false;
		
		
		
		
};

