#include <cmath>

#include <windowsx.h>

#include "draw_direct2d2.h"

void DpiScale::Initialize(::ID2D1Factory *d2Factory)
{
	float dpiX, dpiY;
	d2Factory->GetDesktopDpi(&dpiX, &dpiY);
	DpiScale::ScaleX = dpiX / 96.0f;
	DpiScale::ScaleY = dpiY / 96.0f;
}

MainWindow::~MainWindow(void)
{
	this->ReleaseDeviceResources();
	this->ReleaseResources();
}

LRESULT MainWindow::HandleMessage(unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		if (!this->CreateResouces())
			return -1;	// Fail CreateWindowExW() if creating resources failed.
		else
		{
			DpiScale::Initialize(this->D2Factory);
			return 0;
		}
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		this->OnPaint();
		return 0;
	case WM_LBUTTONDOWN:
		this->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
		this->OnLButtonUp();
		return 0;
	case WM_MOUSEMOVE:
		this->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), static_cast<unsigned int>(wParam));
		return 0;
	default:
		return ::DefWindowProcW(this->hWnd, msg, wParam, lParam);
	}
}

bool MainWindow::CreateDeviceResources(void)
{
	if (this->RenderTarget != nullptr)
		return true;	// If render target is already created, then just return true.
	else
	{
		// Create render target for the given client area.
		::RECT rect;
		if (::GetClientRect(this->hWnd, &rect))
		{
			::D2D1_SIZE_U sz = D2D1::SizeU(rect.right, rect.bottom);
			HRESULT result = this->D2Factory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(this->hWnd, sz),
				&this->RenderTarget);
			if (SUCCEEDED(result))
			{
				// Create brush.
				const ::D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
				result = this->RenderTarget->CreateSolidColorBrush(color, &this->Brush);
				if (SUCCEEDED(result))
					return true;
			}
		}
		return false;
	}
}

void MainWindow::ReleaseResources(void)
{
	SafeRelease(this->D2Factory);
}

void MainWindow::ReleaseDeviceResources(void)
{
	SafeRelease(this->RenderTarget);
	SafeRelease(this->Brush);
}

void MainWindow::OnLButtonDown(int x, int y)
{
	::SetCapture(this->hWnd);
	this->PtMouse = DpiScale::PixelsToDips(x, y);
	this->Ellipse.point = this->PtMouse;
	this->Ellipse.radiusX = 1.0f;
	this->Ellipse.radiusY = 1.0f;

	// Triggers WM_PAINT for the entire client area.
	::InvalidateRect(this->hWnd, nullptr, FALSE);
}

void MainWindow::OnLButtonUp(void)
{
	::ReleaseCapture();
}

void MainWindow::OnMouseMove(int x, int y, unsigned int flags)
{
	if (flags & MK_LBUTTON)
	{
		const ::D2D1_POINT_2F dips = DpiScale::PixelsToDips(x, y);
		const float radius_x = (dips.x - this->PtMouse.x) / 2;
		const float radius_y = (dips.y - this->PtMouse.y) / 2;
		const ::D2D1_POINT_2F ptCenter = D2D1::Point2F(this->PtMouse.x + radius_x, this->PtMouse.y + radius_y);
		this->Ellipse = D2D1::Ellipse(ptCenter, radius_x, radius_y);

		// Triggers WM_PAINT for the entire client area.
		::InvalidateRect(this->hWnd, nullptr, FALSE);
	}
}

void MainWindow::OnPaint(void)
{
	if (this->CreateDeviceResources())
	{
		::PAINTSTRUCT ps;
		::BeginPaint(this->hWnd, &ps);
		this->RenderTarget->BeginDraw();
		this->RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
		this->RenderTarget->FillEllipse(this->Ellipse, this->Brush);
		HRESULT result = this->RenderTarget->EndDraw();
		if (FAILED(result) || result == D2DERR_RECREATE_TARGET)
			this->ReleaseDeviceResources();
		::EndPaint(this->hWnd, &ps);	// return BOOL?
	}
	// If any operation failed, then just silently finish the function.
}

bool MainWindow::CreateResouces(void)
{
	HRESULT result = ::D2D1CreateFactory(::D2D1_FACTORY_TYPE_SINGLE_THREADED, &this->D2Factory);
	return SUCCEEDED(result) ? true : false;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, wchar_t *cmdLine, int cmdShow)
{
	// Initialize COM library.
	if (SUCCEEDED(::CoInitializeEx(nullptr, ::COINIT_APARTMENTTHREADED | ::COINIT_DISABLE_OLE1DDE)))
	{
		// NOTE: Wrap MainWindow object with {}, so all member sources are already released
		// before calling CoUninitialize().
		{
			MainWindow win;
			if (!win.Create(L"Learn to Program Windows API with Direct2D", WS_OVERLAPPEDWINDOW))
				return 0;
			::ShowWindow(win.Window(), cmdShow);
			RunMessageLoop();
		}
		::CoUninitialize();
	}

	return 0;
}