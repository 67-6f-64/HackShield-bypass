#include "codesection.hpp"

typedef struct
{
	unsigned short offset_00;
	unsigned short offset_02;
	Padding(0x04);
	unsigned int offset_08;
	unsigned int offset_0C;
	unsigned int offset_10;
	unsigned int offset_14;
	unsigned int offset_18;
	Padding(0x04);
} codesection_t;

namespace ahn
{
	codesection::codesection()
	{
		/* constructor */
	}

	codesection::~codesection()
	{
		if (this->crc_allocation != nullptr)
		{
			free(this->crc_allocation);
		}

		if (this->codesections != nullptr)
		{
			free(this->codesections);
		}
	}

	bool codesection::initialize(buffer::request* request)
	{
		this->codesection_count = request->get<unsigned char>();		/* request + 0x00 */
		this->codesection_byte = request->get<unsigned char>();			/* request + 0x01 */

		request->indent(6);												/* request + 0x02 -> not used */

		this->codesection_data_1 = request->get<unsigned int>();		/* request + 0x08 */
		this->codesection_data_2 = request->get<unsigned int>();		/* request + 0x0C */

		for (unsigned int i = 0; i < codesection_count; i++)
		{
			unsigned int start_address = request->get<unsigned int>();	/* request + (0x10 + (0x08 * i)) */
			unsigned int size = request->get<unsigned int>();			/* request + (0x14 + (0x08 * i)) */

			this->codesection_infos.push_back({ start_address, start_address + size - 1, size });
		}

		if (this->already_initialized())
		{
			return true;
		}

		this->crc_allocation = reinterpret_cast<unsigned char*>(malloc(1024));

		if (!this->crc_allocation)
		{
			return false;
		}

		this->initialize_allocation(0x037ABE17, this->crc_allocation);

		if (this->codesection_count <= 0 || this->codesection_count > 40)
		{
			memset(this->crc_allocation, 0, 1024);
			free(this->crc_allocation);
			this->crc_allocation = nullptr;
			return false;
		}
		
		this->codesections = reinterpret_cast<unsigned char*>(malloc(32 * this->codesection_count));

		if (this->codesections == nullptr)
		{
			memset(this->crc_allocation, 0, 1024);
			free(this->crc_allocation);
			this->crc_allocation = nullptr;
			return false;
		}

		codesection_t* codesection = reinterpret_cast<codesection_t*>(this->codesections);
		unsigned int total_bitsum = 0;
		
		for (auto info : this->codesection_infos)
		{
			codesection->offset_08 = (this->codesection_data_1 + info.start_address) & 0xFFFFF000;
			codesection->offset_0C = this->codesection_data_2 + (info.start_address >= static_cast<unsigned int>(-static_cast<int>(this->codesection_data_1)));
			codesection->offset_10 = (this->codesection_data_1 + info.end_address) & 0xFFFFF000;
			codesection->offset_14 = this->codesection_data_2 + (info.end_address >= static_cast<unsigned int>(-static_cast<int>(this->codesection_data_1)));
			codesection->offset_18 = (info.size % 0x1000);

			unsigned int bitsum = codesection->offset_10 - codesection->offset_08 + 1;

			bitsum >>= 12;
			bitsum++;

			codesection->offset_00 = total_bitsum;
			codesection->offset_02 = bitsum;

			total_bitsum += bitsum;
			codesection += 32;
		}

		this->maximum_size_check = total_bitsum / this->codesection_byte;
		this->average_size_check = total_bitsum % this->codesection_byte;

		if (this->average_size_check == 0)
		{
			this->maximum_size_check--;
			this->average_size_check = this->codesection_byte;
		}

		srand(GetTickCount());

		this->current_size_check = rand() % (this->maximum_size_check + 1);
		this->minimum_size_check = this->codesection_byte;

		return true;
	}

	bool codesection::produce(buffer::request* request, unsigned char* response, unsigned int* size)
	{
		if (!this->crc_allocation)
		{
			return false;
		}
		
		*reinterpret_cast<unsigned int*>(response) = 0xFFFFFFFF;
		memset(response + 7, 0, this->codesection_count + (static_cast<unsigned int>(this->minimum_size_check + 7) >> 3));
		
		unsigned char zero_buffer_small_size = 0;
		unsigned short size_check_multiplier = 0;
		unsigned int loop_count = this->average_size_check + (this->minimum_size_check * this->maximum_size_check);
		
		for (unsigned int i = 0; i < loop_count; i += zero_buffer_small_size)
		{
			if (this->current_size_check == this->maximum_size_check)
			{
				zero_buffer_small_size = this->average_size_check;
				size_check_multiplier = this->current_size_check;
				this->current_size_check = 0;
			}
			else
			{
				zero_buffer_small_size = this->minimum_size_check;
				size_check_multiplier = this->current_size_check++;
			}

			*reinterpret_cast<unsigned short*>(response + 4) = size_check_multiplier;
			response[6] = 0;

			if (!this->code_call(response, zero_buffer_small_size, this->minimum_size_check * size_check_multiplier, true))
			{
				return false;
			}
		}

		if (response[6] <= 0)
		{
			return false;
		}

		unsigned short crc_size_check = request->get<unsigned short>();
		unsigned char crc_small_size = request->get<unsigned char>();

		request->indent(1);	/* for proper request-alignment */

		if (!this->code_call(response, crc_small_size, crc_size_check, false))
		{
			return false;
		}

		*size = 7 + response[6];
		return true;
	}

	bool codesection::codesections_exists()
	{
		return (this->codesections != nullptr);
	}

	bool codesection::code_call(unsigned char* response, unsigned short small_size, unsigned short size_check, bool first_call)
	{
		if (!small_size || !this->codesections_exists())
		{
			return false;
		}

		codesection_t* codesection = reinterpret_cast<codesection_t*>(this->codesections);

		for (unsigned int i = 0; i < this->codesection_count && small_size; i++, codesection += 32)
		{
			if (size_check < codesection->offset_00 || size_check >= (codesection->offset_00 + codesection->offset_02))
			{
				continue;
			}

			unsigned short big_size = static_cast<unsigned short>(codesection->offset_02 - (size_check - codesection->offset_00));

			unsigned short size = (small_size < big_size ? small_size : big_size);
			unsigned int extra_size = (small_size < big_size ? 0 : codesection->offset_18);
			
			unsigned int codesection_offset = ((size_check - codesection->offset_00) << 12);
			unsigned int codesection_size = (size << 12);

			unsigned char* codesection_start = reinterpret_cast<unsigned char*>(codesection->offset_08 + codesection_offset);
			unsigned char* codesection_end = codesection_start + codesection_size - 4096;
			
			if (first_call)
			{
				if (!this->produce_zero_buffer(codesection_start, codesection_end, size, codesection_size, extra_size, response))
				{
					return false;
				}
			}
			else
			{
				if (!this->produce_crc_data(codesection_start, codesection_end, size, codesection_size, extra_size, response))
				{
					return false;
				}
			}
			
			small_size -= size;
			size_check += size;
		}

		return true;
	}

	bool codesection::produce_zero_buffer(unsigned char* codesection_start, unsigned char* codesection_end, unsigned short size, 
		unsigned int codesection_size, unsigned int extra_size, unsigned char* response)
	{
		if (!this->crc_allocation)
		{
			return false;
		}
		
		/* sub_1009EAA7(dword_10153250, 0, codesection_start, &v81); */

		unsigned int zero_buffer_size = ((size + 7) >> 3);
		memset(response + 7, 0, zero_buffer_size);
		
		unsigned int start_size = (reinterpret_cast<unsigned int>(codesection_start) & 0xFFFFF000);
		unsigned int end_size = (reinterpret_cast<unsigned int>(codesection_end) & 0xFFFFF000);

		if ((codesection_start > codesection_end) || !response || (zero_buffer_size < (((end_size - start_size) >> 15) + 1)))
		{
			return false;
		}

		response[6] += zero_buffer_size;
		return true;

		/* 
		there's originally more code in this function, which runs under the condition that variable "generate_crc" == 1, 
		but according to the code, it should not be possible for "generate_crc" to actually be 1. Hence why the code has been excluded.
		*/
	}

	bool codesection::produce_crc_data(unsigned char* codesection_start, unsigned char* codesection_end, unsigned short size, 
		unsigned int codesection_size, unsigned int extra_size, unsigned char* response)
	{
		if (!this->crc_allocation)
		{
			return false;
		}

		unsigned short seed_size = 0;
		unsigned char* codesection_pointer = codesection_start;

		for (unsigned short i = 0; i < size; i++, codesection_pointer += seed_size)
		{
			seed_size = 4096;
			
			if (i == (size - 1) && extra_size)
			{
				seed_size = extra_size;
			}
			
			/* sub_1009EAA7(dword_10153250, 0, codesection_start, &v33); */
			
			unsigned int crc_seed = 0xFFFFFFFF;
			
			unsigned char seed_buffer[4096];
			memset(seed_buffer, 0, 4096);
			
			if (!this->copy_codesection_code(codesection_pointer, seed_size, seed_buffer))
			{
				return false;
			}

			this->generate_crc_data(seed_buffer, seed_size, reinterpret_cast<unsigned char*>(&crc_seed));
			this->generate_crc_data(reinterpret_cast<unsigned char*>(&crc_seed), 4, response);
		}

		return true;
	}

	bool codesection::copy_codesection_code(unsigned char* codesection_pointer, unsigned int seed_size, unsigned char* seed_output)
	{
		if (!codesection_pointer || !seed_size || seed_size > 4096)
		{
			return false;
		}
		
		memset(seed_output, 0, seed_size);
		memcpy(seed_output, codesection_pointer, seed_size);
		
		for (unsigned int i = 0; i < (seed_size - 1); i++)
		{
			if (codesection_pointer[i] == 0x90 && (codesection_pointer[i + 1] == 0xE8 || codesection_pointer[i + 1] == 0xE9))
			{
				memcpy(seed_output + i + 2, "FFFF", ((seed_size - i) < 6 ? seed_size - i - 2 : strlen("FFFF")));
			}
		}
		
		memcpy(seed_output, "FFFFF", strlen("FFFFF"));
		return true;
	}
	
	void codesection::generate_crc_data(unsigned char* seed_pointer, unsigned int seed_size, unsigned char* output)
	{
		for (unsigned int i = 0; i < seed_size; i++)
		{
			unsigned int output_lowbyte = (*reinterpret_cast<unsigned int*>(output) & 0xFF);
			unsigned int output_xor_key = (*reinterpret_cast<unsigned int*>(output) >> 8);
			
			unsigned int allocation_index = (seed_pointer[i] ^ output_lowbyte);
			unsigned int allocation_data = *reinterpret_cast<unsigned int*>(this->crc_allocation + (4 * allocation_index));
		
			*reinterpret_cast<unsigned int*>(output) = (allocation_data ^ output_xor_key);
		}
	}

	bool codesection::already_initialized()
	{
		return (this->codesections_exists() && this->minimum_size_check != 0x00);
	}

	void codesection::initialize_allocation(unsigned int key, unsigned char* allocation)
	{
		for (int i = 0; i <= 0xFF; i++)
		{
			*reinterpret_cast<unsigned int*>(allocation + (4 * i)) = (this->bitcheck(i, 8) << 24);

			for (int j = 0; j < 8; j++)
			{
				*reinterpret_cast<unsigned int*>(allocation + (4 * i)) = ((*reinterpret_cast<unsigned int*>(allocation + (4 * i)) & 0x80000000) != 0 ? key : 0) ^ (2 * *reinterpret_cast<unsigned int*>(allocation + (4 * i)));
			}

			*reinterpret_cast<unsigned int*>(allocation + (4 * i)) = this->bitcheck(*reinterpret_cast<unsigned int*>(allocation + (4 * i)), 32);
		}
	}

	unsigned int codesection::bitcheck(unsigned int data, unsigned int size)
	{
		int bit_check = 0;

		for (unsigned int i = 1; i < (size + 1); i++)
		{
			if (data & 1)
			{
				bit_check |= (1 << (size - i));
			}

			data >>= 1;
		}

		return bit_check;
	}
}