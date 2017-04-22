#pragma once

#include "standards.hpp"

#include "buffer.hpp"

#include <vector>

namespace ahn
{
	typedef struct
	{
		unsigned int start_address;
		unsigned int end_address;
		unsigned int size;
	} codesection_info;

	class codesection
	{
	public:
		codesection();
		~codesection();

		bool initialize(buffer::request* request);
		bool produce(buffer::request* request, unsigned char* response, unsigned int* size);

		bool codesections_exists();

	private:
		bool code_call(unsigned char* response, unsigned short small_size, unsigned short size_check, bool first_call);

		bool produce_zero_buffer(unsigned char* codesection_start, unsigned char* codesection_end, unsigned short size, 
			unsigned int codesection_size, unsigned int extra_size, unsigned char* response);
		bool produce_crc_data(unsigned char* codesection_start, unsigned char* codesection_end, unsigned short size, 
			unsigned int codesection_size, unsigned int extra_size, unsigned char* response);

		bool copy_codesection_code(unsigned char* codesection_pointer, unsigned int seed_size, unsigned char* seed_output);
		void generate_crc_data(unsigned char* seed_pointer, unsigned int seed_size, unsigned char* output);

		bool already_initialized();

		void initialize_allocation(unsigned int key, unsigned char* allocation);
		unsigned int bitcheck(unsigned int data, unsigned int size);

		unsigned char codesection_count;
		unsigned char codesection_byte;

		unsigned int codesection_data_1;
		unsigned int codesection_data_2;

		unsigned char* crc_allocation;
		unsigned char* codesections;

		std::vector<codesection_info> codesection_infos;

		unsigned char minimum_size_check;
		unsigned char average_size_check;

		unsigned short maximum_size_check;
		unsigned short current_size_check;
	};
}