// This project does exactly what OpenDialog1 does using __uuidof() and IID_PPV_ARGS macto.
// COM smart pointers are not supported by Visual Studio Express editions as it is a part of ATL.

#include <Windows.h>
#include <ShObjIdl.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, wchar_t *cmdLine, int cmdShow)
{
	// Initialize COM library.
	HRESULT result = ::CoInitializeEx(nullptr, ::COINIT_APARTMENTTHREADED | ::COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(result))
	{
		::IFileOpenDialog *file_open_dialog(nullptr);		
		// Create FileOpenDialog object.
		result = ::CoCreateInstance(__uuidof(::FileOpenDialog), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&file_open_dialog));
		if (SUCCEEDED(result))
		{
			// Show the dialog.
			result = file_open_dialog->Show(NULL);
			if (SUCCEEDED(result))
			{
				// Get the file name from the dialog.
				::IShellItem *shell_item(nullptr);
				result = file_open_dialog->GetResult(&shell_item);
				if (SUCCEEDED(result))
				{
					wchar_t *file_path(nullptr);
					result = shell_item->GetDisplayName(::SIGDN_FILESYSPATH, &file_path);
					if (SUCCEEDED(result))
					{
						::MessageBoxW(NULL, file_path, L"Selected File Path", MB_OK);
						::CoTaskMemFree(file_path);
					}

					shell_item->Release();
				}
			}
			else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {}	// Do nothing if canceled.
			else
				::MessageBoxW(NULL, L"Failed to get the path of the selected file.", L"Error", MB_OK);

			file_open_dialog->Release();
		}

		::CoUninitialize();
	}

	return 0;
}