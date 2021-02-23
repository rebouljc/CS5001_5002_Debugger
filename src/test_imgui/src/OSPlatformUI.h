#pragma once
#pragma once
#include "debugger.h"

namespace OSPlatformUI {
	int open_file(char* returned_file_path, int &pathSize);
	int save_file(char* returned_file_path, int &pathSize);
}
