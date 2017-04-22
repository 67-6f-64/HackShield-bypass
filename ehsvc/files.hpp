#pragma once

#include "standards.hpp"

namespace ahn
{
	namespace files
	{
		enum class file_type
		{
			_hshield_dat = 0,
			_3n_mhe = 1,
			_hsupdate_env = 2,
			_hei_msd = 3,
			_ehsvc_dll = 4,
			_current_process = 5
		};

		bool initialize();

		bool generate_hash(unsigned char* output, file_type filetype, std::size_t file_offset = 0,
			std::size_t hash_offset = 0, std::size_t hash_offset_size = 0);

		bool read_environment(unsigned char* output);
		bool read_guid(unsigned char* output);
		bool read_header(unsigned char* output, file_type filetype, std::size_t length, std::size_t start = 0);

		bool get_uncertificated_size(HANDLE file, unsigned int* real_size);
	};
}