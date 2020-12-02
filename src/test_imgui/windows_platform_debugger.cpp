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
	for (DWORD i = 0; i < cProcesses && i < max; i++) {
		out_processes[i].pid = (unsigned int)aProcesses[i];
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

		if (hProcess != NULL) {
			HMODULE hMod;
			DWORD cbNeeded;

			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
				GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
				//memcpy(out_processes[i].short_name, szProcessName, sizeof(out_processes[i].short_name) / sizeof(char));
				if (szProcessName[0] == '\0') {
					strcpy(out_processes[i].short_name, "<unkown>");
				}
				else {
					for (int x = 0; x < 50; x++) {
						out_processes[i].short_name[x] = szProcessName[x];
					}
				}
			}
			else {
				strcpy(out_processes[i].short_name, "<unkown>");
			}
			//_tprintf(TEXT("%s (PID: %u)\n"), szProcessName, aProcesses[i]);
		}
		CloseHandle(hProcess);
	}

	return (unsigned long)cProcesses;
}