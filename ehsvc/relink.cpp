#include "relink.hpp"

namespace ahn
{
	namespace crypto
	{
		namespace relink
		{
			void make_executable(unsigned char* address, unsigned int length)
			{
				VirtualProtect(address, length, PAGE_EXECUTE_READWRITE, new unsigned long);
			}

			void find_and_replace_offset(unsigned char* start, unsigned int length, unsigned int find, unsigned int replace)
			{
				unsigned char* lpbIndex = start;

				while ((lpbIndex = find_offset(lpbIndex, length, find)) != NULL)
				{
					*reinterpret_cast<unsigned int*>(lpbIndex) = replace;
				}
			}

			void find_and_replace_call(unsigned char* start, unsigned int length, unsigned int find, unsigned int replace)
			{
				unsigned char* lpbIndex = find_offset(start, length, find);

				if (lpbIndex != NULL)
				{
					*reinterpret_cast<unsigned int*>(lpbIndex) = replace - reinterpret_cast<unsigned int>(lpbIndex - 1) - 5;
				}
			}

			unsigned char* find_offset(unsigned char* start, unsigned int length, unsigned int find)
			{
				for (unsigned char* index = start; index < (start + length - 4); index++)
				{
					if (*reinterpret_cast<unsigned int*>(index) == find)
					{
						return index;
					}
				}

				return NULL;
			}
		}
	}
}