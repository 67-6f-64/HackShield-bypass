#include "standards.hpp"

#include "files.hpp"

#include "aes.hpp"
#include "md5.hpp"

namespace main
{
	void attach(HMODULE module)
	{
		AllocConsole();
		SetConsoleTitle("HackShield Bypass");
		AttachConsole(GetCurrentProcessId());
	
		FILE* pFile = nullptr;
		freopen_s(&pFile, "CON", "r", stdin);
		freopen_s(&pFile, "CON", "w", stdout);
		freopen_s(&pFile, "CON", "w", stderr);

		if (ahn::files::initialize())
		{
			ahn::crypto::aes::relink();
			ahn::crypto::md5::relink();
		}

		DisableThreadLibraryCalls(module);
	}
	
	void detach(HMODULE module)
	{
		FreeConsole();
	}
}

int __stdcall DllMain(HMODULE module, unsigned long reason, void* reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		main::attach(module);
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		main::detach(module);
	}

	return 1;
}