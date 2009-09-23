// file debug.h
#ifndef __DEBUG_H__
#define __DEBUG_H__

//tianzuo,2009-8-6
//这个文件提供的TRACE函数有内存溢出的危险，因此全面禁用。
//需要ASSERT或TRACE功能的，用ATLASSERT和ATLTRACE代替
//注意包含头文件<atltrace.h>和<atlbase.h>

#include <atltrace.h>
#include <atlbase.h>
#define ASSERT ATLASSERT
#define TRACE  ATLTRACE
#define VERIFY ATLVERIFY

//#define CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

/*
#ifndef ASSERT

#ifdef _DEBUG

#pragma warning(disable:4127)

void _trace(char *fmt, ...);

#define ASSERT(x) {if(!(x)) _asm{int 0x03}}
#define VERIFY(x) {if(!(x)) _asm{int 0x03}}  // 译注：为调试版本时产生中断有效

#else
#define ASSERT(x)
#define VERIFY(x) x                  // 译注：为发行版本时不产生中断
#endif

#ifdef _DEBUG
#define TRACE _trace

#else
//inline void _trace(LPCTSTR fmt, ...) {fmt; }
//#define TRACE  1 ? (void)0 : _trace
#define TRACE  1 ? (void)0 : (void)0
#endif

#endif // ASSERT
*/
#endif // __DEBUG_H__