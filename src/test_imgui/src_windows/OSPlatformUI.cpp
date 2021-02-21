#include "OSPlatformUI.h"

#include "main_ui.h"

int OSPlatformUI::open_file(char* returned_file_path, HWND &currentHandle, int &pathSize) {
	HRESULT result = BasicFileOpen(returned_file_path, currentHandle, pathSize);
	return 0;
}

int OSPlatformUI::save_file(char* returned_file_path, HWND& currentHandle, int& pathSize) {
	HRESULT result = BasicFileSave(returned_file_path, currentHandle, pathSize);
	return 0;
}

HRESULT BasicFileOpen(char* returned_file_path, HWND &currentHandle, int &pathSize)
{
	// CoCreate the File Open Dialog object.
	IFileDialog* pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
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
			hr = pfd->Show(currentHandle);
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


	return hr;
}

HRESULT BasicFileSave(char* returned_file_path, HWND& currentHandle, int& pathSize)
{
	// CoCreate the File Open Dialog object.
	IFileDialog* pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog,
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
			hr = pfd->Show(currentHandle);
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


	return hr;
}


HRESULT OnFileOk(IFileDialog* pfd) {
	return S_OK; // This accepts the file. S_FALSE refuses a file and keeps the dialog open
}