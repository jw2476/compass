#pragma once

#include <stdio.h>
#include <assert.h>

#define TRACE(...) printf("TRACE:%s:%04d -> ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\033[0;37m\n")
#define INFO(...) printf("\033[0;34mINFO:%s:%04d -> ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\033[0;37m\n")
#define WARN(...) printf("\033[0;33mWARN:%s:%04d -> ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\033[0;37m\n")
#define ERROR(...) printf("\033[0;31mERROR:%s:%04d -> ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\033[0;37m\n")
#define CRITICAL(...) printf("\033[0;31mCRITICAL:%s:%04d -> ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\033[0;37m\n"); assert(false)
