#pragma once


#include "imgui.h"
#include<shobjidl_core.h>
#include <string>



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
		
		
};

