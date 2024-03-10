#pragma once
#include "Config.hpp"

#include <string>

namespace casioemu
{
	namespace logger
	{
		// Note that the printed string should end with a new line character.
		void Info(const char *format, ...);
	}
}

