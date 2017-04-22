#include "buffer.hpp"

#include "aes.hpp"
#include "md5.hpp"

namespace ahn
{
	namespace buffer
	{
		namespace crypto
		{
			const unsigned char cryptography_key[] = { 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10, 0x0F, 0x1E, 0x2D, 0x3C, 0x4B, 0x5A, 0x69, 0x78 };

			void decrypt_aes_key(unsigned char* request, unsigned char* aes_key)
			{
				unsigned char data[16];
				memcpy(data, request, 16);

				unsigned int digest[4];

				for (int i = 0; i < 5; i++)
				{
					ahn::crypto::md5::md5_context context;
					ahn::crypto::md5::initialize_context(context);
					ahn::crypto::md5::update(context, data, 16);
					ahn::crypto::md5::finalize(context, reinterpret_cast<unsigned char*>(digest));

					digest[0] ^= digest[2];
					digest[1] ^= digest[3];

					memcpy(data, digest, 16);
				}

				unsigned int temp[4];

				for (int i = 0; i < 4; i++)
				{
					temp[i] = digest[i] & 0xFFFF0000;
					digest[i] = digest[i] * 0x00010DCD + 1;
					temp[i] |= (digest[i] & 0xFFFF0000) >> 16;
				}

				memcpy(aes_key, temp, 16);

				for (int i = 0; i < 16; i++)
				{
					aes_key[i] ^= ((aes_key[i] >> 1) ^ (aes_key[i] & 1 ? 0xDF : 0x00));
				}
			}

			void decrypt_request(unsigned char* request, unsigned int request_length, unsigned char* aes_key)
			{
				decrypt_aes_key(request, aes_key);

				unsigned char* _request = request;

				unsigned char block[16];
				unsigned char decoded[16];

				ahn::crypto::aes::aes_context decode_context;
				memset(&decode_context, 0, sizeof(ahn::crypto::aes::aes_context));
				ahn::crypto::aes::initialize_context(aes_key, 16, ahn::crypto::aes::aes_mode::decrypt, decode_context);

				for (unsigned int i = 0; i < request_length / 16; i++)
				{
					memcpy(block, _request, 16);
					ahn::crypto::aes::decrypt_block(block, decoded, decode_context);
					memcpy(_request, decoded, 16);

					_request += 16;
				}

				int remainder = request_length % 16;

				if (remainder != 0)
				{
					memcpy(block, _request, remainder);

					for (int i = 0; i < remainder; i++)
					{
						block[i] ^= cryptography_key[i];
					}

					memcpy(_request, block, remainder);
				}
			}

			void encrypt_response(unsigned char* response, unsigned char* aes_key)
			{
				int length = *reinterpret_cast<unsigned short*>(response + 8);
			
				unsigned char* _response = response;

				unsigned char block[16];
				unsigned char encoded[16];

				ahn::crypto::aes::aes_context encode_context;
				memset(&encode_context, 0, sizeof(ahn::crypto::aes::aes_context));
				ahn::crypto::aes::initialize_context(aes_key, 16, ahn::crypto::aes::aes_mode::encrypt, encode_context);

				for (int i = 0; i < length / 16; i++)
				{
					memcpy(block, _response, 16);
					ahn::crypto::aes::encrypt_block(block, encoded, encode_context);
					memcpy(_response, encoded, 16);

					_response += 16;
				}

				int remainder = length % 16;

				if (remainder != 0)
				{
					memcpy(block, _response, remainder);

					for (int i = 0; i < remainder; i++)
					{
						block[i] ^= cryptography_key[i];
					}

					memcpy(_response, block, remainder);
				}
			}
		}
	}
}