#pragma once

#include <memory.h>

#include "arena.h"
#include "array.h"
#include "file.h"
#include "str.h"
#include "types.h"
#include "log.h"

#define CLEAR(obj) memset(&obj, 0, sizeof(obj))
