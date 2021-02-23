#pragma once
#pragma once
#include <shobjidl_core.h>
#include "debugger.h"

namespace OSPlatformUI {
	int open_file(char* returned_file_path, int &pathSize);
	int save_file(char* returned_file_path, int& pathSize);
}
HRESULT OnFileOk(IFileDialog* pfd);
// This accepts the file. S_FALSE refuses a file and keeps the dialog open


HRESULT BasicFileOpen(char* returned_file_path, int &pathSize);
HRESULT BasicFileSave(char* returned_file_path, int &pathSize);