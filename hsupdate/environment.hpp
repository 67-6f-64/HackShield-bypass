#pragma once

#include "standards.hpp"

namespace ahn
{
	class environment
	{
	public:
		environment();
		~environment();

		bool initialize(std::string const& directory);

	private:
		bool read_data(unsigned char* buffer, std::size_t size);

		bool indent(std::size_t size);
		bool read(unsigned char* buffer, std::size_t size);

		void decrypt_data(unsigned char* buffer, std::size_t size);
		bool prepare_buffer(unsigned char* buffer, std::size_t size);

		HANDLE file;
		unsigned int element_count;
		unsigned int crypto_key;

	protected:
		std::string address;
		unsigned char product;
	};
}