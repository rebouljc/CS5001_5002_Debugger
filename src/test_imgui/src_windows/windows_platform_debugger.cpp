#include "debugger.h"
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <stdio.h> // temporary include

int Debugger::get_number_registers() {
	return 10;
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