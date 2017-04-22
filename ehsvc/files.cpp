#include "files.hpp"
#include "environment.hpp"
#include "md5.hpp"

#include <ImageHlp.h>
#pragma comment(lib, "ImageHlp")

namespace ahn
{
	namespace files
	{
		std::string current_directory = std::string("");
		std::string current_process = std::string("");

		const std::string file_strings[5] =
		{
			std::string("\\hshield\\hshield.dat"),
			std::string("\\hshield\\3n.mhe"),
			std::string("\\hshield\\hsupdate.env"),
			std::string("\\hshield\\hei.msd"),
			std::string("\\hshield\\ehsvc_old.dll")
		};

		bool initialize()
		{
			char buffer[256];

			/* get the full path of the current process */
			if (!GetModuleFileName(NULL, buffer, 256))
			{
				return false;
			}

			current_process = std::string(buffer);
			*strrchr(buffer, '\\') = 0;
			current_directory = std::string(buffer);
			return true;
		}

		bool generate_hash(unsigned char* output, file_type filetype, std::size_t file_offset,
			std::size_t hash_offset, std::size_t hash_offset_size)
		{
			memset(output, 0, 16);

			std::string file_path = (filetype == file_type::_current_process ? current_process :
				current_directory + file_strings[static_cast<int>(filetype)]);

			HANDLE file = CreateFile(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (file == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			/* retrieve the full file-size */
			unsigned int file_size = GetFileSize(file, NULL) - file_offset;

			if (file_size == -1)
			{
				CloseHandle(file);
				return false;
			}

			unsigned int hash_location_offset = 0;
			unsigned int hash_size = file_size;

			if (hash_offset != 0 && hash_offset_size != 0)
			{
				unsigned int real_size = 0;

				/* retrieve the fixed file-size */
				if (!get_uncertificated_size(file, &real_size))
				{
					CloseHandle(file);
					return false;
				}

				if (real_size > 0)
				{
					hash_size = real_size;
				}

				SetFilePointer(file, 0, NULL, FILE_BEGIN);

				unsigned char buffer[4096];
				memset(buffer, 0, 4096);

				/* read 0x1000 (4096) bytes from the file (header) */
				if (ReadFile(file, buffer, 4096, new unsigned long, NULL) != TRUE)
				{
					CloseHandle(file);
					return false;
				}

				/* check if the retrieved information is correct */
				if (buffer + *reinterpret_cast<unsigned int*>(buffer + 0x3C) + 0xF8 <= buffer)
				{
					CloseHandle(file);
					return false;
				}

				hash_location_offset = *reinterpret_cast<unsigned int*>(buffer + 0x3C) + 0xF8;

				/* check if the retrieved size is correct */
				if (hash_size <= hash_location_offset)
				{
					CloseHandle(file);
					return false;
				}

				hash_size -= hash_location_offset;
			}

			HANDLE object = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);

			if (object == NULL)
			{
				CloseHandle(file);
				return false;
			}

			/* retrieve of the base-address of the required data */
			unsigned char* base_address = reinterpret_cast<unsigned char*>(MapViewOfFile(object, FILE_MAP_READ, 0, 0, file_size));

			if (base_address == nullptr)
			{
				CloseHandle(object);
				CloseHandle(file);
				return false;
			}

			/* generate md5-hash of the hash-data */
			ahn::crypto::md5::md5_context context;
			ahn::crypto::md5::initialize_context(context);

			if (hash_offset != 0 && hash_offset_size != 0)
			{
				unsigned char* hash_location = base_address + hash_location_offset;

				ahn::crypto::md5::update(context, hash_location + hash_offset, hash_offset_size);
				ahn::crypto::md5::update(context, hash_location, hash_size);
			}
			else
			{
				ahn::crypto::md5::update(context, base_address + file_offset, file_size);
			}

			ahn::crypto::md5::finalize(context, output);

			UnmapViewOfFile(base_address);
			CloseHandle(object);
			CloseHandle(file);
			return true;
		}

		bool read_environment(unsigned char* output)
		{
			memset(output, 0, 16);

			/* set file_path to hsupdate.env from the string-table */
			std::string file_path = current_directory + file_strings[static_cast<int>(file_type::_hsupdate_env)];

			HANDLE file = CreateFile(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (file == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			/* read the environment data (using the environment functions) */
			unsigned int crypto_key = 0;

			if (!environment::read_key(file, &crypto_key))
			{
				CloseHandle(file);
				return false;
			}

			if (!environment::read_data(file, output, 16, crypto_key))
			{
				CloseHandle(file);
				return false;
			}

			CloseHandle(file);
			return true;
		}

		bool read_guid(unsigned char* output)
		{
			memset(output, 0, 16);

			/* 16-byte (128-bit) table used as a key to check if the position of the GUID is correct */
			const unsigned int guid_table[4] = { 0xB74C84F2, 0x455B4CD5, 0x59D0D780, 0xE70E71EC };

			HANDLE file = CreateFile(current_process.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (file == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			/* retrieve both the full file-size and the fixed file-size */
			unsigned int file_size = GetFileSize(file, NULL);

			if (file_size == -1)
			{
				CloseHandle(file);
				return false;
			}

			unsigned int real_size = 0;

			if (!get_uncertificated_size(file, &real_size))
			{
				CloseHandle(file);
				return false;
			}

			if (real_size != 0)
			{
				file_size = real_size;
			}

			/* set the read-location of the comparison-check followed by the guid */
			SetFilePointer(file, file_size - 32, NULL, FILE_BEGIN);

			unsigned char buffer[16];
			memset(buffer, 0, 16);

			/* read 0x10 (16) bytes from the file (comparison-check) */
			if (ReadFile(file, buffer, 16, new unsigned long, NULL) != TRUE)
			{
				CloseHandle(file);
				return false;
			}

			/* compare the retrieved data with the guid_table key */
			if (memcmp(buffer, guid_table, 16) != 0)
			{
				CloseHandle(file);
				return false;
			}

			/* read 0x10 (16) bytes from the file (guid) */
			if (ReadFile(file, output, 16, new unsigned long, NULL) != TRUE)
			{
				CloseHandle(file);
				return false;
			}

			CloseHandle(file);
			return true;
		}

		bool read_header(unsigned char* output, file_type filetype, std::size_t length, std::size_t start_offset)
		{
			memset(output, 0, length);

			/* find the proper file in the file_strings variable */
			std::string file_path = current_directory + file_strings[static_cast<std::size_t>(filetype)];

			HANDLE file = CreateFile(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (file == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			/* set the start position of the file (0 by default) */
			SetFilePointer(file, start_offset, NULL, FILE_BEGIN);

			/* read <length> bytes from the file (header) */
			if (ReadFile(file, output, length, new unsigned long, NULL) != TRUE)
			{
				CloseHandle(file);
				return false;
			}

			CloseHandle(file);
			return true;
		}

		bool get_uncertificated_size(HANDLE file, unsigned int* real_size)
		{
			if (ImageEnumerateCertificates(file, CERT_SECTION_TYPE_ANY, new unsigned long, NULL, 0) != TRUE)
			{
				return false;
			}

			WIN_CERTIFICATE certficiate_header;

			if (ImageGetCertificateHeader(file, 0, &certficiate_header) != TRUE)
			{
				return false;
			}

			/* retrieve the index of the current position in the file */
			unsigned int current_position = SetFilePointer(file, 0, 0, FILE_CURRENT);

			/* retrieve the full fixed file-size */
			unsigned int size = GetFileSize(file, NULL) - certficiate_header.dwLength;

			SetFilePointer(file, size - 8, NULL, FILE_BEGIN);

			unsigned char buffer[8];

			/* read 0x08 (8) bytes from the file (real size parameter) */
			if (ReadFile(file, buffer, 8, new unsigned long, 0) != TRUE)
			{
				return false;
			}

			/* calculate the actual size */
			for (int i = 0; i < sizeof(buffer); i++)
			{
				if (buffer[7 - i] != 0)
				{
					*real_size = size - i;
					break;
				}
			}

			/* set the index of the current position in the file to the old position prior to reading */
			SetFilePointer(file, current_position, NULL, FILE_BEGIN);
			return true;
		}
	}
}