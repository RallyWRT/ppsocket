// file debug.h
#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifndef ASSERT

#ifdef _DEBUG

#pragma warning(disable:4127)

//void _trace(char *fmt, ...);

#define ASSERT(x) {if(!(x)) _asm{int 0x03}}
#define VERIFY(x) {if(!(x)) _asm{int 0x03}}  // 译注：为调试版本时产生中断有效

#else
#define ASSERT(x)
#define VERIFY(x) x                  // 译注：为发行版本时不产生中断
#endif

//#ifdef _DEBUG
//#define TRACE _trace
//
//#else
//inline void _trace(LPCTSTR fmt, ...) {fmt; }
//#define TRACE  1 ? (void)0 : _trace
//#endif

#endif // ASSERT

#endif // __DEBUG_H__