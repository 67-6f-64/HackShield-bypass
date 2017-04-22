#pragma once

#include "standards.hpp"

#include "buffer.hpp"

namespace ahn
{
	class crc_generator
	{
	public:
		crc_generator();
		~crc_generator();

		void initialize(buffer::request* request);
		void update(buffer::request* request);

		unsigned int produce();

	private:
		typedef unsigned int (__cdecl* crc_generator_t)(unsigned int*, unsigned int);
		crc_generator_t generator;

		unsigned int crc_offset;
		unsigned int crc_key;
	};
}