#pragma once

#include "standards.hpp"

#include "basic_types.hpp"
#include "widget.hpp"

namespace gui
{
	class listbox : public widget
	{
	public:
		listbox(HWND hwnd_parent, HINSTANCE instance);

		bool assemble(rectangle& rect, bool timestamp = false);
		void add_string(const char* format, ...);

	private:
		bool timestamp;

		std::string get_timestamp();
	};
}