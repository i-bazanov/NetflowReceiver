#pragma once

#if defined(__linux__)
	#define PLATFORM_LINUX 1
#elif defined(_WIN32) || defined(_WIN64)
	#define PLATFORM_WINDOWS 1
#endif
