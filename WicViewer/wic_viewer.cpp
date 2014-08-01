#include <ShObjIdl.h>

#include "wic_viewer.h"

MainWindow::~MainWindow(void)
{
	this->ReleaseDeviceResources();
	this->ReleaseResources();
}

LRESULT MainWindow::HandleMessage(unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_OPEN:
			if (this->CreateD2DBmpFromFile())
				::InvalidateRect(this->hWnd, nullptr, TRUE);
			break;
		case ID_FILE_EXIT:
			::PostMessageW(this->hWnd, WM_CLOSE, 0, 0);
		}
		return 0;
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
		this->OnSize(lParam);
		return 0;
	default:
		return ::DefWindowProcW(this->hWnd, msg, wParam, lParam);
	}
}

bool MainWindow::CreateD2DBmpFromFile(void)
{
	bool final_result(false);
	std::wstring path_src;
	if (this->LocateImagefile(path_src))
	{
		// Decode source image.
		::IWICBitmapDecoder *decoder(nullptr);
		HRESULT result = this->WicFactory->CreateDecoderFromFilename(path_src.c_str(), nullptr,
			GENERIC_READ, ::WICDecodeMetadataCacheOnDemand, &decoder);
		if (SUCCEEDED(result))
		{
			// Get the frame.
			::IWICBitmapFrameDecode *frame(nullptr);
			result = decoder->GetFrame(0, &frame);
			if (SUCCEEDED(result))
			{
				// Convert the source image frame to 24bit BGR and keep it.
				SafeRelease(this->BmpSrc);
				result = this->WicFactory->CreateFormatConverter(&this->BmpSrc);
				if (SUCCEEDED(result))
				{
					result = this->BmpSrc->Initialize(frame, ::GUID_WICPixelFormat32bppPBGRA,
						::WICBitmapDitherTypeNone, nullptr, 0.0, ::WICBitmapPaletteTypeCustom);
					if (SUCCEEDED(result))
					{
						// Create a bitmap for screen from the stored bitmap.
						result = this->CreateDeviceResources();
						if (SUCCEEDED(result))
						{
							SafeRelease(this->Bmp);
							result = this->RenderTarget->CreateBitmapFromWicBitmap(this->BmpSrc,
								nullptr, &this->Bmp);							
							if (FAILED(result))
								::MessageBoxW(this->hWnd, L"Failed to create a bitmap from a WIC bitmap.",
								L"Error", MB_OK);
							else
								final_result = true;							
						}
						else
							::MessageBoxW(this->hWnd, L"Failed to create device dependent resources.",
							L"Error", MB_OK);

					}
					else
						::MessageBoxW(this->hWnd, L"Failed to convert the source image frame.",
						L"Error", MB_OK);
				}
				else
					::MessageBoxW(this->hWnd, L"Failed to create a format converter.", L"Error",
					MB_OK);

				frame->Release();
			}
			else
				::MessageBoxW(this->hWnd, L"Failed to get an image frame from an WIC decoder.",
				L"Error", MB_OK);

			decoder->Release();
		}
		else
			::MessageBoxW(this->hWnd, L"Failed to create a WIC decoder from the selected file.",
			L"Error", MB_OK);
	}
	return final_result;
}

bool MainWindow::LocateImagefile(std::wstring &path)
{
	bool final_result(false);
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
					path = file_path;
					::CoTaskMemFree(file_path);
					final_result = true;
				}

				shell_item->Release();
			}
		}
		else if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {}
		else
			::MessageBoxW(NULL, L"Failed to get the path of the selected file.", L"Error", MB_OK);

		file_open_dialog->Release();
	}
	return final_result;
}

bool MainWindow::CreateResouces(void)
{
	HRESULT result = ::D2D1CreateFactory(::D2D1_FACTORY_TYPE_SINGLE_THREADED, &this->D2Factory);
	if (SUCCEEDED(result))
	{
		result = ::CoCreateInstance(::CLSID_WICImagingFactory, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&this->WicFactory));
		return SUCCEEDED(result) ? true : false;
	}
	else
		return false;
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
			::D2D1_SIZE_U sz = D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top);
			HRESULT result = this->D2Factory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(this->hWnd, sz),
				&this->RenderTarget);
			if (SUCCEEDED(result))
				return true;
		}
		return false;
	}
}

void MainWindow::ReleaseResources(void)
{
	SafeRelease(this->D2Factory);
	SafeRelease(this->BmpSrc);
	SafeRelease(this->WicFactory);
}

void MainWindow::ReleaseDeviceResources(void)
{
	SafeRelease(this->RenderTarget);
	SafeRelease(this->Bmp);
}

void MainWindow::OnPaint(void)
{
	if (this->CreateDeviceResources())
	{
		::PAINTSTRUCT ps;
		::BeginPaint(this->hWnd, &ps);

		if (!(this->RenderTarget->CheckWindowState() & ::D2D1_WINDOW_STATE_OCCLUDED))
		{
			this->RenderTarget->BeginDraw();

			// If Direct2D bitmap had been released due to divice loss, recreate it
			// from source bitmap.
			HRESULT result(S_OK);
			if (this->BmpSrc != nullptr && this->Bmp == nullptr)
				result = this->RenderTarget->CreateBitmapFromWicBitmap(this->BmpSrc, &this->Bmp);
			if (SUCCEEDED(result) && this->Bmp != nullptr)
			{
				// Draw an image and scale it to the current window size.
				const ::D2D1_SIZE_F sz = this->RenderTarget->GetSize();
				::D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f, sz.width, sz.height);
				this->RenderTarget->DrawBitmap(this->Bmp, rect);
			}
			result = this->RenderTarget->EndDraw();
			if (FAILED(result) || result == D2DERR_RECREATE_TARGET)
				this->ReleaseDeviceResources();
		}

		::EndPaint(this->hWnd, &ps);
	}
}

void MainWindow::OnSize(LPARAM lParam)
{
	if (this->RenderTarget != nullptr)
	{
		::D2D1_SIZE_U sz = D2D1::SizeU(LOWORD(lParam), HIWORD(lParam));
		if (FAILED(this->RenderTarget->Resize(sz)))
			this->ReleaseDeviceResources();
		else
			// Triggers WM_PAINT for the entire client area.
			::InvalidateRect(this->hWnd, nullptr, FALSE);
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
			if (!win.Create(L"Image viewer with Direct2D and WIC technology", WS_OVERLAPPEDWINDOW))
				return 0;
			::ShowWindow(win.Window(), cmdShow);
			RunMessageLoop();
		}
		::CoUninitialize();
	}

	return 0;
}