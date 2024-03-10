#include "Logger.hpp"

#include <stdio.h>
#include <stdarg.h>


namespace casioemu
{
	namespace logger
	{
		void Info(const char *format, ...)
		{
			// * TODO may introduce race condition
			
			va_list args;
			va_start(args, format);
			vprintf(format, args);
			va_end(args);
		}
	}
}

