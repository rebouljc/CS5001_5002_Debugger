#pragma once


#include "imgui.h"
#include "main_ui.h"


namespace Debugger
{
	class SourceCodeViewer
	{
	public:
		SourceCodeViewer();
		SourceCodeViewer(struct Persistant_Vars* var);
		SourceCodeViewer* getClassPtr();
	private:
		void drawCodeViewerWindow(struct Persistant_Vars* var);
	};
}
