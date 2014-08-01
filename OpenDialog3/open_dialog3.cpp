#include <ShObjIdl.h>

#include "open_dialog3.h"

LRESULT MainWindow::HandleMessage(unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_OPEN:
			this->OnOpen();
			return 0;
		case ID_FILE_EXIT:
			::PostMessageW(this->hWnd, WM_CLOSE, 0, 0);
			return 0;
		}
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	{
		::PAINTSTRUCT ps;
		HDC hDc = ::BeginPaint(this->hWnd, &ps);
		::FillRect(hDc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
		::EndPaint(this->hWnd, &ps);
	}
		return 0;
	case WM_CLOSE:
		if (::MessageBoxW(hWnd, L"Really Quit?", L"My Application", MB_OKCANCEL) == IDOK)
			::DestroyWindow(this->hWnd);
		return 0;
	default:
		return ::DefWindowProcW(this->hWnd, msg, wParam, lParam);
	}
}

void MainWindow::OnOpen(void)
{
	::IFileOpenDialog *file_open_dialog(nullptr);
	// Create FileOpenDialog object.
	HRESULT result = ::CoCreateInstance(__uuidof(::FileOpenDialog), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&file_open_dialog));
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
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, wchar_t *cmdLine, int cmdShow)
{
	// Initialize COM library.
	if (SUCCEEDED(::CoInitializeEx(nullptr, ::COINIT_APARTMENTTHREADED | ::COINIT_DISABLE_OLE1DDE)))
	{
		// NOTE: Wrap MainWindow object with {}, so all member sources are already released
		// before calling CoUninitialize().
		{
			// Register and create a window.
			MainWindow win;
			if (!win.Create(L"Learn to Program Windows", WS_OVERLAPPEDWINDOW))
				return 0;

			// Show the window.
			::ShowWindow(win.Window(), cmdShow);

			// Run a message loop.
			BOOL result(0);
			MSG msg;
			while ((result = ::GetMessageW(&msg, NULL, 0, 0)) != 0)
			{
				if (result == -1)
					break;
				else
				{
					::TranslateMessage(&msg);
					::DispatchMessageW(&msg);
				}
			}
		}

		::CoUninitialize();
	}

	return 0;
}
