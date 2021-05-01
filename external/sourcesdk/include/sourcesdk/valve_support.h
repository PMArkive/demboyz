
#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <string.h>
#include "vector.h"

using uint64 = uint64_t;
using uint32 = uint32_t;
using uint16 = uint16_t;
using uint8 = uint8_t;
using int64 = int64_t;
using int32 = int32_t;
using int16 = int16_t;
using int8 = int8_t;
using uint = uint32_t;
using byte = uint8_t;
using uintp = uintptr_t;
using intp = intptr_t;

#if defined(_M_IX86)
#define __i386__ 1
#endif

#define IsPC() true
#ifdef _WIN64
    #define PLATFORM_WINDOWS_PC64 1
#endif

#define Assert(x) assert(x)
#define AssertMsg(x, ...) assert(x)
#define AssertMsg2(x, ...) assert(x)
#define AssertFatalMsg(x, ...) assert(x)

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define Q_memcpy memcpy

template <typename T>
inline T DWordSwapC( T dw )
{
   uint32 temp;

   static_assert( sizeof( T ) == sizeof(uint32) );

   temp  =   *((uint32 *)&dw) 				>> 24;
   temp |= ((*((uint32 *)&dw) & 0x00FF0000) >> 8);
   temp |= ((*((uint32 *)&dw) & 0x0000FF00) << 8);
   temp |= ((*((uint32 *)&dw) & 0x000000FF) << 24);

   return *((T*)&temp);
}
#define DWordSwap DWordSwapC
#define BigLong( val )				DWordSwap( val )


#define LittleDWord( val )			( val )

inline uint32 LoadLittleDWord( uint32 *base, unsigned int dwordIndex )
{
    return LittleDWord( base[dwordIndex] );
}

inline void StoreLittleDWord( uint32 *base, unsigned int dwordIndex, uint32 dword )
{
    base[dwordIndex] = LittleDWord(dword);
}

inline void LittleFloat(float* pOut, float* pIn)
{
    *pOut = *pIn;
}


#ifndef _WIN32
#define _strnicmp strncasecmp
#endif

inline int V_strnicmp( const char *str1, const char *str2, int n )
{
    const unsigned char *s1 = (const unsigned char*)str1;
    const unsigned char *s2 = (const unsigned char*)str2;
    for ( ; n > 0 && *s1; --n, ++s1, ++s2 )
    {
        if ( *s1 != *s2 )
        {
            // in ascii char set, lowercase = uppercase | 0x20
            unsigned char c1 = *s1 | 0x20;
            unsigned char c2 = *s2 | 0x20;
            if ( c1 != c2 || (unsigned char)(c1 - 'a') > ('z' - 'a') )
            {
                // if non-ascii mismatch, fall back to CRT for locale
                if ( (c1 | c2) >= 0x80 ) return _strnicmp( (const char*)s1, (const char*)s2, n );
                // ascii mismatch. only use the | 0x20 value if alphabetic.
                if ((unsigned char)(c1 - 'a') > ('z' - 'a')) c1 = *s1;
                if ((unsigned char)(c2 - 'a') > ('z' - 'a')) c2 = *s2;
                return c1 > c2 ? 1 : -1;
            }
        }
    }
    return (n > 0 && *s2) ? -1 : 0;
}
