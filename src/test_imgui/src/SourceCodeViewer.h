#pragma once


#include "imgui.h"
#include<shobjidl_core.h>



class SourceCodeViewer
{
	public:
		SourceCodeViewer();
		void displayLoop();
		
		
	private:
		
		void drawCodeViewerWindow();
		virtual bool openSourceFile();
};

