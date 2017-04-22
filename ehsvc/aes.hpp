#pragma once

#include "standards.hpp"

namespace ahn
{
	namespace crypto
	{
		namespace aes
		{
			struct aes_context
			{
				Padding(0x20C);
			};

			enum aes_mode
			{
				encrypt = 1,
				decrypt = 2,
			};

			void initialize_context(unsigned char* key, int key_length, aes_mode mode, aes_context& context);

			void decrypt_block(unsigned char* input, unsigned char* output, aes_context& context);
			void encrypt_block(unsigned char* input, unsigned char* output, aes_context& context);

			void relink();
		}
	}
}