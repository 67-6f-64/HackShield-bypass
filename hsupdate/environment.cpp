#include "environment.hpp"

#include <vector>
#include <string>

namespace ahn
{
	environment::environment() : file(INVALID_HANDLE_VALUE)
	{

	}

	environment::~environment()
	{
		if (this->file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(this->file);
		}
	}

	bool environment::initialize(std::string const& directory)
	{
		std::string filename = directory + "\\HSUpdate.env";

		this->file = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (this->file == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		SetFilePointer(this->file, 0, 0, FILE_BEGIN);

		/* [0] read header and calculate decryption key */
		unsigned char header_buffer[28];

		if (!this->read(header_buffer, 28))
		{
			return false;
		}

		if (*reinterpret_cast<unsigned short*>(header_buffer) != 'HV')
		{
			return false;
		}

		unsigned int key_one = *reinterpret_cast<unsigned int*>(header_buffer + 0x04);
		unsigned int key_two = *reinterpret_cast<unsigned int*>(header_buffer + 0x08);

		this->element_count = (key_two ^ key_one) & 0xFF;
		this->crypto_key = this->element_count ^ key_one;

		/* [28] indent by 16, as we don't need this data */
		this->indent(16);

		/* [44] indent by the size of the index-buffer, as we don't need this data */
		this->indent(*reinterpret_cast<unsigned int*>(header_buffer + 0x14) << 3); // size == 48

		/* [92] indent by the size of 'SR' */
		this->indent(2);

		/* [94] read the table-count of the string-table */
		unsigned char count_buffer[12];

		if (!this->read_data(count_buffer, 12))
		{
			return false;
		}

		unsigned int table_count = *reinterpret_cast<unsigned int*>(count_buffer);
		unsigned int table_size = 624;
		unsigned int full_table_size = table_size * (table_count + 1);

		/* [106] read the string-table */
		std::vector<std::string> ahn_strings;

		for (unsigned int i = 0; i < table_count; i++)
		{
			unsigned char* table = new unsigned char[table_size];

			if (!this->read_data(table, table_size))
			{
				delete[] table;
				return false;
			}

			for (char* table_string = reinterpret_cast<char*>(table); table_string != reinterpret_cast<char*>(table + 624);
				table_string += std::string(table_string).size() + 1)
			{
				std::string ahn_string = std::string(table_string);

				if (!ahn_string.empty())
				{
					ahn_strings.push_back(ahn_string);
				}
			}

			delete[] table;
			this->product = static_cast<unsigned char>(9);
			this->address = ahn_strings[3];
		}

		for (auto x : ahn_strings)
		{
			//MessageBox(NULL, x.c_str(), 0, 0);
		}

		/* [730] indent by the size of 'FG' + the data it contains */
		this->indent(sizeof(static_cast<unsigned short>('FG')) + 8);

		/* [740] indent by the size of 'PI' + the data it contains */
		this->indent(sizeof(static_cast<unsigned short>('PI')) + 8);

		/* [750] indent by the size of 'TM' + the data it contains */
		this->indent(sizeof(static_cast<unsigned short>('TM')) + 16);

		/* [768] indent by the size of 'CD' + the data it contains */
		this->indent(sizeof(static_cast<unsigned short>('CD')) + 16);	// Fails to read this (file is probably at the end of document)

		/* [786] indent by the size of 'GR' + the data it contains */
		this->indent(sizeof(static_cast<unsigned short>('GR')) + 16);	// Fails to read this (file is probably at the end of document)

		/* [804] ... end of file ... */

		//TerminateProcess(GetCurrentProcess(), NULL);
		return true;
	}

	bool environment::read_data(unsigned char* buffer, std::size_t size)
	{
		if (!this->prepare_buffer(buffer, size))
		{
			return false;
		}

		unsigned char* temp_buffer = new unsigned char[size];

		if (!this->prepare_buffer(temp_buffer, size))
		{
			delete[] temp_buffer;
			return false;
		}

		if (!this->read(temp_buffer, size))
		{
			delete[] temp_buffer;
			return false;
		}

		this->decrypt_data(temp_buffer, size);
		memcpy(buffer, temp_buffer, size);

		delete[] temp_buffer;
		return true;
	}

	bool environment::indent(std::size_t size)
	{
		unsigned char* buffer = new unsigned char[size];

		if (!this->read(buffer, size))
		{
			delete[] buffer;
			return false;
		}

		delete[] buffer;
		return true;
	}

	bool environment::read(unsigned char* buffer, std::size_t size)
	{
		unsigned long bytes_read;
		return (ReadFile(this->file, buffer, size, &bytes_read, NULL) != FALSE && bytes_read == size);
	}

	void environment::decrypt_data(unsigned char* buffer, std::size_t size)
	{
		unsigned char xor_size = static_cast<unsigned char>(size);
		unsigned char* crypto_key = reinterpret_cast<unsigned char*>(&this->crypto_key);

		for (unsigned int i = 0; i < size; i++)
		{
			unsigned char xor_key = buffer[i] & 0xFF;
			buffer[i] = xor_size ^ buffer[i] ^ crypto_key[i & 3];
			xor_size += xor_key;
		}
	}

	bool environment::prepare_buffer(unsigned char* buffer, std::size_t size)
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