#include "debugger.h"
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <fileapi.h> // Reading filename from a file handle
#include <stdio.h> // temporary include

void EnterDebugLoop(const LPDEBUG_EVENT debug_event, Debugger::DebuggerData* data);

int Debugger::get_number_registers() {
	return 10;
}

int Debugger::debug_init(Debugger::DebuggerData* data) {

	// We should probably make a memset function
	for (int i = 0; i < DEBUGGER_MAX_PATH; i++) {
		data->exe_path[i] = 0;
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

