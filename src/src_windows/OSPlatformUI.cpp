#include "OSPlatformUI.h"
#include "main_ui.h"
#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>
#include <fileapi.h> // Reading filename from a file handle
#include <stdio.h> // temporary include
#include <ShObjIdl_core.h>


int OSPlatformUI::open_file(char* returned_file_path, int &pathSize) {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr)) {
		CoUninitialize(); // CoUninitialize needs to be called whether or not the initize succeeded
		return 0;
	}

	// CoCreate the File Open Dialog object.
	IFileDialog* pfd = NULL;
	hr = CoCreateInstance(CLSID_FileOpenDialog, 
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
					PWSTR pszFilePath = NULL;
					hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Because the pszFilePath is a wide string (two bytes per character)
					// and our file_path is currently only 1 byte per character,
					// we're going to manually copy it over.
					//
					
					//Now, just remember to free this within the calling class or you will have a potential memory leak (just a little one),
					//every time you call this method.
					int i = 0;
					while (i < DEBUGGER_MAX_PATH && pszFilePath != nullptr && pszFilePath[i] != 0) {
						// This will do the 2-byte to 1-byte conversion for us
						returned_file_path[i] = (char)pszFilePath[i];
						i++;
					}
					pathSize = strlen(returned_file_path);
					if (SUCCEEDED(hr)) {
						CoTaskMemFree(pszFilePath);
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
					}
					*/
					psiResult->Release();
				}
			}
		}
		pfd->Release();
	}

	CoUninitialize();
	return 0;
}

int OSPlatformUI::save_file(char* returned_file_path, int& pathSize) {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
	if (!SUCCEEDED(hr)) {
		CoUninitialize(); // CoUninitialize needs to be called whether or not the initize succeeded
		return 0;
	}

	// CoCreate the File Open Dialog object.
	IFileDialog* pfd = NULL;
	hr = CoCreateInstance(CLSID_FileSaveDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd)
	);
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
					//

					//Now, just remember to free this within the calling class or you will have a potential memory leak (just a little one),
					//every time you call this method.
					int i = 0;
					while (i < DEBUGGER_MAX_PATH && pszFilePath != nullptr && pszFilePath[i] != 0) {
						// This will do the 2-byte to 1-byte conversion for us
						returned_file_path[i] = (char)pszFilePath[i];
						i++;
					}
					pathSize = strlen(returned_file_path);

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

	CoUninitialize();
	return 0;
}


HRESULT OnFileOk(IFileDialog* pfd) {
	return S_OK; // This accepts the file. S_FALSE refuses a file and keeps the dialog open
}
