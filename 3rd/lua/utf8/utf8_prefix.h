#ifndef _LUAUTF8_PREFIX_H_
#define _LUAUTF8_PREFIX_H_

#if !defined(lua_c) && !defined(luac_c)
#include "utf8_crt.h"
#if !defined(lundump_c)
#include <Windows.h>
#endif
#if defined fopen
#undef fopen
#endif
#if defined popen
#undef popen
#endif
#if defined system
#undef system
#endif
#if defined remove
#undef remove
#endif
#if defined rename
#undef rename
#endif
#if defined getenv
#undef getenv
#endif
#define fopen(...) utf8_fopen(__VA_ARGS__)
#define popen(...) utf8_popen(__VA_ARGS__)
#define system(...) utf8_system(__VA_ARGS__)
#define remove(...) utf8_remove(__VA_ARGS__)
#define rename(...) utf8_rename(__VA_ARGS__)
#define getenv(...) utf8_getenv(__VA_ARGS__)
#define LoadLibraryExA(...) utf8_LoadLibraryExA(__VA_ARGS__)
#define GetModuleFileNameA(...) utf8_GetModuleFileNameA(__VA_ARGS__)
#define FormatMessageA(...) utf8_FormatMessageA(__VA_ARGS__)

#endif

#endif
