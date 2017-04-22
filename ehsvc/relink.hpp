#pragma once

#include "standards.hpp"

namespace ahn
{
	namespace crypto
	{
		namespace relink
		{
			void make_executable(unsigned char* address, unsigned int length);

			void find_and_replace_offset(unsigned char* start, unsigned int length, unsigned int find, unsigned int replace);
			void find_and_replace_call(unsigned char* start, unsigned int length, unsigned int find, unsigned int replace);

			unsigned char* find_offset(unsigned char* start, unsigned int length, unsigned int find);
		}
	}
}