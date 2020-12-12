#include "debugger.h"
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <stdio.h> // temporary include

int Debugger::get_number_registers() {
	return 10;
}

int Debugger::start_and_debug_exe(Debugger::DebuggerData* data){
	// We might need some memory from the platform layer for
	// general allocation for the operating system specific
	// debugger implementation. 
	// For example, it might be important for us to keep 
	// a handle on STARTUPINFO si and PROCESS_INFORMATION pi
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
		0,
		NULL,
		NULL, &si, &pi)) {
		printf("CreateProcess failed (%d).\n", GetLastError());
		return 0;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

unsigned long Debugger::list_of_processes(Debugger::Process* out_processes, unsigned long max) {
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
	
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
		return 0;
	}

	cProcesses = cbNeeded / sizeof(DWORD);


	TCHAR szProcessName[MAX_PATH] = TEXT("<unkown>");
	HANDLE hProcess;
	char* unkown_text = "<unkown>";
	for (DWORD i = 0; i < cProcesses && i < max; i++) {
		out_processes[i].pid = (unsigned int)aProcesses[i];
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

		for (int x = 0; x < 50; x++) {
			out_processes[i].short_name[x] = szProcessName[x];
		}

		if (hProcess != NULL) {
			HMODULE hMod;
			DWORD cbNeeded;

			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
				GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
				//memcpy(out_processes[i].short_name, szProcessName, sizeof(out_processes[i].short_name) / sizeof(char));
				if (szProcessName[0] == '\0') {
					for (int x = 0; x < 8; x++) {
						out_processes[x].short_name[x] = unkown_text[x];
					}
				}
			}
			//_tprintf(TEXT("%s (PID: %u)\n"), szProcessName, aProcesses[i]);
		}
		CloseHandle(hProcess);
	}

	return (unsigned long)cProcesses;
}