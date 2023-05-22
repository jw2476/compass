#pragma once

#include <stdbool.h>

#define MAKE_OPTION(type) \
typedef struct type ## _option_t {
	bool some;
	type data;
}
