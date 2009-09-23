/******************************************************************
* Core Library Version 1.6, June 2003
* Copyright (c) 2003-2004 chn cq hdsoft sp projects
* 
* File: MemoryPool.h
* Synopsis:
*      a memory pool template class.
*
* Written by 
*       stevenyiyi 
*
* Email: <stevenyiyi@126.com>
*
* $Id: MemoryPool.h,v 1.1 2003/06/17 14:30:59 
*****************************************************************/
#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include <cstdlib>
#include <typeinfo>
#include <iostream>
#include "jmutex.h"
using namespace std;

#define CORE_EXPANSION_SIZE 1024
namespace core
{
	template<class T> class MemoryPool : public JMutex
	{
	public:
	MemoryPool() : next(NULL), nCount(0)
	{
		Init();//≥ı ºªØª•≥‚¡ø
	}
	~MemoryPool()
	{
		releaseFreeList(nCount);
	}
	// allocate a T element from the free list.
	void* allocate(size_t) 
	{
		Lock();
		if (next == NULL)
			expandFreeList();
		MemoryPool<T> *head = next;
		next = head->next;
		nCount --;
		Unlock();
		return head;
	}
	// return a T element to the free list.
	void free(void* doomed) 
	{
		Lock();		
		if (doomed == NULL)
		{
			Unlock();		
			return;
		}
		 MemoryPool<T> *head = static_cast<MemoryPool<T> *>(doomed);
		 head->next = next;
		 next = head;
		 nCount ++;
		 Unlock();
	}
	// Access the corresponding static global allocator.
	static MemoryPool<T>& global_allocator()
	{
	   return memPool;
	}
	// release free list.
	void releaseFreeList(int howMany)
	{
		int i;
		MemoryPool<T> *nextPtr = next;  
		for (i=0; (nextPtr != NULL && i < howMany); i++)
		{
			nextPtr = next;
			next = nextPtr->next;
			::delete[] reinterpret_cast<char *>(nextPtr);
		}
		nCount -= i; /* in case failure */
	}
	private:
	    // next element on the free list.
		MemoryPool<T> *next;
    	// expand free list.
		void expandFreeList(int howMany = CORE_EXPANSION_SIZE)
		{
			size_t size = sizeof(T);
			if ( size < sizeof(MemoryPool<T> *) )
				size = sizeof(MemoryPool<T> *);
			char* p = ::new char[size];
			if (p == NULL)
				std::cerr << "Out of Memory!!!" << std::endl;
			MemoryPool<T> *runner = reinterpret_cast<MemoryPool<T> *>(p);
			next = runner;
			for (int i=0; i<howMany-1; i++)
			{
				p = ::new char[size];
				if (p == NULL)
					std::cerr << "Out of Memory!!!" << std::endl;
				runner->next = reinterpret_cast<MemoryPool<T> *>(p);
				runner = runner->next;
			}
			runner->next = NULL;
			nCount += howMany;
		}
    	// number of the free element.
		int nCount;
	    // Static global allocator.
		static MemoryPool<T> memPool;
	};
	template <class T> MemoryPool<T> MemoryPool<T>::memPool;
}
#endif

