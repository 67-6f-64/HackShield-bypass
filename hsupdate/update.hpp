#pragma once

#include "standards.hpp"

#include "environment.hpp"

namespace ahn
{
	typedef unsigned int (__cdecl* Bz32_FileExtract_t)(const char* source, const char* destination);

	enum class patch_type
	{
		patch = 1,
		update = 2,
		autoup = 3
	};

	class ui
	{
	protected:
		void get_ui_base(char* ui_path, char* filename, char* output, std::size_t output_size);
		void get_ui_info(char* ui_path, char* filename, char* output, std::size_t output_size);

	private:		
		bool get_ui(char* ui_path, char* filename, char* output, std::size_t output_size, bool base) const;
	};

	class update : public environment, public ui
	{
	public:
		update();

		bool initialize(const char* directory);

		void update_mhe();
		void update_ehsvc();
		void update_dat();
		void update_autoup();

	private:
		bool make_directory(const char* directory) const;

		void update_ui(bool patch);
		void update_file(char* filename, char* destination_file, patch_type type);

		bool download(char* directory, char* url, char* filename) const;
		bool extract(char* directory, char* destination, char* filename) const;

		bool outdated(char* file_path, char* ui_info);
		bool get_version(char* file_path, std::string& file_version) const;

		char directory[MAX_PATH];

		Bz32_FileExtract_t Bz32_FileExtract;
	};
}