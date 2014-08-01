// This project demonstrates a simple example of Windows native API.
// It opens up a window without a menu, and pops up a message box when asked to close.

#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, wchar_t *cmdLine, int cmdShow)
{
	// Register window class.
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	::WNDCLASSW wc = {};	// NOTE: '= {}' is mandatory.
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	::RegisterClassW(&wc);

	// Create the window.
	HWND hWnd = ::CreateWindowExW(0, CLASS_NAME, L"Learn to Program Windows", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, nullptr);
	if (hWnd == NULL)
		return 0;

	// Show the window.
	::ShowWindow(hWnd, cmdShow);

	// Run a message loop.
	MSG msg;
	while (::GetMessageW(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_PAINT:
	{
		::PAINTSTRUCT ps;
		HDC hDc = ::BeginPaint(hWnd, &ps);
		::FillRect(hDc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
		::EndPaint(hWnd, &ps);
	}
		return 0;
	case WM_CLOSE:
		if (::MessageBoxW(hWnd, L"Really Quit?", L"My Application", MB_OKCANCEL) == IDOK)
			::DestroyWindow(hWnd);
		return 0;
	default:
		return ::DefWindowProcW(hWnd, msg, wParam, lParam);
	}
}