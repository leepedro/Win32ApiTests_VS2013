#include "simple2.h"

LRESULT MainWindow::HandleMessage(unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
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


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, wchar_t *cmdLine, int cmdShow)
{
	// Register and create a window.
	MainWindow win;
	if (!win.Create(L"Learn to Program Windows", WS_OVERLAPPEDWINDOW))
		return 0;

	// Show the window.
	::ShowWindow(win.Window(), cmdShow);

	// Run a message loop.
	MSG msg;
	while (::GetMessageW(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}

	return 0;
}
