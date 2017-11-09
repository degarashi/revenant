#pragma once

#ifdef __clang__
	#define OPTIMIZE_OFF	_Pragma("clang optimize off")
	#define OPTIMIZE_RESET	_Pragma("clang optimize on")
#else
	#define OPTIMIZE_OFF	_Pragma("GCC optimize(push)") _Pragma("GCC optimize(\"O0\")")
	#define OPTIMIZE_RESET	_Pragma("GCC optimize(pop)")
#endif
