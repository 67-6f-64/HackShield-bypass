#pragma once

#include "defs.hpp"
#include "standards.hpp"

namespace ahn
{
	namespace buffer
	{
		#pragma pack(push, 1)
		struct response
		{
			unsigned int key;
			unsigned int monitor;
			unsigned short length;
			unsigned short type;
			unsigned char buffer[ANTICPX_TRANS_BUFFER_MAX - 12];
			unsigned short final_length;

			response()
			{
				this->length = 12;
				this->final_length = 12;
				memset(this->buffer, 0, sizeof(this->buffer));
			}

			template <typename T>
			void add(T data)
			{
				*reinterpret_cast<T*>(this->buffer + this->final_length - 12) = data;
				this->length += sizeof(T);
				this->final_length += sizeof(T);
			}

			void add_aob(unsigned char* input, std::size_t size)
			{
				memcpy(this->buffer + this->final_length - 12, input, size);
				this->length += static_cast<unsigned short>(size);
				this->final_length += static_cast<unsigned short>(size);
			}
		};

		struct request
		{
			union
			{
				struct
				{
					Padding(16); 	/* aes-key data */		/* 0x00 - 0x10 */
					unsigned int key;						/* 0x10 - 0x14 */
					unsigned short length;					/* 0x14 - 0x16 */
					unsigned char type;						/* 0x16 */
					Padding(1);								/* 0x17 */
					unsigned int flags;						/* 0x18 - 0x1C */
				};

				unsigned char buffer[ANTICPX_TRANS_BUFFER_MAX];
			};

			unsigned int offset;

			request(unsigned char* request) : offset(28)
			{
				memset(this->buffer, 0, sizeof(this->buffer));
				memcpy(this->buffer, request, sizeof(this->buffer));
			}

			void indent(std::size_t size)
			{
				this->offset += size;
			}

			template <typename T>
			T peek() const
			{
				return *reinterpret_cast<T*>(this->pointer);
			}

			template <typename T>
			T get()
			{
				this->offset += sizeof(T);
				return *reinterpret_cast<T*>(this->buffer + offset - sizeof(T));
			}

			void peek_aob(unsigned char* output, std::size_t size) const
			{
				if (output)
				{
					memcpy(output, this->buffer + offset, size);
				}
			}

			void get_aob(unsigned char* output, std::size_t size)
			{
				this->peek_aob(output, size);
				this->offset += size;
			}
		};
		#pragma pack(pop)

		namespace crypto
		{
			void decrypt_aes_key(unsigned char* request, unsigned char* aes_key);
			void decrypt_request(unsigned char* request, unsigned int request_length, unsigned char* aes_key);
			void encrypt_response(unsigned char* response, unsigned char* aes_key);
		}
	}
}