#include "update.hpp"

#include "window.hpp"

#include <strsafe.h>
#include <cstring>
#include <iostream>

#include <WinInet.h>

#pragma comment(lib, "wininet")
#pragma comment(lib, "version")

namespace ahn
{
	void strchg(char *buf, const char *str1, const char *str2)
	{
		char tmp[1024 + 1];
		char *p;

		while ((p = strstr(buf, str1)) != NULL) 
		{
			*p = '\0';
			p += strlen(str1);
			strcpy(tmp, p);
			strcat(buf, str2);
			strcat(buf, tmp);
		}
	}

	/* ui class */
	void ui::get_ui_base(char* ui_path, char* filename, char* output, std::size_t output_size)
	{
		this->get_ui(ui_path, filename, output, output_size, true);
	}

	void ui::get_ui_info(char* ui_path, char* filename, char* output, std::size_t output_size)
	{
		this->get_ui(ui_path, filename, output, output_size, false);
	}

	bool ui::get_ui(char* ui_path, char* filename, char* output, std::size_t output_size, bool base) const
	{
		char section[1024] = "\0";
		char* _section = section;
		GetPrivateProfileStringA(NULL, NULL, "DEFAULT", _section, 1024, ui_path);

		if (!strncmp(_section, "DEFAULT", 7))
		{
			return false;
		}

		while (*_section)
		{
			char key[1024] = "\0";
			char* _key = key;
			GetPrivateProfileStringA(_section, NULL, "DEFAULT", _key, 1024, ui_path);

			if (!strncmp(_section, "DEFAULT", 7))
			{
				return 0;
			}

			while (*_key)
			{
				if (!strncmp(_key, ".BASE", 5) && base)
				{
					GetPrivateProfileStringA(_section, _key, "DEFAULT", output, output_size, ui_path);
				}
				else if (!strcmp(_key, filename))
				{
					if (!base)
					{
						GetPrivateProfileStringA(_section, _key, "DEFAULT", output, output_size, ui_path);
					}

					return true;
				}

				_key = reinterpret_cast<char*>(memchr(_key, 0, 1024)) + 1;
			}

			_section = reinterpret_cast<char*>(memchr(_section, 0, 1024)) + 1;
		}

		return false;
	}

	/* update class */
	update::update() : Bz32_FileExtract(nullptr)
	{

	}

	bool update::initialize(const char* directory)
	{
		gui::window::singleton()->terminal->add_string("initializing environment...");

		strcpy(this->directory, directory);

		HMODULE Bz32Ex = LoadLibrary("Bz32Ex.dll");

		if (!Bz32Ex)
		{
			return false;
		}

		this->Bz32_FileExtract = reinterpret_cast<Bz32_FileExtract_t>(GetProcAddress(Bz32Ex, "Bz32Ex_FileExtract"));

		if (!this->Bz32_FileExtract)
		{
			return false;
		}

		environment::initialize(this->directory);

		this->update_ui(false);
		this->update_ui(true);
		return true;
	}

	void update::update_mhe()
	{
		this->update_file("3n.mh-", "3n.mhe", patch_type::update);
	}

	void update::update_ehsvc()
	{
		this->update_file("ehsvc.dl-", "ehsvc_old.dll", patch_type::patch);
	}

	void update::update_dat()
	{
		this->update_file("hshield.da-", "hshield.dat", patch_type::patch);
	}

	void update::update_autoup()
	{
		this->update_file("autoup.exe", "autoup.exe", patch_type::autoup);
	}

	bool update::make_directory(const char* directory) const
	{
		char temp_path[MAX_PATH] = "\0";

		while (true)
		{
			strncpy(temp_path, directory, strlen(directory));

			if (CreateDirectory(directory, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
			{
				break;
			}

			if (GetLastError() != ERROR_PATH_NOT_FOUND)
			{
				return false;
			}

			while (true)
			{
				*strrchr(temp_path, '\\') = '\0';

				if (CreateDirectory(temp_path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
				{
					break;
				}

				if (GetLastError() != ERROR_PATH_NOT_FOUND)
				{
					return false;
				}
			}		
		}

		return true;
	}

	void update::update_ui(bool patch)
	{
		gui::window::singleton()->terminal->add_string("updating ahn.ui...");

		char destination[MAX_PATH] = "\0";
		char url[MAX_PATH] = "\0";

		if (patch)
		{
			sprintf(destination, "%s\\Update\\patch\\%02X\\", this->directory, this->product);
			sprintf(url, "%spatch/%02X/", this->address.c_str(), this->product);
		}
		else
		{
			sprintf(destination, "%s\\Update\\", this->directory);
			sprintf(url, "%s", this->address.c_str());
		}

		this->make_directory(destination);
		this->download(destination, url, "ahn.ui");
	}

	void update::update_file(char* filename, char* destination_file, patch_type type)
	{
		gui::window::singleton()->terminal->add_string("checking %s for updates...", destination_file);

		char url[MAX_PATH] = "\0";

		char source_directory[MAX_PATH] = "\0";
		char destination_path[MAX_PATH] = "\0";

		char ui_base[120] = "\0";
		char ui_info[120] = "\0";

		switch (type)
		{
		case patch_type::patch:
			{
				char ui_path[MAX_PATH] = "\0";
				sprintf(ui_path, "%s\\Update\\patch\\%02X\\ahn.ui", this->directory, this->product);

				this->get_ui_base(ui_path, filename, ui_base, 120);
				this->get_ui_info(ui_path, filename, ui_info, 120);

				sprintf(source_directory, "%s\\Update\\patch\\%02X\\%s\\", this->directory, this->product, ui_base);

				strchg(ui_base, "\\", "/");
				sprintf(url, "%spatch/%02X/%s/", this->address.c_str(), this->product, ui_base);

				sprintf(destination_path, "%s\\%s", this->directory, destination_file);
				break;
			}
		case patch_type::update:
		case patch_type::autoup:
			{
				char ui_path[MAX_PATH] = "\0";
				sprintf(ui_path, "%s\\Update\\ahn.ui", this->directory);

				this->get_ui_base(ui_path, filename, ui_base, 120);
				this->get_ui_info(ui_path, filename, ui_info, 120);

				sprintf(source_directory, "%s\\Update\\%s\\", this->directory, ui_base);

				strchg(ui_base, "\\", "/");
				sprintf(url, "%s/%s/", this->address.c_str(), ui_base);

				sprintf(destination_path, (type == patch_type::update ? "%s\\%s" : "%s\\Update\\%s"), this->directory, destination_file);
				break;
			}

		default:
			throw std::string("wrong patch type!");
		}

		this->make_directory(source_directory);

		if (!this->outdated(destination_path, ui_info))
		{
			gui::window::singleton()->terminal->add_string("up-to-date...", destination_file);
		}
		else
		{
			gui::window::singleton()->terminal->add_string("outdated...", destination_file);

			this->download(source_directory, url, filename);

			if (type == patch_type::patch || type == patch_type::update)
			{
				this->extract(source_directory, destination_path, filename);
			}
		}
	}

	bool update::download(char* directory, char* url, char* filename) const
	{
		gui::window::singleton()->terminal->add_string("downloading...", filename);

		char url_buffer[512] = "\0";
		DWORD buffer_length = 512;

		if (!InternetCanonicalizeUrl(url, url_buffer, &buffer_length, ICU_NO_ENCODE))
		{
			return false;
		}

		char hostname[256] = "\0";
		char url_path[2084] = "\0";

		URL_COMPONENTS url_components;
		memset(&url_components, 0, sizeof(url_components));

		url_components.dwStructSize = sizeof(url_components);
		url_components.lpszHostName = hostname;
		url_components.dwHostNameLength = 256;
		url_components.lpszUrlPath = url_path;
		url_components.dwUrlPathLength = 2084;

		if (!InternetCrackUrl(url_buffer, 0, 0, &url_components))
		{
			return false;
		}

		sprintf(url_path, "%s%s", url_path, filename);

		HINTERNET internet = InternetOpen("HttpApp/1.0", 0, NULL, NULL, 0);
		
		if (!internet)
		{
			return false;
		}

		HINTERNET http_session = InternetConnect(internet, hostname, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
		
		if (!http_session)
		{
			return false;
		}

		HINTERNET http_request = HttpOpenRequest(http_session, "GET", url_path, NULL, NULL, 0, 0x84000100, 0);
	
		if (!http_request)
		{
			return false;
		}

		if (!HttpSendRequest(http_request, NULL, 0, NULL, 0))
		{
			return false;
		}

		char destination_path[MAX_PATH] = "\0";
		sprintf(destination_path, "%s%s", directory, filename);

		HANDLE file = CreateFileA(destination_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		
		if (file == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		while (true)
		{
			unsigned long number_of_bytes = 0;

			unsigned char buffer[MAX_PATH];
			InternetReadFile(http_request, buffer, sizeof(buffer), &number_of_bytes);

			if (!number_of_bytes || !strncmp(reinterpret_cast<char*>(buffer), "File not found", 14))
			{
				break;
			}

			if (!WriteFile(file, buffer, number_of_bytes, new unsigned long, NULL))
			{
				return false;
			}
		}

		CloseHandle(file);
		InternetCloseHandle(http_request);
		InternetCloseHandle(http_session);
		InternetCloseHandle(internet);
		return true;
	}

	bool update::extract(char* directory, char* destination, char* filename) const
	{
		gui::window::singleton()->terminal->add_string("extracting...", filename);

		char source_path[MAX_PATH] = "\0";
		sprintf(source_path, "%s%s", directory, filename);

		return (this->Bz32_FileExtract(source_path, destination) != 0);
	}

	bool update::outdated(char* file_path, char* ui_info)
	{
		char version[120] = "\0";

		strchg(ui_info, ",,", ", ,");
		sscanf_s(ui_info, "%*[^,],%*[^,],%*[^,],%*[^,],%[^,],%*[^,]", version, 120);
		strchg(version, " ", "");

		std::string file_version;

		if (!this->get_version(file_path, file_version))
		{
			return true;
		}

		return (file_version.find(version) == std::string::npos);
	}

	bool update::get_version(char* file_path, std::string& file_version) const
	{
		std::size_t size = GetFileVersionInfoSize(file_path, nullptr);

		if (!size)
		{
			return false;
		}

		unsigned char* data = new unsigned char[size];

		if (!GetFileVersionInfo(file_path, NULL, size, data))
		{
			return false;
		}

		VS_FIXEDFILEINFO* version_info = nullptr;

		if (!VerQueryValueA(data, "\\", reinterpret_cast<void**>(&version_info), &size))
		{
			return false;
		}

		if (size == 0 || version_info->dwSignature != 0xfeef04bd)
		{
			return false;
		}

		std::string file_version_parts[] =
		{
			std::to_string(HIWORD(version_info->dwFileVersionMS)),
			std::to_string(LOWORD(version_info->dwFileVersionMS)),
			std::to_string(HIWORD(version_info->dwFileVersionLS)),
			std::to_string(LOWORD(version_info->dwFileVersionLS))
		};

		for (std::size_t index = 0; index < _countof(file_version_parts); index++)
		{
			if (index != 0)
			{
				file_version += std::string(".");
			}

			file_version += file_version_parts[index];
		}

		delete[] data;
		return true;
	}
}