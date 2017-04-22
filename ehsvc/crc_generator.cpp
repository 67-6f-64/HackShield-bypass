#include "crc_generator.hpp"

namespace ahn
{
	crc_generator::crc_generator()
	{
		this->generator = reinterpret_cast<crc_generator_t>(VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE));
	}

	crc_generator::~crc_generator()
	{
		VirtualFree(this->generator, 0x1000, MEM_FREE | MEM_RELEASE);
	}

	void crc_generator::initialize(buffer::request* request)
	{
		const unsigned int crc_max_size = 0x00000150;

		this->crc_offset = request->get<unsigned int>();
		this->crc_key = request->get<unsigned int>();
		this->crc_offset = (this->crc_offset ^ this->crc_key ^ 0xE830F91F) & 0xFFFF;

		if (this->crc_offset > crc_max_size)
		{
			throw std::string("crc offset exceeds maximum size");
		}
		
		unsigned int size = (crc_max_size - this->crc_offset - 4);

		if (size <= 0)
		{
			throw std::string("invalid crc-generator size");
		}
						
		request->indent(this->crc_offset);

		unsigned char buffer[crc_max_size];
		request->get_aob(buffer, size);

		for (unsigned int i = 0, key = this->crc_key; i <= size; i++)
		{
			unsigned int encoded = *reinterpret_cast<unsigned int*>(buffer + i);
			*reinterpret_cast<unsigned int*>(reinterpret_cast<unsigned char*>(this->generator) + i) = encoded ^ key;
			key = encoded;
		}
	}

	void crc_generator::update(buffer::request* request)
	{
		this->crc_offset = request->get<unsigned int>();
		this->crc_key = request->get<unsigned int>();
		request->indent(4);
	}

	unsigned int crc_generator::produce()
	{
		return (this->generator ? this->generator(&this->crc_key, 4) : 0);
	}
}