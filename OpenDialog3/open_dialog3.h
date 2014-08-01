#if !defined(OPEN_DIALOG3_H)
#define OPEN_DIALOG3_H

// This project demonstrates file open dialog feature shown in OpenDialog1 and OpenDialog2
// with a menu bar.
// Window procedure is defined using a class.

#include <Windows.h>

#include "resource.h"

template <class DERIVED_TYPE>
class BaseWindow
{
public:
	static LRESULT CALLBACK WindowProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam);
	BaseWindow(void) = default;
	bool Create(wchar_t *wndName, unsigned long style, unsigned long extraStyle = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int width = CW_USEDEFAULT, int height = CW_USEDEFAULT, HWND hWndParent = NULL, HMENU hMenu = NULL);
	HWND Window() const { return this->hWnd; }

protected:
	virtual wchar_t *ClassName() const = 0;
	virtual LRESULT HandleMessage(unsigned int msf, WPARAM wParam, LPARAM lParam) = 0;
	HWND hWnd = nullptr;	// Default value for HANDLE data type doesn't really make sense as it presents somewhat like a reference.
};

template <class DERIVED_TYPE>
LRESULT CALLBACK BaseWindow<DERIVED_TYPE>::WindowProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	DERIVED_TYPE *self(nullptr);
	if (msg == WM_NCCREATE)
	{
		::CREATESTRUCTW *create = reinterpret_cast<CREATESTRUCTW *>(lParam);
		self = reinterpret_cast<DERIVED_TYPE *>(create->lpCreateParams);
		::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
		self->hWnd = hWnd;
	}
	else
		self = reinterpret_cast<DERIVED_TYPE *>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	if (self != nullptr)
		return self->HandleMessage(msg, wParam, lParam);
	else
		return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

template <class DERIVED_TYPE>
bool BaseWindow<DERIVED_TYPE>::Create(wchar_t *wndName, unsigned long style, unsigned long extraStyle, int x, int y, int width, int height, HWND hWndParent, HMENU hMenu)
{
	// Register window class.
	::WNDCLASSW wc = {};	// NOTE: '= {}' is mandatory.
	wc.lpfnWndProc = DERIVED_TYPE::WindowProc;
	wc.hInstance = ::GetModuleHandleW(nullptr);
	wc.lpszClassName = this->ClassName();
	wc.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU1);
	::RegisterClassW(&wc);

	// Create the window.
	this->hWnd = ::CreateWindowExW(extraStyle, this->ClassName(), wndName, style, x, y, width, height, hWndParent, hMenu, wc.hInstance, this);
	return this->hWnd == NULL ? false : true;

	// If lpszMenuName was set for WNDCLASSW before RegisterClassW() and the same menu is being used,
	// then the corresponding argument can be set as NULL for CreateWindowExW().
}

class MainWindow : public BaseWindow < MainWindow >
{
public:
	wchar_t *ClassName() const { return L"Sample Window Class with File Open Dialog"; }
	LRESULT HandleMessage(unsigned int msg, WPARAM wParam, LPARAM lParam);

protected:
	void OnOpen(void);
};

#endif
