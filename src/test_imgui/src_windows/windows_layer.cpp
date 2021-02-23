#include "os_layer.h"
#include "debugger.h"
#include <Windows.h>
#include <shobjidl_core.h>



HRESULT BasicFileOpen(char* returned_file_path);

int OSLayer::open_file(char* returned_file_path) {
	HRESULT result = BasicFileOpen(returned_file_path);
	return 0;
}

// This is the start of some kind of memory system. It's not that good. It's just here
// so we can start using it. 
void* OSLayer::allocate_memory(unsigned long long size_bytes) {
	HANDLE heap = GetProcessHeap();
	void* memory = HeapAlloc(heap, NULL, size_bytes);
	return memory;
}

bool OSLayer::free_memory(void* memory) {
	HANDLE heap = GetProcessHeap();
	return HeapFree(heap, NULL, memory);
}

HRESULT BasicFileOpen(char* returned_file_path)
{
	// CoCreate the File Open Dialog object.
	IFileDialog* pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		DWORD dwFlags;

		hr = pfd->GetOptions(&dwFlags);
		if (SUCCEEDED(hr))
		{
			// Show the dialog
			hr = pfd->Show(NULL);
			if (SUCCEEDED(hr))
			{
				// Obtain the result once the user clicks 
				// the 'Open' button.
				// The result is an IShellItem object.
				IShellItem* psiResult;
				hr = pfd->GetResult(&psiResult);
				if (SUCCEEDED(hr))
				{
					// We are just going to print out the 
					// name of the file for sample sake.
					PWSTR pszFilePath = NULL;
					hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Because the pszFilePath is a wide string (two bytes per character)
					// and our file_path is currently only 1 byte per character,
					// we're going to manually copy it over.
					int i = 0;
					while (i < DEBUGGER_MAX_PATH && pszFilePath[i] != 0) {
						// This will do the 2-byte to 1-byte conversion for us
						returned_file_path[i] = (char)pszFilePath[i];
						i++;
					}


					/*
					if (SUCCEEDED(hr))
					{
						TaskDialog(NULL,
							NULL,
							L"CommonFileDialogApp",
							pszFilePath,
							NULL,
							TDCBF_OK_BUTTON,
							TD_INFORMATION_ICON,
							NULL);
						CoTaskMemFree(pszFilePath);
					}
					*/
					psiResult->Release();
				}
			}
		}
		pfd->Release();
	}
	return hr;
}


HRESULT OnFileOk(IFileDialog* pfd) {
	return S_OK; // This accepts the file. S_FALSE refuses a file and keeps the dialog open
}