// file debug.h
#ifndef __DEBUG_H__
#define __DEBUG_H__

//tianzuo,2009-8-6
//����ļ��ṩ��TRACE�������ڴ������Σ�գ����ȫ����á�
//��ҪASSERT��TRACE���ܵģ���ATLASSERT��ATLTRACE����
//ע�����ͷ�ļ�<atltrace.h>��<atlbase.h>

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
#define VERIFY(x) {if(!(x)) _asm{int 0x03}}  // ��ע��Ϊ���԰汾ʱ�����ж���Ч

#else
#define ASSERT(x)
#define VERIFY(x) x                  // ��ע��Ϊ���а汾ʱ�������ж�
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