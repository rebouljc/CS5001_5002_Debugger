#include "debugger.h"
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <fileapi.h> // Reading filename from a file handle
#include <stdio.h> // temporary include

struct data_dir {
    unsigned long virtual_address;
    unsigned long size;
};

struct COFF {
    unsigned short machine;
    unsigned short number_of_sections;
    unsigned int time_date_stamp;
    unsigned int pointer_to_symbol_table;
    unsigned int number_of_symbols;
    unsigned short size_of_optional_header;
    unsigned short characteristics;
};

struct PEOP {
    unsigned short magic;
    unsigned char major_linker_version;
    unsigned char minor_linker_version;
    unsigned int size_of_code;
    unsigned int size_of_initialized_data;
    unsigned int size_of_uninitialized_data;
    unsigned int address_of_entry_point;
    unsigned int base_of_code;
};

struct DEBUG_DIR {
    unsigned int characteristics;
    unsigned int time_date_stamp;
    unsigned short major_version;
    unsigned short minor_version;
    unsigned int type;
    unsigned int size_of_data;
    unsigned int address_of_raw_data;
    unsigned int pointer_to_raw_data;
};

struct CODEVIEW {
    unsigned char signature[4];
	unsigned long guid1;
	unsigned short guid2;
	unsigned short guid3;
	unsigned char guid4[8];
    unsigned int age;
};

struct SECTION {
    char name[8];
    unsigned int virtual_size;
    unsigned int virtual_address;
    unsigned int size_of_raw_data;
    unsigned int pointer_to_raw_data;
    unsigned int pointer_to_relocations;
    unsigned int pointer_to_line_numbers;
    unsigned short number_of_relocations;
    unsigned short number_of_line_numbers;
    unsigned int characteristics;
};

void EnterDebugLoop(const LPDEBUG_EVENT debug_event, Debugger::DebuggerData* data);
void get_pdb_file_path(char* exe_path, char* output_path);

int Debugger::get_number_registers() {
	return 10;
}

int Debugger::debug_init(Debugger::DebuggerData* data) {

	// We should probably make a memset function
	for (int i = 0; i < DEBUGGER_MAX_PATH; i++) {
		data->exe_path[i] = 0;
		data->pdb_path[i] = 0;
	}

	HANDLE handle;
	// not sure what this size is for...
	unsigned long size = sizeof(unsigned long) * 100;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &handle)) {
		LUID restore_priv;
		TOKEN_PRIVILEGES new_priv, orig_priv;

		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &restore_priv)) {
			new_priv.PrivilegeCount = 1;
			new_priv.Privileges[0].Luid = restore_priv;
			new_priv.Privileges[0].Attributes = true;

			if (AdjustTokenPrivileges(handle, false, &new_priv, sizeof(orig_priv), &orig_priv, &size)) {
				printf("got debugging privilages\n");
			}
		}
	}
	
	return 0;
}

int Debugger::debug_loop(Debugger::DebuggerData* data) {
	DEBUG_EVENT debug_event = { 0 };
	EnterDebugLoop(&debug_event, data);
	return 0;
}

int Debugger::start_and_debug_exe(Debugger::DebuggerData* data){
	// We might need some memory from the platform layer for
	// general allocation for the operating system specific
	// debugger implementation. 
	// For example, it might be important for us to keep 
	// a handle on STARTUPINFO si and PROCESS_INFORMATION pi
	get_pdb_file_path(data->exe_path, data->pdb_path);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// UTF-8 problems :( there's definately a better way to handle this
	WCHAR path[DEBUGGER_MAX_PATH];
	for (int i = 0; i < DEBUGGER_MAX_PATH; i++){
		path[i] = data->exe_path[i];
	}

	// Just start the process for now
	if (!CreateProcess(NULL,
		path,
		NULL,
		NULL,
		FALSE,
		DEBUG_PROCESS,
		NULL,
		NULL, &si, &pi)) {
		printf("CreateProcess failed (%d).\n", GetLastError());
		return 0;
	}
	else {
		data->running = false;
		data->debugging = false;
	}

	data->running = true;
	data->debugging = true;
	data->pid = pi.dwProcessId;
	data->tid = pi.dwThreadId;

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return pi.dwProcessId;
}

unsigned long Debugger::list_of_processes(Debugger::Process* out_processes, unsigned long max) {
	DWORD aProcesses[1024], cbNeeded, cProcesses;

	
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
		return 0;
	}

	cProcesses = cbNeeded / sizeof(DWORD);


	TCHAR szProcessName[MAX_PATH] = TEXT("<unkown>");
	HANDLE hProcess;
	const char unkown_text[4] = {'.', '.', '.', 0};
	for (DWORD i = 0; i < cProcesses && i < max; i++) {
		out_processes[i].pid = (unsigned long)aProcesses[i];
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

		for (int x = 0; x < 4; x++) {
			out_processes[i].short_name[x] = unkown_text[x];
		}

		if (hProcess != NULL) {
			HMODULE hMod;
			DWORD cbNeeded;

			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
				GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
				//memcpy(out_processes[i].short_name, szProcessName, sizeof(out_processes[i].short_name) / sizeof(char));
				if (szProcessName[0] != '\0') {
					for (int x = 0; x < 50; x++) {
						out_processes[i].short_name[x] = szProcessName[x];
					}
					out_processes[i].short_name[49] = 0;
				}
		}
			//_tprintf(TEXT("%s (PID: %u)\n"), szProcessName, aProcesses[i]);
		    CloseHandle(hProcess);
		}
	}

	return (unsigned long)cProcesses;
}

// This is the windows way of checking for debug events
// from the target process
void EnterDebugLoop(const LPDEBUG_EVENT debug_event, Debugger::DebuggerData *data) {
	WaitForDebugEvent(debug_event, 0);  // Check for an event and then continue. Non-blocking

	// This might allow us to debug multiple processes at the same time
	// not sure how useful that's going to be.
	// But it's here...
	if (debug_event->dwDebugEventCode  == 0 || debug_event->dwProcessId != data->pid) {
		return;
	}
	char dll_name[DEBUGGER_MAX_PATH] = { 0 };

	switch (debug_event->dwDebugEventCode) {
	case 0: break; // no debug event occurred 

	case CREATE_PROCESS_DEBUG_EVENT:
		printf("create process event\n");
		CloseHandle(debug_event->u.CreateProcessInfo.hFile);
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	case CREATE_THREAD_DEBUG_EVENT:
		printf("create thread event\n");
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	case EXCEPTION_DEBUG_EVENT:
		printf("exception event\n");
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	case EXIT_PROCESS_DEBUG_EVENT:
		printf("exit process event\n");
		data->running = false;
		data->debugging = false;
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	case EXIT_THREAD_DEBUG_EVENT:
		printf("exit thread event\n");
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	case LOAD_DLL_DEBUG_EVENT:
		GetFinalPathNameByHandleA(debug_event->u.LoadDll.hFile, dll_name, DEBUGGER_MAX_PATH, FILE_NAME_NORMALIZED);
		printf("load dll event: %s\n", dll_name);
		CloseHandle(debug_event->u.LoadDll.hFile);
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	case UNLOAD_DLL_DEBUG_EVENT:
		printf("unload dll event\n");
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	case OUTPUT_DEBUG_STRING_EVENT:
		printf("output string event\n");
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	case RIP_EVENT:
		printf("system debugging error event\n");
		ContinueDebugEvent(debug_event->dwProcessId, debug_event->dwThreadId, DBG_CONTINUE);
		break;
	}

}

void get_pdb_file_path(char* exe_path, char* output_path) {

	WCHAR exe_wide[DEBUGGER_MAX_PATH];
	for (int i = 0; i < DEBUGGER_MAX_PATH; i++) {
		exe_wide[i] = (WCHAR)exe_path[i];
	}

    HANDLE file_handle;
    file_handle = CreateFile(
        exe_wide,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL 
    );    


    char buffer[2048];
    unsigned long bytes_read = 0;

    bool success = ReadFile(file_handle, buffer, 2048, &bytes_read, NULL);
    printf("bytes read: %i\n", bytes_read);

    unsigned int pe_offset = ((*(unsigned short*)(buffer + 0x3C)) & 0xFFFF) + 4;
    unsigned int pe_op_offset = pe_offset + 20;

    COFF coff_data = *(COFF*)(buffer + pe_offset);
    PEOP op_standard_entries = *(PEOP*)(buffer+pe_op_offset);

    printf("pe_offset: %i\n", pe_offset);
    //unsigned int pe_offset = 5;
    // + 4 to account for PE\0\0

    unsigned int section_start = pe_op_offset + coff_data.size_of_optional_header;

	printf("Debug Data stripped flag: %i\n", coff_data.characteristics & 0x0200);
    printf("Optional header size: %i\n", coff_data.size_of_optional_header);
    printf("Number of sections: %i\n", coff_data.number_of_sections);
    /*
    printf("Characteristics hex : %#02x\n", characteristics);
    */
    printf("PEOP magic number: %#08x\n", op_standard_entries.magic);
    unsigned int rva_offset = 0;
    unsigned int data_dirs_offset;
    unsigned int debug_data_dir_offset;

    if (op_standard_entries.magic == 0x10b) {
		printf("PE32 format\n");
		rva_offset = pe_op_offset + 92;
		data_dirs_offset = pe_op_offset + 96;
        debug_data_dir_offset = pe_op_offset + 144;
    }
    else if (op_standard_entries.magic == 0x20b) {
		printf("PE32+ format\n");
		rva_offset = pe_op_offset + 108;
		data_dirs_offset = pe_op_offset + 112;
        debug_data_dir_offset = pe_op_offset + 160;
    }
    else if (op_standard_entries.magic == 0x107) {
		printf("ROM format\n");
    }

    unsigned int num_rva_sizes = *(unsigned int*)(buffer + rva_offset) & 0xFFFFFFFF;
    printf("NumberOfRvaAndSizes: %i\n", num_rva_sizes);

    data_dir debug_dir = *(data_dir*)(buffer + debug_data_dir_offset);
    printf("\tDebug data VirtualAddress: %#08X Size: %#08X", debug_dir.virtual_address, debug_dir.size);
    unsigned long debug_dir_file_offset;

    printf("Sections: \n");

    int current_section = 0;
    char name[9] = { 0 };
    for (int i = 0; i < coff_data.number_of_sections; i++) {
        SECTION current_section = *(SECTION*)(buffer + section_start + i * sizeof(SECTION));

        for (int x = 0; x < 8; x++) {
            name[x] = current_section.name[x];
        }
        

        //if (current_section.characteristics == (IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA)) {
            printf("\t%s\t%#08X\t%i\t%#08X\n", name, current_section.pointer_to_raw_data, current_section.characteristics & IMAGE_SCN_CNT_CODE, current_section.pointer_to_raw_data+current_section.size_of_raw_data);
            printf("\t\tCharacteristics: %#08X\n", current_section.characteristics);
        //}
        if ((debug_dir.virtual_address < (current_section.virtual_address + current_section.virtual_size))
            && (debug_dir.virtual_address > current_section.virtual_address)) {
            debug_dir_file_offset = debug_dir.virtual_address- ((current_section.virtual_address) - (current_section.pointer_to_raw_data));
            printf("\t\t\tDebug dir file offset: %#08X\n", debug_dir_file_offset);
        }
    }

    unsigned long current_start = 0;
    unsigned long size = 2048;

    while (debug_dir_file_offset >= (current_start + size)) {
		success = ReadFile(file_handle, buffer, 2048, &bytes_read, NULL);
        if (!success) {
            printf("Reached end of file?\n");
            return;
        }
        current_start += bytes_read;
    }

    DEBUG_DIR another_debug_dir = *(DEBUG_DIR*)(buffer+(debug_dir_file_offset-current_start));
    
    if (another_debug_dir.type == 2) {
        printf("Debug data is Visual C++ debug information\n");
        printf("Major version: %i Minor Version %i\n", another_debug_dir.major_version, another_debug_dir.minor_version);
    }


    while (another_debug_dir.pointer_to_raw_data >= (current_start + size)) {
		success = ReadFile(file_handle, buffer, 2048, &bytes_read, NULL);
        if (!success) {
            printf("Reached end of file?\n");
            return;
        }
        current_start += bytes_read;
    }

    unsigned long code_view_offset = + another_debug_dir.pointer_to_raw_data - current_start;
    CODEVIEW codeview = *(CODEVIEW*)(buffer +code_view_offset);
    printf("\tcodeview header:\n");
    printf("\t\tSignature: ");
    for (int i = 0; i < 4; i++) {
        printf("%c", codeview.signature[i]);
    }
    printf("\n\t\tGUID: %#04X-%#02X-%#02X-%#08X\n", codeview.guid1, codeview.guid2, codeview.guid3, codeview.guid4);
    printf("\t\tage: %i\n",codeview.age);

    printf("\t\tFile Path: %s\n", (buffer+code_view_offset+sizeof(codeview)));

	char* pdb_file_path = (buffer + code_view_offset + sizeof(codeview));

	int i = 0;
	while (i < DEBUGGER_MAX_PATH && pdb_file_path[i] != 0) {
		output_path[i] = pdb_file_path[i];
		i++;
	}

    CloseHandle(file_handle);

}
