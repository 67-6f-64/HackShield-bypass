#include "listbox.hpp"

#include <iomanip>
#include <thread>

namespace gui
{
	listbox::listbox(HWND hwnd_parent, HINSTANCE instance)
		: widget(hwnd_parent, instance, false), timestamp(false)
	{

	}

	bool listbox::assemble(rectangle& rect, bool timestamp)
	{
		if (!this->create(WC_LISTBOX, "", rect, WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOSEL, WS_EX_CLIENTEDGE, NULL))
		{
			return false;
		}

		this->timestamp = timestamp;
		return true;
	}

	void listbox::add_string(const char* format, ...)
	{
		char buffer[1024];
		memset(buffer, 0, sizeof(buffer));

		va_list va;
		va_start(va, format);

		vsprintf(buffer, format, va);

		if (this->timestamp)
		{
			ListBox_AddString(this->get_handle(), ("[" + this->get_timestamp() + "] " + buffer).c_str());
		}
		else
		{
			ListBox_AddString(this->get_handle(), buffer);
		}

		va_end(va);

		static int selection = 0;
		ListBox_SetCurSel(this->get_handle(), selection++);
	}

	std::string listbox::get_timestamp()
	{
		std::chrono::system_clock::time_point time_point_now = std::chrono::system_clock::now();
		std::time_t time_now = std::chrono::system_clock::to_time_t(time_point_now);
		std::put_time(std::localtime(&time_now), "%F %T");
		tm timenow = *std::localtime(&time_now);

		char time[256];
		strftime(time, 32, "%H:%M:%S", &timenow);
		return std::string(time);
	}
}