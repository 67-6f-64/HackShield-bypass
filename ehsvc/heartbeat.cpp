#include "heartbeat.hpp"
#include "defs.hpp"
#include "files.hpp"

#include <functional>

#include "output.hpp"

namespace ahn
{
	heartbeat::heartbeat(unsigned int options) : options(options)
	{
		/* setup the code-section generator */
		this->codesection = new ahn::codesection();

		/* setup the crc generator */
		this->crc_generator = new ahn::crc_generator();

		/* setup the nkcs verification */
		this->nkcs = new ahn::nkcs();
	}

	heartbeat::~heartbeat()
	{

	}

	bool heartbeat::make_response(unsigned char* request, unsigned int length, unsigned char* response)
	{
		try
		{
			memset(response, 0, sizeof(buffer::response));
			
			buffer::request _request(request);
			buffer::crypto::decrypt_request(_request.buffer, length, this->aes_key);

			printf("Heartbeat request: %04X, %04X\n", _request.type, _request.flags);

			buffer::response _response;
			_response.key = (_request.key << 15) ^ _request.key;
			_response.monitor = ((_request.key & 0xFFFFFEFF) | 0x01010001) ^ _request.key;
			_response.type = _request.type + 1;

			this->handle_type(&_request, &_response);
			this->handle_flags(&_request, &_response);
			
			buffer::crypto::encrypt_response(reinterpret_cast<unsigned char*>(&_response), this->aes_key);
			memcpy(response, &_response, sizeof(buffer::response));
		}
		catch (std::string& error_exception)
		{
			printf("error/exception: %s\n", error_exception.c_str());
			return false;
		}

		return true;
	}

	void heartbeat::handle_type(buffer::request* request, buffer::response* response)
	{
		if (request->type < 0x0001 || request->type > 0x0009 || (request->type % 2) == 0)
		{
			throw std::string("heartbeat type is illegal");
		}

		std::function<void(buffer::request*, buffer::response*)> type_handlers[] =
		{
			nullptr,
			std::bind(&heartbeat::type_handler_0001, this, request, response),
			nullptr,
			std::bind(&heartbeat::type_handler_0003, this, request, response),
			nullptr,
			std::bind(&heartbeat::type_handler_0005, this, request, response),
			nullptr,
			std::bind(&heartbeat::type_handler_0007, this, request, response),
			nullptr,
			std::bind(&heartbeat::type_handler_0009, this, request, response)
		};

		type_handlers[request->type](request, response);
	}

	void heartbeat::type_handler_0001(buffer::request* request, buffer::response* response)
	{
		this->nkcs->reset_incrementor();

		unsigned char buffer[160];
		memset(buffer, 0, sizeof(buffer));

		ahn::files::read_guid(buffer);
		response->add_aob(buffer, 16);

		ahn::files::read_header(buffer, ahn::files::file_type::_hshield_dat, 160);
		response->add_aob(buffer, 160);

		ahn::files::read_header(buffer, ahn::files::file_type::_3n_mhe, 160, 2);
		response->add_aob(buffer, 160);

		ahn::files::read_environment(buffer);
		response->add_aob(buffer, 16);

		ahn::files::generate_hash(buffer, ahn::files::file_type::_hsupdate_env);
		response->add_aob(buffer, 16);

		ahn::files::read_header(buffer, ahn::files::file_type::_hei_msd, 40, 136);
		response->add_aob(buffer, 40);

		response->add<unsigned int>(0x00000000); /* if (sub_10030BE0(v7, dword_1014E844, buffer)) | Has a real value */
		response->add<unsigned int>(0x00000000);

		response->add<unsigned int>(static_cast<unsigned char>(0x01));
		response->add<unsigned int>(0x00000000);
	}

	void heartbeat::type_handler_0003(buffer::request* request, buffer::response* response)
	{
		if (request->length < 0x34)
		{
			throw std::string("request length is less than minimum for type 3.");
		}

		this->codesection->initialize(request);

		if (request->length < (request->offset + 16))
		{
			throw std::string("request length is less than minimum for type 3.");
		}

		request->indent(16); /* Contains some data to be output, but never actually used in generation. */
	}

	void heartbeat::type_handler_0005(buffer::request* request, buffer::response* response)
	{
		/* A basic heartbeat - the type has no impact on the response */
	}

	void heartbeat::type_handler_0007(buffer::request* request, buffer::response* response)
	{
		if (request->flags)
		{
			throw std::string("type 7 error");
		}

		/* type 0x0007 handler is not coded */
	}

	void heartbeat::type_handler_0009(buffer::request* request, buffer::response* response)
	{
		if (!request->flags || request->length < 0x120)
		{
			throw std::string("type 9 error");
		}

		unsigned int error_code = request->get<unsigned int>();

		if (error_code > ANTICPXSVR_BASECODE_ERROR && error_code <= ERROR_ANTICPXSVR_UNKNOWN)
		{
			/* decrypt the AntiCpxSvr error string */
		}

		printf("Heartbeat error: %08X, %s\n", error_code, ahn::get_error_name(error_code).c_str());
	}

	void heartbeat::handle_flags(buffer::request* request, buffer::response* response)
	{
		unsigned short flags[] = 
		{ 
			0x0020, 0x0001, 0x0010, 0x0002, 0x0004, 
			0x0008, 0x0040, 0x0080, 0x0100, 0x0200, 
			0x0400, 0x0800, 0x1000, 0x0100, 0x2000,
		};

		std::function<void(buffer::request*, buffer::response*)> flag_handlers[] =
		{
			std::bind(&heartbeat::flag_handler_0020, this, request, response),
			std::bind(&heartbeat::flag_handler_0001, this, request, response),
			std::bind(&heartbeat::flag_handler_0010, this, request, response),
			std::bind(&heartbeat::flag_handler_0002, this, request, response),
			std::bind(&heartbeat::flag_handler_0004, this, request, response),
			std::bind(&heartbeat::flag_handler_0008, this, request, response),
			std::bind(&heartbeat::flag_handler_0040, this, request, response),
			std::bind(&heartbeat::flag_handler_0080, this, request, response),
			std::bind(&heartbeat::flag_handler_0100, this, request, response),
			std::bind(&heartbeat::flag_handler_0200, this, request, response),
			std::bind(&heartbeat::flag_handler_0400, this, request, response),
			std::bind(&heartbeat::flag_handler_0800, this, request, response),
			std::bind(&heartbeat::flag_handler_1000, this, request, response),
			std::bind(&heartbeat::flag_handler_0100, this, request, response),
			std::bind(&heartbeat::flag_handler_2000, this, request, response),
		};

		unsigned int flag_sum = request->flags;

		for (int i = 0; i < _countof(flags); i++)
		{
			if (request->flags & flags[i])
			{
				if (flag_handlers[i] != nullptr)
				{
					flag_handlers[i](request, response);
				}

				if (flag_sum & flags[i])
				{
					flag_sum &= ~(flags[i]);
				}
			}

			if (response->length > ANTICPX_TRANS_BUFFER_MAX)
			{
				throw std::string("response size exceeds the maximum size");
			}
		}

		/* check if any unknown/unhandled flags appeared */
		if (flag_sum)
		{
			printf("There are unhandled flags. Unhandled flag sum: %04X\n", flag_sum);
		}
	}

	void heartbeat::flag_handler_0001(buffer::request* request, buffer::response* response)
	{
		unsigned char buffer[16];
		memset(buffer, 0, sizeof(buffer));

		ahn::files::generate_hash(buffer, ahn::files::file_type::_ehsvc_dll, 0, 0);
		response->add_aob(buffer, 16);
	}

	void heartbeat::flag_handler_0002(buffer::request* request, buffer::response* response)
	{
		unsigned int offset = request->get<unsigned int>();
		unsigned int size = request->get<unsigned int>();

		unsigned char buffer[16];
		memset(buffer, 0, sizeof(buffer));

		ahn::files::generate_hash(buffer, ahn::files::file_type::_current_process, 0, offset, size);
		response->add_aob(buffer, 16);
	}
	
	void heartbeat::flag_handler_0004(buffer::request* request, buffer::response* response)
	{
		if (!this->codesection->codesections_exists())
		{
			throw std::string("codesection hasn't been initialized yet");
		}

		unsigned int size = 0;

		unsigned char buffer[256];
		memset(buffer, 0, sizeof(buffer));

		if (!this->codesection->produce(request, buffer, &size))
		{
			throw std::string("codesection failed to produce crc");
		}

		response->add_aob(buffer, size);
	}

	void heartbeat::flag_handler_0008(buffer::request* request, buffer::response* response)
	{
		unsigned char buffer[16];
		memset(buffer, 0, sizeof(buffer));

		ahn::files::generate_hash(buffer, files::file_type::_3n_mhe, 162);
		response->add_aob(buffer, 16);
	}

	void heartbeat::flag_handler_0010(buffer::request* request, buffer::response* response)
	{
		if (request->get<unsigned int>() != 'SCKN')
		{
			throw std::string("nkcs offset is wrong");
		}

		request->indent(12);
		
		this->nkcs->set_key(request->get<unsigned int>());

		if ((request->flags & 0x0040) && request->length >= 0x15C)
		{
			unsigned char buffer[32];
			memset(buffer, 0, sizeof(buffer));
			
			request->peek_aob(buffer, 31);
			this->nkcs->pack(buffer);
		}

		response->add<unsigned int>(this->nkcs->get_product());
		response->add<unsigned int>(response->key);
		response->add<unsigned int>(0x00100000);
		response->add<unsigned int>(0x00000000);
		response->add<unsigned int>((this->nkcs->get_key() * this->nkcs->get_product()) % 0x0141ECB3);
	}

	void heartbeat::flag_handler_0020(buffer::request* request, buffer::response* response)
	{
		/* Sent with flag 0x0009, simply a heartbeat-error indicator */
	}

	void heartbeat::flag_handler_0040(buffer::request* request, buffer::response* response)
	{
		if ((request->length - request->offset) >= 0x15C)
		{
			this->crc_generator->initialize(request);
		}
		else
		{
			this->crc_generator->update(request);
		}

		response->add<unsigned int>(this->crc_generator->produce());
	}

	void heartbeat::flag_handler_0080(buffer::request* request, buffer::response* response)
	{
		response->add<unsigned int>(~(this->nkcs->get_key() * (this->options & 0xFFFF)) % 0xA8CFA835);
		response->add<unsigned int>(~(this->nkcs->get_key() * (this->options >> 0x10)) % 0xA8CFA835);
	}

	void heartbeat::flag_handler_0100(buffer::request* request, buffer::response* response)
	{
		static bool first = true;

		if (first)
		{
			this->nkcs->get_extended_response_1(request, response);
		}
		else
		{
			this->nkcs->get_extended_response_2(response);
		}

		first ^= true;
	}

	void heartbeat::flag_handler_0200(buffer::request* request, buffer::response* response)
	{
		/* Not yet coded in the client */
	}

	void heartbeat::flag_handler_0400(buffer::request* request, buffer::response* response)
	{
		unsigned char buffer[16];
		memset(buffer, 0, sizeof(buffer));

		ahn::files::generate_hash(buffer, ahn::files::file_type::_hsupdate_env);
		response->add_aob(buffer, 16);
	}

	void heartbeat::flag_handler_0800(buffer::request* request, buffer::response* response)
	{
		/* Not yet coded in the client */
	}

	void heartbeat::flag_handler_1000(buffer::request* request, buffer::response* response)
	{
		printf("flag 0x1000 handler accessed.\n");

		unsigned char buffer[32];
		memset(buffer, 0, sizeof(buffer));

		response->add_aob(buffer, 32);
	}

	void heartbeat::flag_handler_2000(buffer::request* request, buffer::response* response)
	{
		/* Not yet coded in the client */
	}
}