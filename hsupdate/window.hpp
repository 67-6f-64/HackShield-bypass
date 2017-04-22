#pragma once

#include "standards.hpp"

#include "basic_types.hpp"
#include "widget.hpp"

#include "listbox.hpp"
#include "progressbar.hpp"

#include <functional>
#include <unordered_map>

namespace gui
{
	WPARAM execute();

	class window : public widget
	{
	public:
		window();
		~window();

		bool assemble(std::string const& class_name, std::string const& window_name, rectangle& rect, const char* icon_name = IDI_APPLICATION);

		static window* singleton()
		{
			static window* singleton = new window;
			return singleton;
		}

		listbox* terminal;

	private:
		bool worker_thread();

		bool create_class(std::string const& class_name, const char* icon_name);
		bool create_window(std::string const& class_name, std::string const& window_name, rectangle& rect);

		void set_message_handlers();

		progressbar* progress;

		HFONT paint_font;
		HBITMAP background;
	};
}