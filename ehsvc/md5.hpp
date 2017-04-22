#pragma once

#include "standards.hpp"

namespace ahn
{
	namespace crypto
	{
		namespace md5
		{
			struct md5_context
			{
				Padding(0x100);
			};

			void initialize_context(md5_context& context);
			void update(md5_context& context, void* data, int length);
			void finalize(md5_context& context, unsigned char* digest);

			void relink();
		}
	}
}