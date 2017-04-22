#pragma once

#include "standards.hpp"

#include "buffer.hpp"

namespace ahn
{
	class nkcs
	{
	public:
		nkcs();
		~nkcs();

		void pack(unsigned char* source);

		void set_key(unsigned int key);
		
		void get_extended_response_1(buffer::request* request, buffer::response* response);
		void get_extended_response_2(buffer::response* response);

		void reset_incrementor();

		unsigned int get_product() const;
		unsigned int get_key() const;
		
	private:
		unsigned int get_key_product_1(unsigned int bit_flags);
		unsigned int get_key_product_2();
		
		void seed(unsigned char* source);
		bool verify(unsigned int count, unsigned char end);

		unsigned int nkcs_key;
		unsigned int nkcs_product;

		unsigned int incrementor;

		unsigned char input[0x20];
		unsigned char table[0x20];
	};
}