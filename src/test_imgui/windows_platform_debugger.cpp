#include "debugger.h"
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <stdio.h> // temporary include

int Debugger::get_number_registers() {
	return 10;
}

unsigned long Debugger::list_of_pids(unsigned long** out_pids, unsigned long max) {
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
	
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
		return 0;
	}

	cProcesses = cbNeeded / sizeof(DWORD);


	TCHAR szProcessName[MAX_PATH] = TEXT("<unkown>");
	HANDLE hProcess;
	for (DWORD i = 0; i < cProcesses; i++) {
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

		if (hProcess != NULL) {
			HMODULE hMod;
			DWORD cbNeeded;

			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
				GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
			}
			_tprintf(TEXT("%s (PID: %u)\n"), szProcessName, aProcesses[i]);
		}
		CloseHandle(hProcess);
	}

	return (unsigned long)cProcesses;
}