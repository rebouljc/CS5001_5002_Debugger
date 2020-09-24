#include<Windows.h>
#include<Psapi.h>
#include<iostream>
#include<stdint.h>  

// Here is some test code I wrote to play with attaching to a running process in windows.
// it is not complete by any means. If you want to try it out yourself, change the PID to a PID of a running process on your system
// attaching might fail, if it does, try a different PID

// Errors:
// if something fails, you can always check GetLastError() for windows system error codes. I've hit a couple so far (5, 50, and 87)
// there is a programatic way to get the name of the errors, but right now you can just check: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes

int main() {

	unsigned long pid_to_attach = 14456;

	unsigned long* identifiers = new unsigned long[100];
	unsigned long size = sizeof(unsigned long) * 100;
	unsigned long returned;

	HANDLE handle;
	
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &handle)) {
		printf("got process token\n");

		LUID restore_priv;
		TOKEN_PRIVILEGES new_priv, orig_priv;
		
		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &restore_priv)) {
			printf("got old priv\n");
			new_priv.PrivilegeCount = 1;
			new_priv.Privileges[0].Luid = restore_priv;
			new_priv.Privileges[0].Attributes = true;

			if (AdjustTokenPrivileges(handle, false, &new_priv, sizeof(orig_priv), &orig_priv, &size)) {
				printf("got the privilage!\n");

				// try to attach to some process.
				if (DebugActiveProcess(pid_to_attach)) {
					printf("attached!!!!!\n");
				}
				else {
					printf("could not attached: %i\n", GetLastError());
				}
			}
		}

		CloseHandle(handle);
	}

	return 0;
}