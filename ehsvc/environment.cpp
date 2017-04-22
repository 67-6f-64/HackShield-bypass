#include "environment.hpp"

#include <vector>
#include <string>

namespace ahn
{
	namespace environment
	{
		bool read_key(HANDLE file, unsigned int* crypto_key)
		{
			SetFilePointer(file, 0, 0, FILE_BEGIN);

			unsigned char header_buffer[28];

			if (!read(file, header_buffer, 28))
			{
				return false;
			}

			if (*reinterpret_cast<unsigned short*>(header_buffer) != 'HV')
			{
				return false;
			}

			unsigned int key_one = *reinterpret_cast<unsigned int*>(header_buffer + 0x04);
			unsigned int key_two = *reinterpret_cast<unsigned int*>(header_buffer + 0x08);

			unsigned int element_count = (key_two ^ key_one) & 0xFF;
			*crypto_key = element_count ^ key_one;
			return true;
		}

		bool read_data(HANDLE file, unsigned char* buffer, std::size_t size, unsigned int crypto_key)
		{
			if (!prepare_buffer(buffer, size))
			{
				return false;
			}

			unsigned char* temp_buffer = new unsigned char[size];

			if (!prepare_buffer(temp_buffer, size))
			{
				delete[] temp_buffer;
				return false;
			}

			if (!read(file, temp_buffer, size))
			{
				delete[] temp_buffer;
				return false;
			}

			decrypt_data(temp_buffer, size, crypto_key);
			memcpy(buffer, temp_buffer, size);

			delete[] temp_buffer;
			return true;
		}

		bool indent(HANDLE file, std::size_t size)
		{
			unsigned char* buffer = new unsigned char[size];

			if (!read(file, buffer, size))
			{
				delete[] buffer;
				return false;
			}

			delete[] buffer;
			return true;
		}

		bool read(HANDLE file, unsigned char* buffer, std::size_t size)
		{
			unsigned long bytes_read;
			return (ReadFile(file, buffer, size, &bytes_read, NULL) != FALSE && bytes_read == size);
		}

		void decrypt_data(unsigned char* buffer, std::size_t size, unsigned int crypto_key)
		{
			unsigned char xor_size = static_cast<unsigned char>(size);
			unsigned char* temp_crypto_key = reinterpret_cast<unsigned char*>(&crypto_key);

			for (unsigned int i = 0; i < size; i++)
			{
				unsigned char xor_key = buffer[i] & 0xFF;
				buffer[i] = xor_size ^ buffer[i] ^ temp_crypto_key[i & 3];
				xor_size += xor_key;
			}
		}

		bool prepare_buffer(unsigned char* buffer, std::size_t size)
		{
			if (!buffer)
			{
				return false;
			}

			memset(buffer, 0, 4 * (size >> 2));

			unsigned char* buffer_end = buffer + (4 * (size >> 2));

			for (int i = (size & 3); i != 0; --i)
			{
				*reinterpret_cast<unsigned char*>(buffer_end++) = 0;
			}

			return true;
		}
	}
}