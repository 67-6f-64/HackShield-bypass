#pragma once

#include "defs.hpp"
#include "standards.hpp"
#include "buffer.hpp"

#include "codesection.hpp"
#include "crc_generator.hpp"
#include "nkcs.hpp"

namespace ahn
{
	class heartbeat
	{
	public:
		heartbeat(unsigned int options);
		~heartbeat();

		bool make_response(unsigned char* request, unsigned int length, unsigned char* response);

	private:
		void handle_type(buffer::request* request, buffer::response* response);

		void type_handler_0001(buffer::request* request, buffer::response* response);
		void type_handler_0003(buffer::request* request, buffer::response* response);
		void type_handler_0005(buffer::request* request, buffer::response* response);
		void type_handler_0007(buffer::request* request, buffer::response* response);
		void type_handler_0009(buffer::request* request, buffer::response* response);

		void handle_flags(buffer::request* request, buffer::response* response);

		void flag_handler_0001(buffer::request* request, buffer::response* response);
		void flag_handler_0002(buffer::request* request, buffer::response* response);
		void flag_handler_0004(buffer::request* request, buffer::response* response);
		void flag_handler_0008(buffer::request* request, buffer::response* response);
		void flag_handler_0010(buffer::request* request, buffer::response* response);
		void flag_handler_0020(buffer::request* request, buffer::response* response);
		void flag_handler_0040(buffer::request* request, buffer::response* response);
		void flag_handler_0080(buffer::request* request, buffer::response* response);
		void flag_handler_0100(buffer::request* request, buffer::response* response);
		void flag_handler_0200(buffer::request* request, buffer::response* response);
		void flag_handler_0400(buffer::request* request, buffer::response* response);
		void flag_handler_0800(buffer::request* request, buffer::response* response);
		void flag_handler_1000(buffer::request* request, buffer::response* response);
		void flag_handler_2000(buffer::request* request, buffer::response* response);

		unsigned int options;

		unsigned char aes_key[16];

		ahn::codesection* codesection;
		ahn::crc_generator* crc_generator;
		ahn::nkcs* nkcs;
	};
}