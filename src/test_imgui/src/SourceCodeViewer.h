#pragma once


#include "imgui.h"




class SourceCodeViewer
{
	public:
		SourceCodeViewer();
		void displayLoop();
		
	private:
		
		void drawCodeViewerWindow();
		bool openSourceFile();
};

