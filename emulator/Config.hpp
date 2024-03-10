#pragma once

#include <cstdio>

#ifdef __GNUG__
# define FUNCTION_NAME __PRETTY_FUNCTION__
#else
# define FUNCTION_NAME __func__
#endif
#define PANIC(...) ( \
		std::fprintf(stderr, "%s:%i: in %s: ", __FILE__, __LINE__, FUNCTION_NAME), \
		std::fprintf(stderr, __VA_ARGS__), \
		std::exit(1) \
		)

#define MODEL_DEF_NAME "model.def"

