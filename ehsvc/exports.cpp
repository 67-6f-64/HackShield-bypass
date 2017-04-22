#include "standards.hpp"

#include "defs.hpp"
#include "heartbeat.hpp"

#include <thread>

static _AhnHS_Callback_t _AhnHS_Callback = nullptr;

int __stdcall _AhnHS_CallbackProc(void)
{
	if (_AhnHS_Callback != nullptr)
	{
		try
		{
			while (unsigned int status = HS_RUNNING_STATUS_CHECK_MONITORING_THREAD)
			{
				_AhnHS_Callback(AHNHS_ACTAPC_STATUS_HACKSHIELD_RUNNING, sizeof(unsigned int), &status);
				std::this_thread::sleep_for(std::chrono::seconds(25));
			}
		}
		catch (...)
		{
			printf("exception occured in callback thread\n");
		}
	}

	return 0;
}

bool __stdcall _AhnHS_ServiceDispatch(unsigned int code, void** params, unsigned int* error)
{	
	static ahn::heartbeat* heartbeat_emulator = nullptr;

	printf("Service dispatch %d\n", code);
	
	*error = HS_ERR_OK;

	switch (code)
	{
	case 4: /* Initialize */
 		{
			if (heartbeat_emulator)
			{
				*error = HS_ERR_ALREADY_INITIALIZED;
				return false;
			}

			_AhnHS_Callback = reinterpret_cast<_AhnHS_Callback_t>(params[0]);
			heartbeat_emulator = new ahn::heartbeat(reinterpret_cast<unsigned int>(params[3]));
			
			if (!heartbeat_emulator)
			{
				*error = HS_ERR_UNKNOWN;
				return false;
			}

			break;
		}
	case 5: /* StartService */
		{
			static bool service = false;

			if (service)
			{
				*error = HS_ERR_ALREADY_SERVICE_RUNNING;
			}
			else
			{
				service = true;
			}

			break;
		}
	case 13: /* MakeResponse */
		{
			if (!heartbeat_emulator)
			{
				*error = HS_ERR_NOT_INITIALIZED;
				return false;
			}

			if (!heartbeat_emulator->make_response(reinterpret_cast<unsigned char*>(params[0]),	reinterpret_cast<unsigned long>(params[1]), reinterpret_cast<unsigned char*>(params[2])))
			{
				*error = HS_ERR_UNKNOWN;
				return false;
			}

			break;
		}
	case 20: /* IsModuleSecure */
		{
			*error = HS_ERR_UNKNOWN;
			break;
		}
	case 34: /* CheckHackShieldRunningStatus */
		{
			if (_AhnHS_Callback != nullptr)
			{
				std::thread thrdCallback(_AhnHS_CallbackProc);
				thrdCallback.detach();
			}

			break;
		}
	default:
		break;
	}

	return true;
}

void __stdcall _AhnHS_SendHsLog(DWORD dwError, const char* szUserID, const char* szHShieldPath)
{
	return;
}

int __stdcall _AhnHS_VerifyProtectedFunction()
{
	return HS_ERR_OK;
}

BOOL __stdcall _AhnHS_QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
	return QueryPerformanceCounter(lpPerformanceCount);
}

BOOL __stdcall _AhnHS_QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
{
	return QueryPerformanceFrequency(lpFrequency);
}

unsigned long __stdcall _AhnHS_GetTickCount()
{
	return GetTickCount();
}