#pragma once
#pragma once
#include "debugger.h"

namespace OSPlatformUI {
	int open_file_dialog(char* returned_file_path, int &pathSize);
	int save_file_dialog(char* returned_file_path, int &pathSize);

	void* allocate_memory(unsigned long size_bytes);
	bool free_memory(void* memory);
}
