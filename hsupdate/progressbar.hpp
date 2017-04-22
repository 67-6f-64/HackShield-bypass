#pragma once

#include "standards.hpp"

#include "basic_types.hpp"
#include "widget.hpp"

namespace gui
{
	enum class progressbar_state
	{
		red = PBST_ERROR,
		green = PBST_NORMAL,
		yellow = PBST_PAUSED
	};

	class progressbar : public widget
	{
	public:
		progressbar(HWND hwnd_parent, HINSTANCE instance);

		bool assemble(rectangle& rect, std::size_t minimum, std::size_t maximum);
		
		void set_position(unsigned int position);
		void set_state(progressbar_state state);
	};
}