#include <cmath>

#include "draw_direct2d1.h"

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
			return 0;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		this->OnPaint();
		return 0;
	case WM_SIZE:
		this->OnSize();
		return 0;
	default:
		return ::DefWindowProcW(this->hWnd, msg, wParam, lParam);
	}
}

bool MainWindow::CreateResouces(void)
{
	HRESULT result = ::D2D1CreateFactory(::D2D1_FACTORY_TYPE_SINGLE_THREADED, &this->D2Factory);
	return SUCCEEDED(result) ? true : false;
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
				{
					// Compute layout to draw.
					// Note that this is not done every time WM_PAINT is triggered.
					// Instead it is done only the render target is being created.
					this->CalcualteLayout();
					return true;
				}					
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

void MainWindow::OnSize(void)
{
	if (this->RenderTarget != nullptr)
	{
		::RECT rect;
		if (::GetClientRect(this->hWnd, &rect))
		{
			::D2D1_SIZE_U sz = D2D1::SizeU(rect.right, rect.bottom);
			this->RenderTarget->Resize(sz);
			this->CalcualteLayout();

			// InvalidateRect() triggers WM_PAINT.
			::InvalidateRect(this->hWnd, nullptr, FALSE);
		}
		// If any operation failed, then just silently finish the function.
	}
	// If render target is not already created, then just silently finish the function.
}

// This function only compute the layout without actually drawing it.
// Actual drawing is done by OnPaint().
void MainWindow::CalcualteLayout(void)
{
	if (this->RenderTarget != nullptr)
	{
		::D2D1_SIZE_F sz = this->RenderTarget->GetSize();
		const float x = sz.width / 2;
		const float y = sz.height / 2;
		const float radius = std::fmin(x, y);
		this->Ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
	}
	// If render target is not already created, then just silently finish the function.
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