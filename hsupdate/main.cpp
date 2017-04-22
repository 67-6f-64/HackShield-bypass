#include "standards.hpp"

#include "resource.hpp"
#include "window.hpp"

int __stdcall WinMain(HINSTANCE instance, HINSTANCE prev_instance, char* command_line, int command_show)
{
	CreateMutex(NULL, TRUE, "mtx_hsupdate");

	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		gui::window* window = gui::window::singleton();
		window->set_instance(instance);

		if (!window->assemble("hsupdate_class", "hsupdate", gui::rectangle(0, 0, 290, 265), MAKEINTRESOURCE(IDI_ICON_RED)))
		{
			MessageBox(NULL, "Failed to create user interface.", "Error!", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
		}

		gui::execute();
	}

	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, "Global\\EF81BA4B-4163-44f5-90E2-F05C1E49C12D");
	SetEvent(hEvent);
	CloseHandle(hEvent);

	return 0;
}