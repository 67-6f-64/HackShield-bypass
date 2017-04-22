#include "window.hpp"

#include "resource.hpp"

#include "update.hpp"

#include <thread>
#include <algorithm>

namespace gui
{
	WPARAM execute()
	{
		MSG message;
		while (GetMessage(&message, 0, 0, 0) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		return message.wParam;
	}

	window::window() : widget(0, 0, true), paint_font(0)
	{

	}
	
	window::~window()
	{
		if (this->paint_font)
		{
			DeleteObject(this->paint_font);
		}
	}

	bool window::assemble(std::string const& class_name, std::string const& window_name, rectangle& rect, const char* icon_name)
	{
		if (!this->create_class(class_name, icon_name))
		{
			return false;
		}

		if (!this->create_window(class_name, window_name, rect))
		{
			return false;
		}

		this->set_message_handlers();

		SetLayeredWindowAttributes(this->get_handle(), 0, 229, LWA_ALPHA);

		ShowWindow(this->get_handle(), SW_SHOWNORMAL);
		UpdateWindow(this->get_handle());
		
		this->progress = new progressbar(this->get_handle(), this->get_instance());
		this->progress->assemble(rectangle(-1, 97, 290, 21), 0, 100);

		this->terminal = new listbox(this->get_handle(), this->get_instance());
		this->terminal->assemble(rectangle(-1, 117, 290, 150), true);

		std::thread thread(std::bind(&window::worker_thread, this));
		thread.detach();

		return true;
	}

	bool window::worker_thread()
	{
		char directory[MAX_PATH];

		if (!GetCurrentDirectory(MAX_PATH, directory))
		{
			return false;
		}

		try
		{
			ahn::update* updater = new ahn::update();

			//updater->initialize(directory);
			//this->progress->set_position(20);

			//updater->update_autoup();
			//this->progress->set_position(40);

			//updater->update_mhe();
			//this->progress->set_position(60);

			//updater->update_ehsvc();
			//this->progress->set_position(80);

			//updater->update_dat();
			this->progress->set_position(100);

			this->terminal->add_string("done!");
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
		catch (...)
		{
			MessageBox(NULL, "An exception occured while trying to update", "Exception!", 0);
		}

		SendMessage(this->get_handle(), WM_CLOSE, NULL, NULL);
		return true;
	}

	bool window::create_class(std::string const& class_name, const char* icon_name)
	{
		WNDCLASSEX WndClassEx;
		WndClassEx.cbSize = sizeof(WNDCLASSEX);
		WndClassEx.style = 0;
		WndClassEx.lpfnWndProc = window::WndProc;
		WndClassEx.cbClsExtra = 0;
		WndClassEx.cbWndExtra = 0;
		WndClassEx.hInstance = this->get_instance();
		WndClassEx.hIcon = LoadIcon(this->get_instance(), icon_name);
		WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClassEx.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
		WndClassEx.lpszMenuName = NULL;
		WndClassEx.lpszClassName = class_name.c_str();
		WndClassEx.hIconSm = LoadIcon(this->get_instance(), icon_name);

		return (RegisterClassEx(&WndClassEx) != NULL);
	}

	bool window::create_window(std::string const& class_name, std::string const& window_name, rectangle& rect)
	{
		if (!rect.get_x() && !rect.get_y())
		{
			RECT rc;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

			//center screen:
			//rect.set_xy((rc.right / 2) - (rect.get_width() / 2), (rc.bottom / 2) - (rect.get_height() / 2));

			//left corner screen:
			rect.set_xy(rc.right - rect.get_width(), rc.bottom - rect.get_height());
		}

		return this->create(class_name, window_name, rect, WS_POPUPWINDOW, WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST, NULL);
	}

	void window::set_message_handlers()
	{
		this->add_message_handlers(4,
			message_pair(WM_CLOSE, [](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
			{
				DestroyWindow(hWnd);
				return 0;
			}),
			message_pair(WM_DESTROY, [](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
			{
				PostQuitMessage(0);
				return 0;
			}),
			message_pair(WM_NCHITTEST, [](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
			{
				return HTCAPTION;
			}),
			message_pair(WM_PAINT, [this](HWND hWnd, WPARAM wParam, LPARAM lParam) -> LRESULT
			{
				this->paint_font = CreateFont(12, 0, 0, 0, FW_DONTCARE, 0, 0, 0, ANSI_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 5, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");

				this->background = LoadBitmap(this->get_instance(), MAKEINTRESOURCE(BMP_BACKGROUND));

				PAINTSTRUCT paint_struct;

				HDC hdc_temp = BeginPaint(hWnd, &paint_struct);
				HDC hdc = CreateCompatibleDC(hdc_temp);

				SelectObject(hdc, this->background);
				SelectObject(hdc, this->paint_font);

				SetTextColor(hdc, RGB(0, 0, 0));
				SetBkMode(hdc, TRANSPARENT);

				TextOut(hdc, 130, 80, "Version 2.0 (hs version 5.7.6.502)", 35);
				BitBlt(hdc_temp, 0, 0, 290, 190, hdc, 0, 0, SRCCOPY);

				DeleteDC(hdc);
				EndPaint(hWnd, &paint_struct);
				return 0;
			})
		);
	}
}