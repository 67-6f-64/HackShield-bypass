#pragma once

#include "standards.hpp"

namespace output
{
	void hexdump(void* input, int length);
	void fhexdump(FILE* file, void* input, int length);
}