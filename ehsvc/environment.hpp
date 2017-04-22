#pragma once

#include "standards.hpp"

namespace ahn
{
	namespace environment
	{
		bool read_key(HANDLE file, unsigned int* crypto_key);
		bool read_data(HANDLE file, unsigned char* buffer, std::size_t size, unsigned int crypto_key);

		bool indent(HANDLE file, std::size_t size);
		bool read(HANDLE file, unsigned char* buffer, std::size_t size);

		void decrypt_data(unsigned char* buffer, std::size_t size, unsigned int crypto_key);
		bool prepare_buffer(unsigned char* buffer, std::size_t size);
	}
}